#ifndef _JOG_SELECTOR_H_
#define _JOG_SELECTOR_H_
#include <Arduino.h>
#include <M5Stack.h>
#include "Selector.h"

#define SCROLL_X (18)
#define SCROLL_Y (18)
#define SCROLL_STEP_X (8)
#define SCROLL_STEP_Y (1)
#define BAR_WIDTH (SCROLL_STEP_X * SCROLL_X + 2)
#define BAR_HEIGHT (SCROLL_STEP_Y * SCROLL_Y + 2)
#define OFFSET_X (3)
#define OFFSET_Y (SCROLL_Y / 2 - 16 / 2)

class JogSelector
{
private:
  int _x;
  int _y;
  SelectorCategory *_selector;
  void updateLabel(const char *label, int step_x, int step_y);

public:
  JogSelector(SelectorCategory *root)
  {
    // Serial.println(root->current()->getLabel());
    _selector = (SelectorCategory *)root->current();
  }
  ~JogSelector() {}
  void init(int x, int y);
  void update();
  void next();
  void prev();
};

#endif