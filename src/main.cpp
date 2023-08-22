#include <Arduino.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_EEPROM_I2C.h>
#include <stdarg.h>

#include <Bounce2.h>

#include <JLed_base.h>

#include "fsm.h"
#include "blastoffSettings.h"
#include "display.h"

Adafruit_EEPROM_I2C i2ceeprom;

settings prevSettings;

#define FLASH_PIN LED_BUILTIN
#define ARMED_LIGHT 7
#define ARMING_PIN 9
#define LAUNCH_PIN 8
#define RELAY_PIN 10

enum Events
{
  SAFETY_OFF,
  SAFETY_ON,
  LAUNCH_COUNTDOWN,
  LAUNCH_INSTANT,
  LAUNCH_ON,
  LAUNCH_OFF,
  ENTER_SETTING,
  ENTER_SETTING_VALUE,
  EXIT_SETTING,
  ENTER_ARMED,
  ABORT_DONE,
  LAUNCH
};

#define SERIAL_PRINTF_MAX_BUFF 256
void serialPrintf(const char *fmt, ...)
{
  /* Buffer for storing the formatted data */
  char buff[SERIAL_PRINTF_MAX_BUFF];
  /* pointer to the variable arguments list */
  va_list pargs;
  /* Initialise pargs to point to the first optional argument */
  va_start(pargs, fmt);
  /* create the formatted data and store in buff */
  vsnprintf(buff, SERIAL_PRINTF_MAX_BUFF, fmt, pargs);
  va_end(pargs);
  Serial.print(buff);
}

int currentSettingIndex = 0;


Bounce safetySwitch;
Bounce launchSwitch;

void on_init_enter(void *)
{
  centerText(F("---"));
}

State state_init(on_init_enter, nullptr, nullptr);
Fsm mainFsm(&state_init);

class ArduinoHal {
 public:
    using PinType = uint8_t;

    explicit ArduinoHal(PinType pin) noexcept : pin_(pin) {}

    void analogWrite(uint8_t val) const {
        // some platforms, e.g. STM need lazy initialization
        if (!setup_) {
            ::pinMode(pin_, OUTPUT);
            setup_ = true;
        }
        ::analogWrite(pin_, val);
    }

    uint32_t millis() const { return ::millis(); }

 private:
    mutable bool setup_ = false;
    PinType pin_;
};

class JLed : public jled::TJLed<ArduinoHal, JLed> {
    using jled::TJLed<ArduinoHal, JLed>::TJLed;
};
auto armedLed = JLed(ARMED_LIGHT).MaxBrightness(currentSettings.maxBrightness); // .Repeat(5);


void on_Launching_enter(void *)
{
  digitalWrite(FLASH_PIN, HIGH);
  digitalWrite(RELAY_PIN, HIGH);
  armedLed.On().Update();
  centerText(F("LAUNCH!"));
}

void on_launching_exit(void *)
{
  digitalWrite(FLASH_PIN, LOW);
  digitalWrite(RELAY_PIN, LOW);
  armedLed.FadeOff(2000).Repeat(1);
}

void on_idle_enter(void *)
{
  centerText(F("Safe"));
  armedLed.Off().Repeat(1);
}

void on_armed_enter(void *)
{
  if(currentSettings.mode == MODE_COUNTDOWN) {
    char buff[20];
    sprintf(buff, "%.2f", (float)currentSettings.countdownTime);

    showCountdownText(F(buff), F("Ready to launch"));
  } else {
    centerText(F("Armed"));
  }
  armedLed.Breathe(currentSettings.breathDurationMS).Forever();
}

void on_armed_exit(void *)
{
  armedLed.Stop();
}

void on_postlaunch_enter(void *)
{
  showLogo();
}

void on_prearm_enter(void *)
{
  centerText(F("..."));
}

void on_prearm(void *)
{
  if (launchSwitch.read())
  {
    mainFsm.trigger(ENTER_ARMED);
  }
}

void on_setting_enter(void *)
{
  showText(F("Setting"));
  memcpy(&prevSettings, &currentSettings, sizeof(settings));
  currentSettingIndex = 0;
}

void on_abort_enter(void *)
{
  centerText(F("ABORTED"));
}


void on_abort_update(void *) {
  if(!armedLed.IsRunning()) {
    mainFsm.trigger(ABORT_DONE, nullptr);
  }
}

