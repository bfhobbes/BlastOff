#include "blastoffSettings.h"

char modeDisplayList[][10] = { "COUNTDOWN", "INSTANT" };
char valveTimeList[][8] = {"100ms", "250ms", "300ms", "500ms", "750ms", "1s"};
int valveTimeListValues[] = {100,250,300,500,750,1000};
char countdownTimeList[][4] = { "10s", "5s", "3s"};
int countdownTimeValues[] = { 10, 5, 3};

const struct MenuItem PROGMEM settingMenuDefs[] = {
  { menuSelectFromListIndex, "Mode", &currentSettings.mode, { .setListIndex = { 2, (char*)&modeDisplayList}}},
  { menuSelectFromList, "Valve Time", &currentSettings.valveTime, { .setListValue = { 6, valveTimeListValues, (char*)&valveTimeList}}},
  { menuSelectFromList, "Countdown", &currentSettings.countdownTime, { .setListValue = {3, countdownTimeValues, (char*)&countdownTimeList}}},
  { menuCallMenu, "Exit", NULL, {.menu = 99}},
};

struct settings currentSettings = { 0, 500, 5 };
const int settingMenuDefsSize = sizeof(settings)/sizeof(MenuItem);

