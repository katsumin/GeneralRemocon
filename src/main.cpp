#include <ArduinoJson.h>
#include <SD.h>
#include <Arduino.h>
#include "PLUSEncoder.h"
#include "Device.h"
#include "Selector.h"

#define BAR_WIDTH (8 * 20)
#define BAR_HEIGHT (1 * 20)

char *duplicateString(const char *src)
{
  char *buf = new char[strlen(src)];
  strcpy(buf, src);
  return buf;
}

TFT_eSprite stext1 = TFT_eSprite(&M5.Lcd); // Sprite object graph1
// TFT_eSprite stext2 = TFT_eSprite(&M5.Lcd); // Sprite object graph1
const char *filename = "/remocon.json";
std::map<const char *, Device *> remocon;
std::vector<const char *> device_keys;
SelectorCategory *selector = new SelectorCategory("root");
void setup()
{
  M5.begin();
  Wire.begin();

  Device::init(RMT_CHANNEL_0, GPIO_NUM_13);
  Device::loadJson(filename, remocon);
  for (auto d : remocon)
  {
    // device_keys.push_back(key);
    SelectorCategory *s = new SelectorCategory(d.first);
    selector->addChild(s);
    for (auto b : d.second->getButtonKeys())
    {
      SelectorCategory *s2 = new SelectorCategory(b);
      s->addChild(s2);
    }
    ReturnSelector *r = new ReturnSelector();
    s->addChild(r);
  }
  // for (auto child : selector->getChildren())
  // {
  //   Serial.printf("%s\n", child->getLabel());
  //   if (child->isLeaf())
  //     continue;

  //   for (auto child2 : ((SelectorCategory *)child)->getChildren())
  //   {
  //     Serial.printf(" %s\n", child2->getLabel());
  //   }
  // }

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

void updateLabel(const char *label)
{
  stext1.fillSprite(TFT_BLUE);
  stext1.drawRect(0, 0, BAR_WIDTH, BAR_HEIGHT, TFT_LIGHTGREY);
  stext1.drawString(label, 1, 1, 2);
  stext1.pushSprite(10, 10);
  delay(100);
}

// int state = 0;
// int cur_index = 0;
// int pre_index = -1;
enum select_state
{
  DEVICE_SELECT,
  BUTTON_SELECT,
};
select_state state = DEVICE_SELECT;
Device *d = NULL;
SelectorUnit *childSel = NULL;
void loop()
{
  M5.update();

  if (childSel == NULL)
  {
    childSel = selector->reset();
    const char *key = childSel->getLabel();
    updateLabel(key);
  }
  // Serial.printf("%p, %s\n", childSel, childSel->getLabel());
  // if (pre_index != cur_index)
  // {
  //   const char *key = childSel->getLabel();
  //   updateLabel(key);
  // }
  // pre_index = cur_index;
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
      // cur_index = (--cur_index < 0) ? device_keys.size() - 1 : cur_index;
      // key = device_keys[cur_index];
      // _menu.moveNext();
      childSel = selector->prev();
      updateLabel(childSel->getLabel());
    }
    // エンコーダ正回転
    if (PlusEncoder.wasUp())
    {
      Serial.println("up");
      // cur_index = (++cur_index >= device_keys.size()) ? 0 : cur_index;
      // key = device_keys[cur_index];
      // _menu.movePrev();
      childSel = selector->next();
      updateLabel(childSel->getLabel());
    }
    // クリック
    if (PlusEncoder.isClick())
    {
      // _menu.selectItem();
      Serial.println("click");
      const char *layer = selector->getLabel();
      Serial.printf("layer: %s, child:%s\n", layer, childSel->getLabel());
      if (strcmp("root", layer) == 0)
      {
        // デバイス選択状態→ボタン選択状態にする
        selector = (SelectorCategory *)childSel;
        const char *deviceKey = childSel->getLabel();
        d = remocon[deviceKey];
        Serial.printf("device:%s, %p\n", deviceKey, d);
        childSel = selector->reset();
        const char *buttonKey = childSel->getLabel();
        updateLabel(buttonKey);
      }
      else
      {
        // ボタン選択状態
        const char *buttonKey = childSel->getLabel();
        if (strcmp("<return>", buttonKey) == 0)
        {
          // デバイス選択状態に戻る
          updateLabel(selector->getLabel());
          selector = (SelectorCategory *)selector->getParent();
          // state = DEVICE_SELECT;
          d = NULL;
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

      // const char *k = childSel->getLabel();
      // // Serial.println(k);
      // if (strcmp("<return>", k) == 0)
      // {
      //   updateLabel(selector->getLabel());
      //   selector = (SelectorCategory *)selector->getParent();
      //   state = DEVICE_SELECT;
      // }
      // else
      // {
      //   if (childSel->isLeaf())
      //   {
      //     const char *key = childSel->getLabel();
      //     if (d != NULL)
      //       d->send(key);
      //   }
      //   else
      //   {
      //     // 小階層があるので、降りる
      //     selector = (SelectorCategory *)childSel;
      //     childSel = selector->reset();
      //     const char *key = childSel->getLabel();
      //     updateLabel(key);
      //     if (state == DEVICE_SELECT)
      //     {
      //       // デバイス選択
      //       d = remocon[key];
      //       state = BUTTON_SELECT;
      //     }
      //   }
      // }
      // if (d != NULL)
      // {
      //   std::vector<const char *> buttonKeys = d->getButtonKeys();
      //   const char *key = buttonKeys[0];
      //   Serial.println(key);
      //   d->send(key);
      //   // for (auto buttonKey : buttonKeys)
      //   // {
      //   //   Serial.println(buttonKey);
      //   //   d->send(buttonKey);
      //   // }
      //   // std::map<const char *, std::vector<const char *>> buttons = d->getButtons();
      //   // for (auto button : buttons)
      //   // {
      //   //   Serial.println(button.first);
      //   // }
      // }
    }
  }
}