void displaySetting(int invertLine) {
 auto &menuDef = settingMenuDefs[currentSettingIndex];

  switch (menuDef.selectType)
  {
    case menuCallMenu:
    {
      showText3(
        F("Setting"),
        F(menuDef.itemText),
        nullptr);
      break;
    }
    case menuSelectFromListIndex:
    {
      int settingIndex = *(int *)(menuDef.settingsValue);
      const char *settingValStr = menuDef.parameters.setListIndex.listPtr[settingIndex];
      // serialPrintf("%d %s\n", settingIndex, settingValStr);
      showText3(
          F("Setting"),
          F(menuDef.itemText),
          F(settingValStr),
          invertLine
        );
      break;
    }
    case menuSelectFromIntList:
    {
      int settingValue = *(int*)(menuDef.settingsValue);
      char buff[20];
      menuDef.parameters.setIntListValue.valueFormatter(buff, settingValue);
      // Look for current setting
      showText3(
        F("Setting"),
        F(menuDef.itemText),
        F(buff),
        invertLine
      );
      break;
    }
  default:
    break;
  }
}

void on_settingname_enter(void *)
{
  // serialPrintf("Setting name: %d %d %s\n", currentSettingIndex, settingMenuDefsSize, settingMenuDefs[currentSettingIndex].itemText);
  if (currentSettingIndex >= settingMenuDefsSize)
  {
    currentSettingIndex = 0;
  }
  displaySetting(1);
}

void on_settingvalue_enter(void *)
{
  displaySetting(2);
}

void nextSettingValue() {
  auto &menuDef = settingMenuDefs[currentSettingIndex];
  switch(menuDef.selectType) {
    case menuSelectFromListIndex:
    {
      int settingIndex = *(int *)(menuDef.settingsValue);
      ++settingIndex;
      if(settingIndex >= menuDef.parameters.setListIndex.listEntryCount) {
        settingIndex = 0;
      }
      *(int*)(menuDef.settingsValue) = settingIndex;
      break;
    }
    case menuSelectFromIntList:
    {
      int currentVal = *(int*)menuDef.settingsValue;
      int i = 0;
      for( i; i < menuDef.parameters.setIntListValue.listEntryCount; ++i ) {
        if(currentVal == menuDef.parameters.setIntListValue.listPtr[i]) {
          break;
        }
      }
      ++i;
      if(i >= menuDef.parameters.setIntListValue.listEntryCount) {
        i = 0;
      }
      *(int*)menuDef.settingsValue = menuDef.parameters.setIntListValue.listPtr[i];
      break;
    }
  }
}

int countdownEnd = 0;
void on_countdown_enter(void *)
{
  countdownEnd = millis() + (1000 * currentSettings.countdownTime);
}

void on_countdown_update(void *)
{
  int countdown = countdownEnd - millis();

  char buff[20];
  sprintf(buff, "%.2f", (float)countdown / 1000.0f);

  showCountdownText(F(buff), F("Launching in"));
  if (safetySwitch.rose())
  {
    mainFsm.trigger(SAFETY_ON);
  }
  if (countdown <= 0)
  {
    mainFsm.trigger(LAUNCH);
  }
}

void on_settingname_update(void *ctx)
{
  if (launchSwitch.rose())
  {
    currentSettingIndex++;
    on_settingname_enter(ctx);
  }
  if (safetySwitch.rose())
  {
    if (settingMenuDefs[currentSettingIndex].selectType == menuCallMenu && settingMenuDefs[currentSettingIndex].parameters.menu == 99)
    {
      mainFsm.trigger(EXIT_SETTING);
    }
    else
    {
      mainFsm.trigger(ENTER_SETTING_VALUE);
    }
  }
}
void on_settingvalue_update(void *ctx)
{
  if (launchSwitch.rose())
  {
    nextSettingValue();
    //    pendingValues[currentSettingIndex]++;
    on_settingvalue_enter(ctx);
  }
  if (safetySwitch.fell())
  {
    mainFsm.trigger(ENTER_SETTING);
  }
}

void on_standard_update(void *)
{
  if (safetySwitch.rose())
  {
    mainFsm.trigger(SAFETY_ON);
  }
  if (safetySwitch.fell())
  {
    mainFsm.trigger(SAFETY_OFF);
  }
  if (launchSwitch.fell())
  {
    mainFsm.trigger(LAUNCH_ON);
  }
  if (launchSwitch.rose())
  {
    mainFsm.trigger(LAUNCH_OFF);
  }
}

void on_arming_enter(void*) {
  centerText(F("Arming"));
  armedLed.FadeOn(currentSettings.armingDelayMS).Repeat(1);
}

void on_armed_update(void *)
{
  if (safetySwitch.rose())
  {
    mainFsm.trigger(SAFETY_ON);
  }
  if (launchSwitch.fell())
  {
    if(currentSettings.mode == MODE_COUNTDOWN) {
      mainFsm.trigger(LAUNCH_COUNTDOWN);
    } else {
      mainFsm.trigger(LAUNCH_INSTANT);
    }
  }
}

