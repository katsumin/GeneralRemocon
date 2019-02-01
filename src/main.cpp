#include <ArduinoJson.h>
#include <SD.h>
#include <Arduino.h>
#include "PLUSEncoder.h"
#include "Device.h"
#include "Selector.h"

#define SCROLL (18)
#define OFFSET_X (3)
#define OFFSET_Y (1)
#define BAR_WIDTH (8 * (SCROLL + 2))
#define BAR_HEIGHT (1 * (SCROLL + 2))
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
  if (step_x != 0)
  {
    int offset_x = (step_x < 0) ? SCROLL * 8 + OFFSET_X : -SCROLL * 8 + OFFSET_X;
    int y = 1;
    for (int i = 0; i <= SCROLL; i++)
    {
      int x = i * step_x * 8 + offset_x;
      stext1.drawString(label, x, 1, 2);
      stext1.drawRect(0, 0, BAR_WIDTH, BAR_HEIGHT, TFT_LIGHTGREY);
      stext1.pushSprite(BAR_X, BAR_Y);
      stext1.scroll(step_x * 8, 0);
      M5.Lcd.setCursor(160, 120);
      M5.Lcd.printf("(%3d,%3d)", x, y);
      delay(30);
    }
  }
  if (step_y != 0)
  {
    int x = 3;
    int offset_y = (step_y < 0) ? SCROLL + OFFSET_Y : -SCROLL + OFFSET_Y;
    for (int i = 0; i <= SCROLL; i++)
    {
      int y = i * step_y + offset_y;
      stext1.drawString(label, 3, y, 2);
      stext1.drawRect(0, 0, BAR_WIDTH, BAR_HEIGHT, TFT_LIGHTGREY);
      stext1.pushSprite(BAR_X, BAR_Y);
      stext1.scroll(0, step_y);
      M5.Lcd.setCursor(160, 120);
      M5.Lcd.printf("(%3d,%3d)", x, y);
      delay(30);
    }
  }
  // stext1.fillSprite(TFT_BLUE);
  // stext1.drawString(label, 3, 1, 2);
  // stext1.drawRect(0, 0, BAR_WIDTH, BAR_HEIGHT, TFT_LIGHTGREY);
  // stext1.pushSprite(BAR_X, BAR_Y);
}

void setup()
{
  M5.begin();
  Wire.begin();

  if (SD.exists(filename))
  {
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
  }
  else
  {
    // Demo mode
    selector->addChild(new SelectorCategory("menu1"));
    selector->addChild(new SelectorCategory("menu2"));
    selector->addChild(new SelectorCategory("menu3"));
    std::vector<SelectorUnit *> children = selector->getChildren();
    ((SelectorCategory *)(children[0]))->addChild(new SelectorCategory("menu1-1"));
    ((SelectorCategory *)(children[0]))->addChild(new SelectorCategory("menu1-2"));
    ((SelectorCategory *)(children[0]))->addChild(new ReturnSelector(RETURN_KEY));
    ((SelectorCategory *)(children[1]))->addChild(new SelectorCategory("menu2-1"));
    ((SelectorCategory *)(children[1]))->addChild(new SelectorCategory("menu2-2"));
    ((SelectorCategory *)(children[1]))->addChild(new SelectorCategory("menu2-3"));
    ((SelectorCategory *)(children[1]))->addChild(new ReturnSelector(RETURN_KEY));
    ((SelectorCategory *)(children[2]))->addChild(new SelectorCategory("menu3-1"));
    ((SelectorCategory *)(children[2]))->addChild(new SelectorCategory("menu3-2"));
    ((SelectorCategory *)(children[2]))->addChild(new SelectorCategory("menu3-3"));
    ((SelectorCategory *)(children[2]))->addChild(new SelectorCategory("menu3-4"));
    ((SelectorCategory *)(children[2]))->addChild(new ReturnSelector(RETURN_KEY));
    children = ((SelectorCategory *)(children[2]))->getChildren();
    ((SelectorCategory *)(children[1]))->addChild(new SelectorCategory("menu3-2-1"));
    ((SelectorCategory *)(children[1]))->addChild(new ReturnSelector(RETURN_KEY));
    Serial.println(children[1]->getLabel());
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