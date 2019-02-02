#include <ArduinoJson.h>
#include <SD.h>
#include <Arduino.h>
#include "Device.h"
// #include "Selector.h"
#include "JogSelector.h"

#define SCROLL (18)
#define SCROLL_X (18)
#define SCROLL_Y (18)
#define SCROLL_STEP_X (8)
#define SCROLL_STEP_Y (1)
#define BAR_WIDTH (SCROLL_STEP_X * SCROLL_X + 2)
#define BAR_HEIGHT (SCROLL_STEP_Y * SCROLL_Y + 2)
#define OFFSET_X (3)
#define OFFSET_Y (SCROLL_Y / 2 - 16 / 2)
#define BAR_X (0)
#define BAR_Y (10)
#define RETURN_KEY "<return>"
#define ROOT_KEY "root"

// TFT_eSprite stext1 = TFT_eSprite(&M5.Lcd); // Sprite object graph1
const char *filename = "/remocon.json";

// void updateLabel(const char *label, int step_x, int step_y)
// {
//   int offset_x = -step_x * SCROLL_X * 8 + OFFSET_X;
//   int offset_y = -step_y * SCROLL_Y + OFFSET_Y;
//   int x = offset_x;
//   int y = offset_y;
//   if (step_x != 0 || step_y != 0)
//   {
//     for (int i = 0; i <= SCROLL; i++)
//     {
//       x = i * step_x * SCROLL_STEP_X + offset_x;
//       y = i * step_y * SCROLL_STEP_Y + offset_y;
//       stext1.drawString(label, x, y, 2);
//       stext1.drawRect(0, 0, BAR_WIDTH, BAR_HEIGHT, TFT_LIGHTGREY);
//       stext1.pushSprite(BAR_X, BAR_Y);
//       stext1.scroll(step_x * SCROLL_STEP_X, step_y * SCROLL_STEP_Y);
//       // M5.Lcd.setCursor(160, 120);
//       // M5.Lcd.printf("(%3d,%3d)", x, y);
//       delay(10);
//     }
//   }
//   stext1.fillSprite(TFT_BLUE);
//   stext1.drawString(label, x, y, 2);
//   stext1.drawRect(0, 0, BAR_WIDTH, BAR_HEIGHT, TFT_LIGHTGREY);
//   stext1.pushSprite(BAR_X, BAR_Y);
// }

SelectorCategory *selector;

std::map<const char *, Device *> remocon;
void callback(SelectorUnit *unit)
{
  const char *deviceKey = unit->getParent()->getLabel();
  const char *buttonKey = unit->getLabel();
  Serial.printf("callback:(%s:%s)¥n", deviceKey, buttonKey);
  remocon[deviceKey]->send(buttonKey);
}
void callbackSelectorDown(SelectorUnit *unit)
{
  // // view
  // SelectorCategory *p = (SelectorCategory *)unit;
  // // p = (SelectorCategory *)p->current();
  // if (!p->isLeaf())
  // {
  //   // こ階層があれば下りる
  //   selector = p;
  //   // childSel = selector->reset();
  //   Serial.printf("%s(%d)", p->getLabel(), p->isLeaf());
  //   updateLabel(selector->reset()->getLabel(), 1, 0);
  // }
  // else
  // {
  //   Serial.println("is leaf !");
  // }
}
void callbackSelectorUp(SelectorUnit *unit)
{
  // // view
  // Serial.println("return");
  // SelectorCategory *p = (SelectorCategory *)unit;
  // // childSel = selector;
  // selector = (SelectorCategory *)p->getParent();
  // updateLabel(selector->getLabel(), -1, 0);
}

Device *d = NULL;
SelectorUnit *childSel = NULL;
JogSelector *js = NULL;

