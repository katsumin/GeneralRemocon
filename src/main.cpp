#include <ArduinoJson.h>
#include <SD.h>
#include <Arduino.h>
#include "PLUSEncoder.h"

const char *filename = "/remocon.json";
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
    Serial.println(root.size());
    for (auto kv : root)
    {
      Serial.println(kv.key);
      JsonObject &o = root[kv.key];
      const char *format = o["format"];
      const char *custom = o["custom"];
      Serial.printf(" format:%s, customCode:%s\n", format, custom);
      JsonObject &buttons = o["buttons"];
      Serial.println(" buttons");
      for (auto kv2 : buttons)
      {
        JsonArray &values = kv2.value;
        std::vector<const char *> vec;
        for (auto value : values)
        {
          const char *p = value.as<char *>();
          // b.push_back(value.as<char *>());
          vec.push_back(p);
          Serial.println(p);
        }
        String s = "";
        for (const char *d : vec)
        {
          s.concat(d);
        }

        Serial.printf("  name:%s -> datas:%s", kv2.key, s.c_str());
        // Serial.printf("  name:%s -> data:%s\n", kv2.key, kv2.value.as<char *>());
      }
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