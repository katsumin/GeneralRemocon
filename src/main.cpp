#include <ArduinoJson.h>
#include <SD.h>
#include <Arduino.h>
#include "PLUSEncoder.h"
#include "Device.h"

#define BAR_WIDTH (8 * 20)
#define BAR_HEIGHT (1 * 20)
TFT_eSprite stext1 = TFT_eSprite(&M5.Lcd); // Sprite object graph1
// TFT_eSprite stext2 = TFT_eSprite(&M5.Lcd); // Sprite object graph1
const char *filename = "/remocon.json";
std::map<const char *, Device *> remocon;
std::vector<const char *> device_keys;
void setup()
{
  M5.begin();
  Wire.begin();

  Device::init(RMT_CHANNEL_0, GPIO_NUM_13);
  Device::loadJson(filename, remocon);
  for (auto d : remocon)
    device_keys.push_back(d.first);

  M5.Lcd.clear(BLACK);

  stext1.setColorDepth(8);
  stext1.createSprite(BAR_WIDTH, BAR_HEIGHT);
  stext1.fillSprite(TFT_BLUE);                                 // Fill sprite with blue
  stext1.setScrollRect(0, 0, BAR_WIDTH, BAR_HEIGHT, TFT_BLUE); // here we set scroll gap fill color to blue
  stext1.setTextColor(TFT_WHITE);                              // White text, no background
  // stext1.setTextDatum(TL_DATUM);                            // Bottom right coordinate datum

  // stext2.setColorDepth(8);
  // stext2.createSprite(8 * 20, 16);
  // stext2.fillSprite(TFT_BLUE);                           // Fill sprite with blue
  // stext2.setScrollRect(8 * 20, 0, 8 * 20, 16, TFT_BLUE); // here we set scroll gap fill color to blue
  // stext2.setTextColor(TFT_WHITE);                        // White text, no background
  // stext2.setTextDatum(BR_DATUM);                         // Bottom right coordinate datum
}

int state = 0;
int cur_index = 0;
int pre_index = -1;
Device *d = NULL;
void loop()
{
  M5.update();

  if (pre_index != cur_index)
  {
    d = remocon[device_keys[cur_index]];
    stext1.fillSprite(TFT_BLUE);
    stext1.drawRect(0, 0, BAR_WIDTH, BAR_HEIGHT, TFT_LIGHTGREY);
    stext1.drawString(d->getName(), 1, 1, 2);
    stext1.pushSprite(0, 0);
    // stext1.scroll(-8 * 1, 0);
    delay(100);
  }
  pre_index = cur_index;
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
      Serial.println("down");
      cur_index = (--cur_index < 0) ? device_keys.size() - 1 : cur_index;
      // key = device_keys[cur_index];
      // _menu.moveNext();
    }
    // エンコーダ正回転
    if (PlusEncoder.wasUp())
    {
      Serial.println("up");
      cur_index = (++cur_index >= device_keys.size()) ? 0 : cur_index;
      // key = device_keys[cur_index];
      // _menu.movePrev();
    }
    // クリック
    if (PlusEncoder.isClick())
    {
      // _menu.selectItem();
      Serial.println("click");
      if (d != NULL)
      {
        std::vector<const char *> buttonKeys = d->getButtonKeys();
        const char *key = buttonKeys[0];
        Serial.println(key);
        d->send(key);
        // for (auto buttonKey : buttonKeys)
        // {
        //   Serial.println(buttonKey);
        //   d->send(buttonKey);
        // }
        // std::map<const char *, std::vector<const char *>> buttons = d->getButtons();
        // for (auto button : buttons)
        // {
        //   Serial.println(button.first);
        // }
      }
    }
  }
}