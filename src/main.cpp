#include <SD.h>
#include <Arduino.h>
#include "PLUSEncoder.h"

void setup()
{
  M5.begin();
  Wire.begin();
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