void on_exit_setting(void *)
{
  if (memcmp(&prevSettings, &currentSettings, sizeof(settings)) != 0)
  {
    Serial.println("Updating values");

    if(memcmp(&currentSettings, &prevSettings, sizeof(settings))!=0) {
      i2ceeprom.write(0, (uint8_t*)&currentSettings, sizeof(settings));
    }

    armedLed.MaxBrightness(currentSettings.maxBrightness);
  }
}


State state_idle(on_idle_enter, on_standard_update, nullptr);

State state_prearmed(on_prearm_enter, on_prearm, nullptr);

State state_setting(on_setting_enter, on_standard_update, nullptr);
State state_settingname(on_settingname_enter, on_settingname_update, nullptr);
State state_settingvalue(on_settingvalue_enter, on_settingvalue_update, nullptr);

State state_arming(on_arming_enter, on_standard_update, nullptr);

State state_armed(on_armed_enter, on_armed_update, on_armed_exit);

State state_launching(on_Launching_enter, on_standard_update, on_launching_exit);

State state_countdown(on_countdown_enter, on_countdown_update, nullptr);
State state_abort(on_abort_enter, on_abort_update, nullptr);

State state_postlaunch(on_postlaunch_enter, on_standard_update, nullptr);

void setup()
{
  Serial.begin(9600);

  // Initialise eeprom for persistent configuration settings
  if (i2ceeprom.begin(0x50))
  { // you can stick the new i2c addr in here, e.g. begin(0x51);
    Serial.println("Found I2C EEPROM");
  }
  else
  {
    Serial.println("I2C EEPROM not identified ... check your connections?\r\n");
    while (1)
      delay(10);
  }

  displayInit();
  // Read current values and transform any 255's into 0 as 255 is default state for eeprom.
  i2ceeprom.read(0, (uint8_t*)&prevSettings, sizeof(settings));
  if(prevSettings.magic == currentSettings.magic) {
    memcpy(&currentSettings, &prevSettings, sizeof(settings));
  }

  pinMode(FLASH_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(ARMED_LIGHT, OUTPUT);


  safetySwitch.attach(ARMING_PIN, INPUT_PULLUP);
  safetySwitch.interval(25);

  launchSwitch.attach(LAUNCH_PIN, INPUT_PULLUP);
  launchSwitch.interval(25);

  mainFsm.add_timed_transition(&state_init, &state_idle, 50, nullptr);

  mainFsm.add_transition(&state_idle, &state_prearmed, SAFETY_OFF, nullptr);

  mainFsm.add_timed_transition(&state_prearmed, &state_setting, &currentSettings.settingsDelayTimeMS, nullptr);
  mainFsm.add_transition(&state_prearmed, &state_arming, ENTER_ARMED, nullptr);

  mainFsm.add_timed_transition(&state_arming, &state_armed, &currentSettings.armingDelayMS, nullptr);
  mainFsm.add_transition(&state_arming, &state_idle, SAFETY_ON, nullptr);

  mainFsm.add_transition(&state_setting, &state_idle, SAFETY_ON, nullptr);
  mainFsm.add_transition(&state_setting, &state_settingname, LAUNCH_OFF, nullptr);

  mainFsm.add_transition(&state_settingname, &state_settingvalue, ENTER_SETTING_VALUE, nullptr);
  mainFsm.add_transition(&state_settingname, &state_idle, EXIT_SETTING, on_exit_setting);

  mainFsm.add_transition(&state_settingvalue, &state_settingname, ENTER_SETTING, nullptr);

  mainFsm.add_transition(&state_countdown, &state_abort, SAFETY_ON, [](void *)
                         { armedLed.Blink(300,300).Repeat(3); });
  mainFsm.add_transition(&state_countdown, &state_launching, LAUNCH, nullptr);

  mainFsm.add_timed_transition(&state_abort, &state_idle, 3000, nullptr);
  mainFsm.add_transition(&state_abort, &state_idle, ABORT_DONE, nullptr);

  mainFsm.add_transition(&state_armed, &state_countdown, LAUNCH_COUNTDOWN, nullptr);
  mainFsm.add_transition(&state_armed, &state_launching, LAUNCH_INSTANT, nullptr);
  mainFsm.add_transition(&state_armed, &state_idle, SAFETY_ON, nullptr);

  mainFsm.add_timed_transition(&state_launching, &state_postlaunch, &currentSettings.valveTime, nullptr);
  mainFsm.add_transition(&state_postlaunch, &state_idle, SAFETY_ON, nullptr);

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  clearDisplay();
}

void loop()
{
  armedLed.Update();

  launchSwitch.update();
  safetySwitch.update();

  mainFsm.run_machine();
}
