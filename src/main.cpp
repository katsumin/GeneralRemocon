#include <ArduinoJson.h>
#include <SD.h>
#include <Arduino.h>
#include "PLUSEncoder.h"
#include "Device.h"

const char *filename = "/remocon.json";
std::map<const char *, Device *> remocon;
std::vector<const char *> keys;
void setup()
{
  M5.begin();
  Wire.begin();

  Device::loadJson(filename, remocon);
  for (auto d : remocon)
    keys.push_back(d.first);
}

int device_index = 0;
void loop()
{
  M5.update();

  const char *key = keys[device_index];
  Device *d = remocon[key];
  if (PlusEncoder.update())
  {
    // 長押し
    if (PlusEncoder.isLongClick())
    {
      //   _menu.moveUp();
      Serial.println("longClick");
    }
    // エンコーダ逆回転
    if (PlusEncoder.wasDown())
    {
      device_index = (--device_index < 0) ? keys.size() - 1 : device_index;
      key = keys[device_index];
      // _menu.moveNext();
      Serial.println("down");
      Serial.println(key);
    }
    // エンコーダ正回転
    if (PlusEncoder.wasUp())
    {
      device_index = (++device_index >= keys.size()) ? 0 : device_index;
      key = keys[device_index];
      // _menu.movePrev();
      Serial.println("up");
      Serial.println(key);
    }
    // クリック
    if (PlusEncoder.isClick())
    {
      // _menu.selectItem();
      Serial.println("click");
      d->send();
    }
  }
}