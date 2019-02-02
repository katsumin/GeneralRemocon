#ifndef _SELECTOR_H_
#define _SELECTOR_H_
#include <Arduino.h>
#include <M5Stack.h>
#include <functional>

#define RETURN_KEY "<return>"

class SelectorCategory;
class SelectorUnit
{
private:
  const char *_label;
  SelectorCategory *_parent;
  std::function<void(SelectorUnit *)> _callback = NULL;

public:
  SelectorUnit(const char *label) : _label(label), _parent(NULL) {}
  SelectorUnit(const char *label, std::function<void(SelectorUnit *)> callback) : _label(label), _callback(callback) {}
  const char *getLabel() { return _label; }
  void setLabel(const char *label) { _label = label; }
  SelectorCategory *getParent() { return _parent; }
  void setParent(SelectorCategory *parent) { _parent = parent; }
  std::function<void(SelectorUnit *)> getCallback() { return _callback; }
  void setCallback(std::function<void(SelectorUnit *)> callback) { _callback = callback; }
  virtual bool isLeaf() { return true; }

protected:
  char *duplicateString(const char *src)
  {
    char *buf = new char[strlen(src)];
    strcpy(buf, src);
    return buf;
  }
};

class ReturnSelector : public SelectorUnit
{
public:
  ReturnSelector() : SelectorUnit(RETURN_KEY) {}
};

class SelectorCategory : public SelectorUnit
{
private:
  std::vector<SelectorUnit *> _children;
  int _index;

public:
  SelectorCategory(const char *label) : SelectorUnit(label), _index(0) {}
  SelectorCategory(const char *label, std::function<void(SelectorUnit *)> callback) : SelectorUnit(label, callback), _index(0) {}
  void addChild(SelectorUnit *child)
  {
    _children.push_back(child);
    child->setParent(this);
  }
  std::vector<SelectorUnit *> getChildren() { return _children; }
  SelectorUnit *current() { return isLeaf() ? NULL : _children[_index]; }
  SelectorUnit *reset()
  {
    _index = 0;
    return current();
  }
  SelectorUnit *next()
  {
    if (++_index >= (int)(_children.size()))
    {
      _index = 0;
    }
    return current();
  }
  SelectorUnit *prev()
  {
    if (--_index < 0)
    {
      _index = _children.size() - 1;
    }
    return current();
  }
  virtual bool isLeaf() { return _children.empty(); }
};

#endif