void setup()
{
  M5.begin();
  Wire.begin();

  selector = new SelectorCategory(ROOT_KEY);
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
        SelectorCategory *s2 = new SelectorCategory(b, callback);
        s->addChild(s2);
      }
      ReturnSelector *r = new ReturnSelector();
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
    ((SelectorCategory *)(children[0]))->addChild(new ReturnSelector());
    ((SelectorCategory *)(children[1]))->addChild(new SelectorCategory("menu2-1"));
    ((SelectorCategory *)(children[1]))->addChild(new SelectorCategory("menu2-2"));
    ((SelectorCategory *)(children[1]))->addChild(new SelectorCategory("menu2-3"));
    ((SelectorCategory *)(children[1]))->addChild(new ReturnSelector());
    ((SelectorCategory *)(children[2]))->addChild(new SelectorCategory("menu3-1"));
    ((SelectorCategory *)(children[2]))->addChild(new SelectorCategory("menu3-2"));
    ((SelectorCategory *)(children[2]))->addChild(new SelectorCategory("menu3-3"));
    ((SelectorCategory *)(children[2]))->addChild(new SelectorCategory("menu3-4"));
    ((SelectorCategory *)(children[2]))->addChild(new ReturnSelector());
    children = ((SelectorCategory *)(children[2]))->getChildren();
    ((SelectorCategory *)(children[1]))->addChild(new SelectorCategory("menu3-2-1"));
    ((SelectorCategory *)(children[1]))->addChild(new ReturnSelector());
    Serial.println(children[1]->getLabel());
  }

  M5.Lcd.clear(BLACK);
  js = new JogSelector(selector);
  js->init(BAR_X, BAR_Y);

  // stext1.setColorDepth(8);
  // stext1.createSprite(BAR_WIDTH, BAR_HEIGHT);
  // stext1.fillSprite(TFT_BLUE);                                         // Fill sprite with blue
  // stext1.setScrollRect(1, 1, BAR_WIDTH - 2, BAR_HEIGHT - 2, TFT_BLUE); // here we set scroll gap fill color to blue
  // stext1.setTextColor(TFT_WHITE);                                      // White text, no background
  // // stext1.setTextDatum(TL_DATUM);                            // Bottom right coordinate datum

  // childSel = selector->reset();
  // const char *key = childSel->getLabel();
  // updateLabel(key, 0, 0);
}

void loop()
{
  M5.update();

  js->update();
  delay(100);
  // if (PlusEncoder.update())
  // {
  //   // // 長押し
  //   // if (PlusEncoder.isLongClick())
  //   // {
  //   //   Serial.println("longClick");
  //   // }
  //   // エンコーダ逆回転
  //   if (PlusEncoder.wasDown())
  //   {
  //     Serial.println("down");
  //     // childSel = selector->prev();
  //     // updateLabel(childSel->getLabel(), 0, -1);
  //     updateLabel(selector->prev()->getLabel(), 0, -1);
  //   }
  //   // エンコーダ正回転
  //   if (PlusEncoder.wasUp())
  //   {
  //     Serial.println("up");
  //     // childSel = selector->next();
  //     // updateLabel(childSel->getLabel(), 0, 1);
  //     updateLabel(selector->next()->getLabel(), 0, 1);
  //   }
  //   // クリック
  //   if (PlusEncoder.isClick())
  //   {
  //     Serial.println("click");
  //     selector->current()->getCallback()(selector->current());
  //     // const char *layer = selector->getLabel();
  //     // selector->getCallback()(childSel);
  //     // // Serial.printf("layer: %s, child:%s\n", layer, childSel->getLabel());
  //     // if (strcmp(ROOT_KEY, layer) == 0)
  //     // {
  //     //   // デバイス選択状態→ボタン選択状態にする
  //     //   selector = (SelectorCategory *)childSel;
  //     //   const char *deviceKey = childSel->getLabel();
  //     //   d = remocon[deviceKey];
  //     //   Serial.printf("device:%s, %p\n", deviceKey, d);
  //     //   childSel = selector->reset();
  //     //   const char *buttonKey = childSel->getLabel();
  //     //   updateLabel(buttonKey, 1, 0);
  //     // }
  //     // else
  //     // {
  //     //   // ボタン選択状態
  //     //   const char *buttonKey = childSel->getLabel();
  //     //   if (strcmp(RETURN_KEY, buttonKey) == 0)
  //     //   {
  //     //     // デバイス選択状態に戻る
  //     //     updateLabel(selector->getLabel(), -1, 0);
  //     //     childSel = selector;
  //     //     selector = (SelectorCategory *)selector->getParent();
  //     //     d = NULL;
  //     //     Serial.printf("%p¥n", selector);
  //     //   }
  //     //   else
  //     //   {
  //     //     // ボタン送信
  //     //     if (d != NULL)
  //     //     {
  //     //       d->send(buttonKey);
  //     //     }
  //     //   }
  //     //   Serial.println(buttonKey);
  //     // }
  //   }
  // }
}