#ifndef BLASTOFFSETTINGS_H
#define BLASTOFFSETTINGS_H

#include "Settings.h"

enum ModeSettings {
  Countdown,
  Instant
};

extern const uint32_t settingMagic;

struct settings {
  int mode;
  int valveTime;
  uint8_t countdownTime;
};

extern struct settings currentSettings;

extern const struct MenuItem settingMenuDefs[];
extern const int settingMenuDefsSize;

#endif // BLASTOFSETTINGS_H
