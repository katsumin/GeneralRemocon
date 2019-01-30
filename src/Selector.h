#ifndef _SELECTOR_H_
#define _SELECTOR_H_
#include <Arduino.h>
#include <M5Stack.h>

class SelectorCategory;
class SelectorUnit
{
private:
  const char *_label;
  SelectorCategory *_parent;

public:
  SelectorUnit(const char *label)
  {
    setLabel(label);
    setParent(NULL);
  }
  const char *getLabel() { return _label; }
  void setLabel(const char *label) { _label = label; }
  SelectorCategory *getParent() { return _parent; }
  void setParent(SelectorCategory *parent) { _parent = parent; }
  virtual boolean isLeaf() { return true; }

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
  ReturnSelector(const char *label) : SelectorUnit(label) {}
};

class SelectorCategory : public SelectorUnit
{
private:
  std::vector<SelectorUnit *> _children;
  int _index;

public:
  SelectorCategory(const char *label) : SelectorUnit(label) { _index = 0; }
  void addChild(SelectorUnit *child)
  {
    _children.push_back(child);
    child->setParent(this);
  }
  std::vector<SelectorUnit *> getChildren() { return _children; }
  SelectorUnit *reset()
  {
    _index = 0;
    return _children[_index];
  }
  SelectorUnit *next()
  {
    if (++_index >= (int)(_children.size()))
    {
      _index = 0;
    }
    return _children[_index];
  }
  SelectorUnit *prev()
  {
    if (--_index < 0)
    {
      _index = _children.size() - 1;
    }
    return _children[_index];
  }
  virtual boolean isLeaf() { return _children.empty(); }
};

#endif