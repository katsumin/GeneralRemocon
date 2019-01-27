#ifndef _RemoconDevice_H_
#define _RemoconDevice_H_

#include <ArduinoJson.h>
#include <Arduino.h>
#include <M5Stack.h>
#include <map>
#include "driver/rmt.h"
#include "driver/periph_ctrl.h"
#include "soc/rmt_reg.h"

#define BIN_4BIT(d) ((d - '0' > 9)                                           \
                         ? ((d - 'A' > 5) ? (d - 'a' + 10) : (d - 'A' + 10)) \
                         : d - '0')
/**
 * リモコンデバイス基底クラス
 */
class Device
{
private:
  char *_name;
  ushort _custom_code;
  char *_format;
  std::map<const char *, std::vector<const char *>> _buttons;
  std::vector<const char *> _button_keys;
  char *duplicateString(const char *src)
  {
    char *buf = new char[strlen(src)];
    strcpy(buf, src);
    return buf;
  }

public:
  static rmt_channel_t _channel;
  static void
  loadJson(const char *filename, std::map<const char *, Device *> &remocon);
  static void init(rmt_channel_t channel, gpio_num_t irPin);

  Device(const char *name, JsonObject &o);
  ~Device() {}
  char *getName() { return _name; }
  char *getFormat() { return _format; }
  ushort getCustom() { return _custom_code; }
  std::map<const char *, std::vector<const char *>> getButtons() { return _buttons; }
  std::vector<const char *> getButtonKeys() { return _button_keys; }
  void print()
  {
    String s = "device: ";
    s.concat(_name);
    s.concat("\n format:");
    s.concat(_format);
    s.concat("\n custom:");
    s.concat(_custom_code);
    s.concat("\n buttons:\n  ");
    for (auto &kv : _buttons)
    {
      const char *key = kv.first;
      std::vector<const char *> v = kv.second;
      s.concat(key);
      s.concat(" -> [");
      for (const char *d : _buttons[key])
      {
        s.concat(d);
        s.concat(",");
      }
      s.concat("]\n  ");
    }
    Serial.println(s);
  }
  virtual void send(const char *key) {}

protected:
  std::vector<const char *> getButtonDatas(const char *key) { return _buttons[key]; }
  void setWord(const ushort data, int bit, rmt_item32_t *rmtData);
  virtual void setLeader(rmt_item32_t *rmtData) {}
  virtual void setBit1(rmt_item32_t *rmtData) {}
  virtual void setBit0(rmt_item32_t *rmtData) {}
  virtual void setStop(rmt_item32_t *rmtData) {}
  void setSpace50ms(rmt_item32_t *rmtData)
  {
    rmtData->duration0 = 25000;
    rmtData->duration1 = 25000;
    rmtData->level0 = 0; // off
    rmtData->level1 = 0; // off
  }
};

/**
 * NEC形式
 */
class DeviceNec : public Device
{
private:
  const int _T = 560;

protected:
  virtual void setLeader(rmt_item32_t *rmtData)
  {
    rmtData->duration0 = 16 * _T;
    rmtData->duration1 = 8 * _T;
    rmtData->level0 = 1; // on
    rmtData->level1 = 0; // off
  }
  virtual void setBit1(rmt_item32_t *rmtData)
  {
    rmtData->duration0 = _T;
    rmtData->duration1 = 3 * _T;
    rmtData->level0 = 1; // on
    rmtData->level1 = 0; // off
  }
  virtual void setBit0(rmt_item32_t *rmtData)
  {
    rmtData->duration0 = _T;
    rmtData->duration1 = _T;
    rmtData->level0 = 1; // on
    rmtData->level1 = 0; // off
  }
  virtual void setStop(rmt_item32_t *rmtData)
  {
    rmtData->duration0 = _T;
    rmtData->duration1 = 20 * _T; // 20 * 0.56 = 11.2ms
    rmtData->level0 = 1;          // on
    rmtData->level1 = 0;          // off
  }

public:
  DeviceNec(const char *name, JsonObject &o) : Device(name, o){};
  virtual void send(const char *key);
};

/**
 * AEHA形式
 */
class DeviceAeha : public Device
{
private:
  const int _T = 425;

protected:
  virtual void setLeader(rmt_item32_t *rmtData)
  {
    rmtData->duration0 = 8 * _T;
    rmtData->duration1 = 4 * _T;
    rmtData->level0 = 1; // on
    rmtData->level1 = 0; // off
  }
  virtual void setBit1(rmt_item32_t *rmtData)
  {
    rmtData->duration0 = _T;
    rmtData->duration1 = 3 * _T;
    rmtData->level0 = 1; // on
    rmtData->level1 = 0; // off
  }
  virtual void setBit0(rmt_item32_t *rmtData)
  {
    rmtData->duration0 = _T;
    rmtData->duration1 = _T;
    rmtData->level0 = 1; // on
    rmtData->level1 = 0; // off
  }
  virtual void setStop(rmt_item32_t *rmtData)
  {
    rmtData->duration0 = _T;
    rmtData->duration1 = 20 * _T; // 20 * 0->425 = 8.5ms >= 8ms
    rmtData->level0 = 1;          // on
    rmtData->level1 = 0;          // off
  }

public:
  DeviceAeha(const char *name, JsonObject &o) : Device(name, o){};
  virtual void send(const char *key);
};

/**
 * SONY形式
 */
class DeviceSony : public Device
{
private:
  const int _T = 600;
  int _addr_bit;

protected:
  virtual void setLeader(rmt_item32_t *rmtData)
  {
    rmtData->duration0 = 4 * _T;
    rmtData->duration1 = 1; // dummy
    rmtData->level0 = 1;    // on
    rmtData->level1 = 0;    // off
  }
  virtual void setBit1(rmt_item32_t *rmtData)
  {
    rmtData->duration0 = _T;
    rmtData->duration1 = 2 * _T;
    rmtData->level0 = 0; // off
    rmtData->level1 = 1; // on
  }
  virtual void setBit0(rmt_item32_t *rmtData)
  {
    rmtData->duration0 = _T;
    rmtData->duration1 = _T;
    rmtData->level0 = 0; // off
    rmtData->level1 = 1; // on
  }
  virtual void setStop(rmt_item32_t *rmtData)
  {
    rmtData->duration0 = 80 * _T; // 0.6 * 80 = 48ms >= 45ms
    rmtData->duration1 = 0;
    rmtData->level0 = 0; // off
    rmtData->level1 = 1; // on
  }

public:
  DeviceSony(const char *name, JsonObject &o) : Device(name, o)
  {
    _addr_bit = atoi(o["addr_bit"]);
  };
  virtual void send(const char *key);
};
#endif
