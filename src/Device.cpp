#include "Device.h"

Device::Device(const char *name, JsonObject &o)
{
  _name = name;
  _format = o["format"];
  _custom_code = o["custom"];
  JsonObject &buttons = o["buttons"];
  for (auto kv2 : buttons)
  {
    JsonArray &values = kv2.value;
    std::vector<const char *> vec;
    for (auto value : values)
    {
      const char *p = value.as<char *>();
      vec.push_back(p);
    }
    _buttons[kv2.key] = vec;
  }
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
      const char *name = kv.key;
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

void DeviceNec::send()
{
  Serial.println("nec");
}

void DeviceAeha::send()
{
  Serial.println("aeha");
}

void DeviceSony::send()
{
  Serial.println("sony");
}