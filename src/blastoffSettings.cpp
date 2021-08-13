#include "blastoffSettings.h"

const uint32_t settingMagic = 0x00000001;

char modeDisplayList[][10] = { "COUNTDOWN", "INSTANT" };
int valveTimeListValues[] = { 100, 250, 300, 500, 750, 1000 };
int countdownTimeValues[] = { 10, 5, 3 };

void msecsFormatter(char *buffer, int val) {
  sprintf(buffer, "%d", val);
}

void secsFormatter(char *buffer, int val) {
  sprintf(buffer, "%d", val);
}

const struct MenuItem PROGMEM settingMenuDefs[] = {
  { menuSelectFromListIndex, "Mode", &currentSettings.mode, { .setListIndex = { 2, (char*)&modeDisplayList}}},
  { menuSelectFromIntList, "Valve Time", &currentSettings.valveTime, { .setIntListValue = { 6, valveTimeListValues, msecsFormatter}}},
  { menuSelectFromIntList, "Countdown", &currentSettings.countdownTime, { .setIntListValue = {3, countdownTimeValues, secsFormatter}}},
  { menuCallMenu, "Exit", NULL, {.menu = 99}},
};

struct settings currentSettings = { 0, 500, 5 };
const int settingMenuDefsSize = sizeof(settingMenuDefs)/sizeof(MenuItem);

