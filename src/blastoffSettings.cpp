#include "blastoffSettings.h"

const uint32_t settingMagic = 0x00000001;

const char * const modeDisplayList[] = { "COUNTDOWN", "INSTANT" };
int valveTimeListValues[] = { 100, 250, 300, 500, 750, 1000 };
int countdownTimeValues[] = { 10, 5, 3 };
int settingsTimeValues[] = { 100, 2000 };

void msecsFormatter(char *buffer, int val) {
  if(val >= 1000) {
    sprintf(buffer, "%.1f S", (float)val / 1000.0f);
  } else {
    sprintf(buffer, "%d ms", val);
  }
}

void secsFormatter(char *buffer, int val) {
  sprintf(buffer, "%d S", val);
}

const struct MenuItem PROGMEM settingMenuDefs[] = {
  { menuSelectFromListIndex, "Mode", &currentSettings.mode, { .setListIndex = { 2, modeDisplayList}}},
  { menuSelectFromIntList, "Vlve Tm", &currentSettings.valveTime, { .setIntListValue = { 6, valveTimeListValues, msecsFormatter}}},
  { menuSelectFromIntList, "Countdown", &currentSettings.countdownTime, { .setIntListValue = {3, countdownTimeValues, secsFormatter}}},
  { menuSelectFromIntList, "Sttng Tm", &currentSettings.settingsDelayTimeMS, { .setIntListValue = { 2, settingsTimeValues, msecsFormatter}}},
  { menuCallMenu, "Exit", NULL, {.menu = 99}},
};

struct settings currentSettings = {settingMagic,  0, 500, 5, 100 };
const int settingMenuDefsSize = sizeof(settingMenuDefs)/sizeof(MenuItem);

