#ifndef _RemoconDevice_H_
#define _RemoconDevice_H_

#include <ArduinoJson.h>
#include <Arduino.h>
#include <M5Stack.h>
#include <map>

/**
 * リモコンデバイス基底クラス
 */
class Device
{
private:
  const char *_name;
  const char *_custom_code;
  const char *_format;
  std::map<const char *, std::vector<const char *>> _buttons;

public:
  static void loadJson(const char *filename, std::map<const char *, Device *> &remocon);

  Device(const char *name, JsonObject &o);
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

/**
 * NEC形式
 */
class DeviceNec : public Device
{
public:
  DeviceNec(const char *name, JsonObject &o) : Device(name, o){};
  virtual void send();
};

/**
 * AEHA形式
 */
class DeviceAeha : public Device
{
public:
  DeviceAeha(const char *name, JsonObject &o) : Device(name, o){};
  virtual void send();
};

/**
 * SONY形式
 */
class DeviceSony : public Device
{
public:
  DeviceSony(const char *name, JsonObject &o) : Device(name, o){};
  virtual void send();
};
#endif
