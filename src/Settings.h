#ifndef SETTINGS_H
#define SETTINGS_H
#include <Arduino.h>

enum menuActions {
  menuCallMenu,
  menuSelectFromList,
  menuSelectFromListIndex,
  menuSelectFromIntList,
  menuBlank
};

struct SetValue {
  bool valType;
  uint8_t minValue;
  uint8_t maxValue;
};

struct SetFromListIndex 
{
  uint8_t listEntryCount;
  char *listPtr[10];
};

struct SetFromList 
{
  uint8_t listEntryCount;
  int *valuePtr;
  char *listPtr;
};

struct SetFromIntList
{
  uint8_t listEntryCount;
  int *listPtr;
  void (*valueFormatter)(char *buffer, int val);
};

union itemParams {
  void (*selectFunction)();
  uint8_t menu;

  struct SetValue setItemValue;
  struct SetFromListIndex setListIndex;
  struct SetFromList setListValue;
  struct SetFromIntList setIntListValue;
};

struct MenuItem
{
  uint8_t selectType;
  char itemText[18];

  void *settingsValue;
  union itemParams parameters;
};


#endif // SETTINGS_H