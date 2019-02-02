#include <ArduinoJson.h>
#include <SD.h>
#include <Arduino.h>
#include "Device.h"
#include "JogSelector.h"

#define BAR_X (10)
#define BAR_Y (10)
#define ROOT_KEY "root"

std::map<const char *, Device *> remocon;

void irSend(SelectorUnit *unit)
{
  const char *deviceKey = unit->getParent()->getLabel();
  const char *buttonKey = unit->getLabel();
  // Serial.printf("callback:(%s:%s)¥n", deviceKey, buttonKey);
  remocon[deviceKey]->send(buttonKey);
}

SelectorCategory *selector;
const char *filename = "/remocon.json";
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
        SelectorCategory *s2 = new SelectorCategory(b, irSend);
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
    // Serial.println(children[1]->getLabel());
  }

  M5.Lcd.clear(BLACK);
  js = new JogSelector(selector);
  js->init(BAR_X, BAR_Y);
}

void loop()
{
  M5.update();
  js->update();
  delay(100);
}