#include <ArduinoJson.h>
#include <SD.h>
#include <Arduino.h>
#include "PLUSEncoder.h"
#include "Device.h"
#include "Selector.h"

#define BAR_WIDTH (8 * 20)
#define BAR_HEIGHT (1 * 20)
#define BAR_X (0)
#define BAR_Y (10)
#define RETURN_KEY "<return>"
#define ROOT_KEY "root"

TFT_eSprite stext1 = TFT_eSprite(&M5.Lcd); // Sprite object graph1
const char *filename = "/remocon.json";
std::map<const char *, Device *> remocon;
SelectorCategory *selector = new SelectorCategory(ROOT_KEY);
Device *d = NULL;
SelectorUnit *childSel = NULL;

void updateLabel(const char *label, int step_x, int step_y)
{
  stext1.fillSprite(TFT_BLUE);
  if (step_x == 0)
  {
    if (step_y == 0)
    {
      stext1.drawString(label, 3, 1, 2);
      stext1.pushSprite(BAR_X, BAR_Y);
      stext1.drawRect(0, 0, BAR_WIDTH, BAR_HEIGHT, TFT_LIGHTGREY);
    }
    else
    {
      int offset_y = (step_y < 0) ? 16 : -14;
      for (int i = 0; i < 16; i++)
      {
        stext1.drawString(label, 3, i * step_y + offset_y, 2);
        stext1.pushSprite(BAR_X, BAR_Y);
        stext1.scroll(0, step_y);
        stext1.drawRect(0, 0, BAR_WIDTH, BAR_HEIGHT, TFT_LIGHTGREY);
        delay(30);
      }
    }
  }
  else
  {
    int offset_x = (step_x < 0) ? 16 * 8 - 5 : -14 * 8 - 5;
    for (int i = 0; i < 16; i++)
    {
      stext1.drawString(label, i * step_x * 8 + offset_x, 1, 2);
      stext1.pushSprite(BAR_X, BAR_Y);
      stext1.scroll(step_x * 8, 0);
      stext1.drawRect(0, 0, BAR_WIDTH, BAR_HEIGHT, TFT_LIGHTGREY);
      delay(30);
    }
  }
}

void setup()
{
  M5.begin();
  Wire.begin();

  Device::init(RMT_CHANNEL_0, GPIO_NUM_13);
  Device::loadJson(filename, remocon);
  for (auto d : remocon)
  {
    SelectorCategory *s = new SelectorCategory(d.first);
    selector->addChild(s);
    for (auto b : d.second->getButtonKeys())
    {
      SelectorCategory *s2 = new SelectorCategory(b);
      s->addChild(s2);
    }
    ReturnSelector *r = new ReturnSelector(RETURN_KEY);
    s->addChild(r);
  }
  M5.Lcd.clear(BLACK);

  stext1.setColorDepth(8);
  stext1.createSprite(BAR_WIDTH, BAR_HEIGHT);
  stext1.fillSprite(TFT_BLUE);                                         // Fill sprite with blue
  stext1.setScrollRect(1, 1, BAR_WIDTH - 2, BAR_HEIGHT - 2, TFT_BLUE); // here we set scroll gap fill color to blue
  stext1.setTextColor(TFT_WHITE);                                      // White text, no background
  // stext1.setTextDatum(TL_DATUM);                            // Bottom right coordinate datum

  childSel = selector->reset();
  const char *key = childSel->getLabel();
  updateLabel(key, 0, -1);
}

void loop()
{
  M5.update();

  if (PlusEncoder.update())
  {
    // 長押し
    if (PlusEncoder.isLongClick())
    {
      Serial.println("longClick");
    }
    // エンコーダ逆回転
    if (PlusEncoder.wasDown())
    {
      Serial.println("down");
      childSel = selector->prev();
      updateLabel(childSel->getLabel(), 0, -1);
    }
    // エンコーダ正回転
    if (PlusEncoder.wasUp())
    {
      Serial.println("up");
      childSel = selector->next();
      updateLabel(childSel->getLabel(), 0, 1);
    }
    // クリック
    if (PlusEncoder.isClick())
    {
      Serial.println("click");
      const char *layer = selector->getLabel();
      Serial.printf("layer: %s, child:%s\n", layer, childSel->getLabel());
      if (strcmp(ROOT_KEY, layer) == 0)
      {
        // デバイス選択状態→ボタン選択状態にする
        selector = (SelectorCategory *)childSel;
        const char *deviceKey = childSel->getLabel();
        d = remocon[deviceKey];
        Serial.printf("device:%s, %p\n", deviceKey, d);
        childSel = selector->reset();
        const char *buttonKey = childSel->getLabel();
        updateLabel(buttonKey, 1, 0);
      }
      else
      {
        // ボタン選択状態
        const char *buttonKey = childSel->getLabel();
        if (strcmp(RETURN_KEY, buttonKey) == 0)
        {
          // デバイス選択状態に戻る
          updateLabel(selector->getLabel(), -1, 0);
          childSel = selector;
          selector = (SelectorCategory *)selector->getParent();
          d = NULL;
          Serial.printf("%p¥n", selector);
        }
        else
        {
          // ボタン送信
          if (d != NULL)
          {
            d->send(buttonKey);
          }
        }
        Serial.println(buttonKey);
      }
    }
  }
}