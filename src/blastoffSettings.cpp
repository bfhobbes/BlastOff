#include "blastoffSettings.h"

const uint32_t settingMagic = 0x00000004;

const char * const modeDisplayList[] = { "COUNTDOWN", "INSTANT" };
int armDelayValues[] = { 0, 50, 500, 1000, 1500, 2000, 3000 };
int valveTimeListValues[] = { 100, 250, 300, 500, 750, 1000 };
int countdownTimeValues[] = { 10, 5, 3, 1, 0 };
int brightnessValues[] = { 0, 32, 64, 96, 128, 160, 192, 224, 255 };
int settingsTimeValues[] = { 100, 2000, 5000, 8000 };
int breatheDurationValues[] = { 250, 500, 1000, 1500, 2000, 3000 };

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

void brightnessFormatter(char *buffer, int val) {
  sprintf(buffer, "%d%%", (int)(100.0 * ((float)val)/255.0));
}

const struct MenuItem PROGMEM settingMenuDefs[] = {
  { menuSelectFromListIndex, "Mode", &currentSettings.mode, { .setListIndex = { 2, modeDisplayList}}},
  { menuSelectFromIntList, "Arm Dly", &currentSettings.armingDelayMS, { .setIntListValue = { 7, armDelayValues, msecsFormatter}}},
  { menuSelectFromIntList, "Vlve Tm", &currentSettings.valveTime, { .setIntListValue = { 6, valveTimeListValues, msecsFormatter}}},
  { menuSelectFromIntList, "Countdown", &currentSettings.countdownTime, { .setIntListValue = {5, countdownTimeValues, secsFormatter}}},
  { menuSelectFromIntList, "Brth Dur", &currentSettings.breathDurationMS, { .setIntListValue = { 6, breatheDurationValues, msecsFormatter}}},
  { menuSelectFromIntList, "Max Brt", &currentSettings.maxBrightness, { .setIntListValue = { 9, brightnessValues, brightnessFormatter}}},
  { menuSelectFromIntList, "Sttng Tm", &currentSettings.settingsDelayTimeMS, { .setIntListValue = { 4, settingsTimeValues, msecsFormatter}}},

  { menuCallMenu, "Exit", NULL, {.menu = 99}},
};

struct settings currentSettings = {settingMagic,  0, 500, 5, 100, 128, 1500, 1500 };
const int settingMenuDefsSize = sizeof(settingMenuDefs)/sizeof(MenuItem);

