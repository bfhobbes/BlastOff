#ifndef BLASTOFFSETTINGS_H
#define BLASTOFFSETTINGS_H

#include "Settings.h"

enum ModeSettings {
  MODE_COUNTDOWN,
  MODE_INSTANT
};

struct settings {
  uint32_t magic;
  int mode;
  int valveTime;
  int countdownTime;
  int settingsDelayTimeMS;
  int maxBrightness;
  int armingDelayMS;
  int breathDurationMS;
};

extern struct settings currentSettings;

extern const struct MenuItem settingMenuDefs[];
extern const int settingMenuDefsSize;

#endif // BLASTOFSETTINGS_H
