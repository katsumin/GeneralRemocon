#include "Device.h"

rmt_channel_t Device::_channel = RMT_CHANNEL_0;

Device::Device(const char *name, JsonObject &o)
{
  _name = duplicateString(name);
  _format = duplicateString(o["format"]);
  _custom_code = (ushort)(0xffff & strtoul(o["custom"], NULL, 16));
  JsonObject &buttons = o["buttons"];
  for (auto kv2 : buttons)
  {
    JsonArray &values = kv2.value;
    std::vector<const char *> vec;
    for (auto value : values)
    {
      vec.push_back(duplicateString(value.as<char *>()));
    }
    char *key = duplicateString(kv2.key);
    _buttons[key] = vec;
    _button_keys.push_back(key);
  }
}

/**
 * 初期化
 */
void Device::init(rmt_channel_t channel, gpio_num_t irPin)
{
  rmt_config_t rmtConfig;

  Device::_channel = channel;
  rmtConfig.rmt_mode = RMT_MODE_TX;
  rmtConfig.channel = channel;
  rmtConfig.clk_div = 80;
  rmtConfig.gpio_num = irPin;
  rmtConfig.mem_block_num = 1;

  rmtConfig.tx_config.carrier_duty_percent = 33;
  rmtConfig.tx_config.carrier_en = true;
  rmtConfig.tx_config.carrier_freq_hz = 38000;
  rmtConfig.tx_config.carrier_level = RMT_CARRIER_LEVEL_HIGH;
  rmtConfig.tx_config.idle_level = RMT_IDLE_LEVEL_LOW;
  rmtConfig.tx_config.idle_output_en = true;
  rmtConfig.tx_config.loop_en = false;

  rmt_config(&rmtConfig);
  rmt_driver_install(rmtConfig.channel, 0, 0);
}

/**
 * jsonファイル読み込み
 */
void Device::loadJson(const char *filename, std::map<const char *, Device *> &remocon)
{
  if (!SD.exists(filename))
  {
    Serial.println("no file!");
    return;
  }
  File file = SD.open(filename);
  DynamicJsonBuffer jsonBuffer;

  // Parse the root object
  JsonObject &root = jsonBuffer.parseObject(file);

  if (root.success())
  {
    // Serial.println(root.size());
    for (auto kv : root)
    {
      const char *name = duplicateString(kv.key);
      // Serial.println(name);
      JsonObject &o = root[name];
      const char *format = o["format"];
      Device *d = NULL;
      if (strcmp(format, "nec") == 0)
        d = new DeviceNec(name, o);
      else if (strcmp(format, "aeha") == 0)
        d = new DeviceAeha(name, o);
      else if (strcmp(format, "sony") == 0)
        d = new DeviceSony(name, o);
      if (d != NULL)
        remocon[name] = d;
    }
  }
  else
  {
    Serial.println(F("Failed to read file, using default configuration"));
  }
}

/**
 * データの送信
 * LSBファーストで、bit引数分だけ出力する。
 */
void Device::setWord(const ushort data, int bit, rmt_item32_t *rmtData)
{
  for (int i = 0; i < bit; i++)
  {
    if (data & (0x01 << i))
      setBit1(rmtData++);
    else
      setBit0(rmtData++);
  }
}

void DeviceNec::send(const char *key)
{
  // Serial.println("nec");
  std::vector<const char *> datas = getButtonDatas(key);
  for (auto data : datas)
  {
    int len = strlen(data);
    rmt_item32_t rmtData[1 + 2 * 8 + len * 4 + 1 + 2]; // leader bit + custom code bit + data bit + stop bit
    int pos = 0;
    setLeader(&rmtData[pos++]);              // 1bit
    setWord(getCustom(), 16, &rmtData[pos]); // 16bit
    pos += 16;
    for (int i = 0; i < len; i += 2)
    {
      ushort tmp = BIN_4BIT(data[i]) << 4;
      tmp |= BIN_4BIT(data[i + 1]);
      setWord(tmp, 8, &rmtData[pos]); // 8bit
      pos += 8;
    }
    setStop(&rmtData[pos++]);
    setSpace50ms(&rmtData[pos++]); // 50ms
    setSpace50ms(&rmtData[pos++]); // 50ms
    int size = sizeof(rmtData) / sizeof(rmt_item32_t);

    rmt_write_items(Device::_channel, rmtData, size, true);
  }
}

void DeviceAeha::send(const char *key)
{
  std::vector<const char *> datas = getButtonDatas(key);
  for (auto data : datas)
  {
    // Serial.println("aeha");
    int len = strlen(data);
    rmt_item32_t rmtData[1 + 2 * 8 + 1 * 4 + (len - 1) * 4 + 1 + 3]; // leader bit + custom code bit + parity bit + data bit + stop bit
    int pos = 0;
    setLeader(&rmtData[pos++]); // 1bit
    ushort d = getCustom();
    setWord(d, 16, &rmtData[pos]); // 16bit
    pos += 16;
    ushort parity = (byte)((d & 0xff) ^ (d >> 8));
    parity = (parity & 0xf) ^ (parity >> 4);
    setWord(parity, 4, &rmtData[pos]); // 4bit
    pos += 4;
    setWord(BIN_4BIT(data[1]), 4, &rmtData[pos]); // 4bit
    pos += 4;
    for (int i = 2; i < len; i += 2)
    {
      char tmp = BIN_4BIT(data[i]) << 4;
      tmp |= BIN_4BIT(data[i + 1]);
      setWord(tmp, 8, &rmtData[pos]); // 8bit
      pos += 8;
    }
    setStop(&rmtData[pos++]);
    setSpace50ms(&rmtData[pos++]); // 50ms
    setSpace50ms(&rmtData[pos++]); // 50ms
    setSpace50ms(&rmtData[pos++]); // 50ms
    int size = sizeof(rmtData) / sizeof(rmt_item32_t);
    // for (int i = 0; i < size; i++)
    // {
    //   Serial.printf("%4d: %d/%d(lv0:%d/lv1:%d)\n", i, //
    //                 rmtData[i].duration0,             //
    //                 rmtData[i].duration1,             //
    //                 rmtData[i].level0,                //
    //                 rmtData[i].level1);
    // }
    // Serial.println(Device::_channel);

    rmt_write_items(Device::_channel, rmtData, size, true);
  }
}

void DeviceSony::send(const char *key)
{
  // Serial.println("sony");
  std::vector<const char *> datas = getButtonDatas(key);
  for (auto data : datas)
  {
    rmt_item32_t rmtData[1 + 7 + _addr_bit + 1]; // leader bit + data bit + addres bit + stop bit
    int pos = 0;
    setLeader(&rmtData[pos++]); // 1bit
    ushort tmp = BIN_4BIT(data[0]) << 4;
    tmp |= BIN_4BIT(data[1]);
    setWord(tmp, 7, &rmtData[pos]); // 7bit
    pos += 7;
    setWord(getCustom(), _addr_bit, &rmtData[pos]); // 5 or 8 or 13bit
    pos += _addr_bit;
    setStop(&rmtData[pos]);
    int size = sizeof(rmtData) / sizeof(rmt_item32_t);
    // for (int i = 0; i < size; i++)
    // {
    //   Serial.printf("%4d: %d/%d(lv0:%d/lv1:%d)\n", i, //
    //                 rmtData[i].duration0,             //
    //                 rmtData[i].duration1,             //
    //                 rmtData[i].level0,                //
    //                 rmtData[i].level1);
    // }

    rmt_write_items(Device::_channel, rmtData, size, true);
  }
}