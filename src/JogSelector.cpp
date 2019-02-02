#include "JogSelector.h"
#include "PLUSEncoder.h"

void JogSelector::update()
{
  if (PlusEncoder.update())
  {
    if (PlusEncoder.wasDown())
    {
      // エンコーダ逆回転
      Serial.println("down");
      prev();
      updateLabel(_selector->getLabel(), 0, -1);
    }
    else if (PlusEncoder.wasUp())
    {
      // エンコーダ正回転
      Serial.println("up");
      next();
      updateLabel(_selector->getLabel(), 0, 1);
    }
    // クリック
    if (PlusEncoder.isClick())
    {
      Serial.println("click");
      const char *label = _selector->getLabel();
      if (strcmp(RETURN_KEY, label) == 0)
      {
        // 上の階層へ
        _selector = _selector->getParent();
        updateLabel(_selector->getLabel(), -1, 0);
      }
      else
      {
        if (_selector->isLeaf())
        {
          // Serial.println("action !");
          std::function<void(SelectorUnit *)> _callback = _selector->getCallback();
          if (_callback != NULL)
            _callback(_selector);
        }
        else
        {
          // 下の階層へ
          _selector = (SelectorCategory *)_selector->reset();
          updateLabel(_selector->getLabel(), 1, 0);
        }
      }
    }
  }
}

TFT_eSprite _stext1 = TFT_eSprite(&M5.Lcd); // Sprite object graph1
void JogSelector::init(int x, int y)
{
  _x = x;
  _y = y;
  _stext1.setColorDepth(8);
  _stext1.createSprite(BAR_WIDTH, BAR_HEIGHT);
  _stext1.fillSprite(TFT_BLUE);                                         // Fill sprite with blue
  _stext1.setScrollRect(1, 1, BAR_WIDTH - 2, BAR_HEIGHT - 2, TFT_BLUE); // here we set scroll gap fill color to blue
  _stext1.setTextColor(TFT_WHITE);                                      // White text, no background
  // stext1.setTextDatum(TL_DATUM);                            // Bottom right coordinate datum
  updateLabel(_selector->getLabel(), 0, 0);
}

void JogSelector::updateLabel(const char *label, int step_x, int step_y)
{
  int offset_x = -step_x * SCROLL_X * 8 + OFFSET_X;
  int offset_y = -step_y * SCROLL_Y + OFFSET_Y;
  int x = offset_x;
  int y = offset_y;
  if (step_x != 0 || step_y != 0)
  {
    for (int i = 0; i <= SCROLL; i++)
    {
      x = i * step_x * SCROLL_STEP_X + offset_x;
      y = i * step_y * SCROLL_STEP_Y + offset_y;
      _stext1.drawString(label, x, y, 2);
      _stext1.drawRect(0, 0, BAR_WIDTH, BAR_HEIGHT, TFT_LIGHTGREY);
      _stext1.pushSprite(_x, _y);
      _stext1.scroll(step_x * SCROLL_STEP_X, step_y * SCROLL_STEP_Y);
      // M5.Lcd.setCursor(160, 120);
      // M5.Lcd.printf("(%3d,%3d)", x, y);
      delay(10);
    }
  }
  _stext1.fillSprite(TFT_BLUE);
  _stext1.drawString(label, x, y, 2);
  _stext1.drawRect(0, 0, BAR_WIDTH, BAR_HEIGHT, TFT_LIGHTGREY);
  _stext1.pushSprite(_x, _y);
}

void JogSelector::prev()
{
  SelectorCategory *parent = _selector->getParent();
  parent->prev();
  _selector = (SelectorCategory *)parent->current();
}

void JogSelector::next()
{
  SelectorCategory *parent = _selector->getParent();
  parent->next();
  _selector = (SelectorCategory *)parent->current();
}
