#ifndef _RemoconDevice_H_
#define _RemoconDevice_H_

#include <ArduinoJson.h>
#include <Arduino.h>
#include <M5Stack.h>
#include <map>

class Device
{
private:
  const char *_name;
  const char *_custom_code;
  const char *_format;
  std::map<const char *, std::vector<const char *>> _buttons;

public:
  Device(const char *name, JsonObject &o)
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
  ~Device() {}
  const char *getName() { return _name; }
  const char *getFormat() { return _format; }
  const char *getCustom() { return _custom_code; }
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
  virtual void send() {}
};

class DeviceNec : public Device
{
public:
  DeviceNec(const char *name, JsonObject &o) : Device(name, o){};
  virtual void send()
  {
    Serial.println("nec");
  }
};
class DeviceAeha : public Device
{
public:
  DeviceAeha(const char *name, JsonObject &o) : Device(name, o){};
  virtual void send()
  {
    Serial.println("aeha");
  }
};
class DeviceSony : public Device
{
public:
  DeviceSony(const char *name, JsonObject &o) : Device(name, o){};
  virtual void send()
  {
    Serial.println("sony");
  }
};
#endif
