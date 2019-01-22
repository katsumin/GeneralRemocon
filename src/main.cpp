#include <ArduinoJson.h>
#include <SD.h>
#include <Arduino.h>
#include "PLUSEncoder.h"
#include "Device.h"

const char *filename = "/remocon.json";
std::map<const char *, Device *> remocon;
void loadConfig()
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
      remocon[name] = d;
    }
  }
  else
  {
    Serial.println(F("Failed to read file, using default configuration"));
  }
}

void setup()
{
  M5.begin();
  Wire.begin();
  loadConfig();

  for (auto d : remocon)
  {
    d.second->print();
    d.second->send();
  }
}

void loop()
{
  M5.update();

  if (PlusEncoder.update())
  {
    // 長押し
    if (PlusEncoder.isLongClick())
      //   _menu.moveUp();
      Serial.println("longClick");
    // エンコーダ正回転
    if (PlusEncoder.wasDown())
      // _menu.moveNext();
      Serial.println("down");
    // エンコーダ逆回転
    if (PlusEncoder.wasUp())
      // _menu.movePrev();
      Serial.println("up");
    // クリック
    if (PlusEncoder.isClick())
      // _menu.selectItem();
      Serial.println("click");
  }
}