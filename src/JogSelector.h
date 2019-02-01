#ifndef _JOG_SELECTOR_H_
#define _JOG_SELECTOR_H_
#include <Arduino.h>
#include <M5Stack.h>
#include "Selector.h"

#define BAR_WIDTH (8 * 20)
#define BAR_HEIGHT (1 * 20)

class JogSelector
{
private:
  SelectorCategory *_selector;
  TFT_eSprite _stext1 = TFT_eSprite(&M5.Lcd); // Sprite object graph1

public:
  JogSelector(SelectorCategory *selector)
  {
    _selector = selector;
  }
  ~JogSelector() {}
  void init(int x, int y)
  {
    _stext1.setColorDepth(8);
    _stext1.createSprite(BAR_WIDTH, BAR_HEIGHT);
    _stext1.fillSprite(TFT_BLUE);                                         // Fill sprite with blue
    _stext1.setScrollRect(1, 1, BAR_WIDTH - 2, BAR_HEIGHT - 2, TFT_BLUE); // here we set scroll gap fill color to blue
    _stext1.setTextColor(TFT_WHITE);                                      // White text, no background
  }
  void next()
  {
  }
  void prev()
  {
  }
  void execute()
  {
  }
};

#endif