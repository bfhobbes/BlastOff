#include <Arduino.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_EEPROM_I2C.h>
#include <stdarg.h>

#include <Bounce2.h>

#include "fsm.h"
#include "blastoffSettings.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
//#define SCREEN_ADDRESS 0x3D ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Adafruit_EEPROM_I2C i2ceeprom;

#define FLASH_PIN LED_BUILTIN
#define ARMED_LIGHT 7
#define ARMING_PIN 9
#define LAUNCH_PIN 8
#define RELAY_PIN 10

enum Events
{
  SAFETY_OFF,
  SAFETY_ON,
  LAUNCH_ON,
  LAUNCH_OFF,
  ENTER_SETTING,
  ENTER_SETTING_VALUE,
  EXIT_SETTING,
  ENTER_ARMED,
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

int currentSetting = 0;

void showText(const arduino::__FlashStringHelper *text)
{
  display.clearDisplay();
  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 0);
  display.println(text);
  display.display(); // Show initial text
}

void showCountdownText(const arduino::__FlashStringHelper *text)
{
  display.clearDisplay();
  display.setTextSize(3); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(30, 20);
  display.println(text);
  display.display(); // Show initial text
}

void showText3(const arduino::__FlashStringHelper *text1, const arduino::__FlashStringHelper *text2, const arduino::__FlashStringHelper *text3)
{
  display.clearDisplay();
  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  if (text1)
  {
    display.setCursor(10, 0);
    display.println(text1);
  }
  if (text2)
  {
    if (text3 == nullptr)
    {
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
    }
    display.setCursor(10, 22);
    display.println(text2);
  }
  if (text3)
  {
    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
    display.setCursor(10, 44);
    display.println(text3);
  }
  display.display(); // Show initial text
}

Bounce safetySwitch;
Bounce launchSwitch;

void on_init_enter(void *)
{
  showText(F("init"));
}

State state_init(on_init_enter, nullptr, nullptr);
Fsm mainFsm(&state_init);

enum LightEvents
{
  LIGHT_OFF,
  LIGHT_ON,
  LIGHT_STROBE
};

State light_off([](void *)
                { digitalWrite(ARMED_LIGHT, LOW); },
                nullptr, nullptr);
State light_on([](void *)
               { digitalWrite(ARMED_LIGHT, HIGH); },
               nullptr, nullptr);
Fsm lightFsm(&light_off);

int strobeCount = 0;
State light_strobe_on(
    [](void *)
    {
      digitalWrite(ARMED_LIGHT, HIGH);
      --strobeCount;
    },
    nullptr,
    nullptr);
State light_strobe_off(
    [](void *)
    { digitalWrite(ARMED_LIGHT, LOW); },
    [](void *)
    {
      if (strobeCount <= 0)
      {
        lightFsm.trigger(LIGHT_OFF);
      }
    },
    nullptr);

void on_light_on_enter(void *)
{
  digitalWrite(FLASH_PIN, HIGH);
}

void on_light_off_enter(void *)
{
  digitalWrite(FLASH_PIN, LOW);
}

void on_Launching_enter(void *)
{
  digitalWrite(FLASH_PIN, HIGH);
  digitalWrite(RELAY_PIN, HIGH);
  showCountdownText(F("LAUNCH"));
}

void on_launching_exit(void *)
{
  digitalWrite(FLASH_PIN, LOW);
  digitalWrite(RELAY_PIN, LOW);
}

void on_idle_enter(void *)
{
  showText(F("idle"));
}

void on_armed_enter(void *)
{
  lightFsm.trigger(LIGHT_ON);

  showText(F("armed"));
}

void on_armed_exit(void *)
{
  lightFsm.trigger(LIGHT_OFF);
}

void on_postlaunch_enter(void *)
{
  showText(F("postlaunch"));
}

void on_prearm_enter(void *)
{
  showText(F("prearmed"));
}

void on_prearm(void *)
{
  if (launchSwitch.read())
  {
    mainFsm.trigger(ENTER_ARMED);
  }
}

settings prevSettings;

void on_setting_enter(void *)
{
  showText(F("setting"));
  memcpy(&prevSettings, &currentSettings, sizeof(settings));
  currentSetting = 0;
}

void on_abort_enter(void *)
{
  showText(F("ABORT"));
}

void on_settingname_enter(void *)
{
  serialPrintf("Setting name: %d %d %s\n", currentSetting, settingMenuDefsSize, settingMenuDefs[currentSetting].itemText);
  if (currentSetting >= settingMenuDefsSize)
  {
    currentSetting = 0;
  }
  showText3(
      F("Setting"),
      F(settingMenuDefs[currentSetting].itemText),
      nullptr);
}

void on_settingvalue_enter(void *)
{
  auto &menuDef = settingMenuDefs[currentSetting];

  switch (menuDef.selectType)
  {
    case menuSelectFromListIndex:
    {
      int settingIndex = *(int *)(menuDef.settingsValue);
      const char *settingValStr = menuDef.parameters.setListIndex.listPtr[settingIndex];
      serialPrintf("%d %s", settingIndex, settingValStr);
      showText3(
          F("Setting"),
          F(menuDef.itemText),
          F(settingValStr));
      break;
    }
  default:
    break;
  }

  // uint8_t currentValue = pendingValues[currentSetting];
  // if(settings[currentSetting].values[currentValue] == nullptr) {
  //   Serial.println("Resetting Value");
  //   currentValue = 0;
  //   pendingValues[currentSetting] = 0;
  // }
  // showText3(
  //   F("Setting"),
  //   F(settings[currentSetting].Name),
  //   F(settings[currentSetting].values[currentValue])
  // );
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

  showCountdownText(F(buff));
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
    currentSetting++;
    on_settingname_enter(ctx);
  }
  if (safetySwitch.rose())
  {
    if (settingMenuDefs[currentSetting].selectType == menuCallMenu && settingMenuDefs[currentSetting].parameters.menu == 99)
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
    Serial.println("FIX THIS");
    //    pendingValues[currentSetting]++;
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

void on_exit_setting(void *)
{
  if (memcmp(&prevSettings, &currentSettings, sizeof(prevSettings)) != 0)
  {
    Serial.println("Updating values");

    memcpy(&currentSettings, &prevSettings, sizeof(prevSettings));
    //i2ceeprom.write(0, currentSettings, sizeof(prevSettings));
  }
}

State state_idle(on_idle_enter, on_standard_update, nullptr);

State state_prearmed(on_prearm_enter, on_prearm, nullptr);

State state_setting(on_setting_enter, on_standard_update, nullptr);
State state_settingname(on_settingname_enter, on_settingname_update, nullptr);
State state_settingvalue(on_settingvalue_enter, on_settingvalue_update, nullptr);

State state_armed(on_armed_enter, on_standard_update, on_armed_exit);

State state_launching(on_Launching_enter, on_standard_update, on_launching_exit);

State state_countdown(on_countdown_enter, on_countdown_update, nullptr);
State state_abort(on_abort_enter, on_standard_update, nullptr);

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

  // Read current values and transform any 255's into 0 as 255 is default state for eeprom.
  // i2ceeprom.read(0, &currentSettings, sizeof(settings));
  // for(size_t i = 0; i < SETTING_MAX; ++i) {
  //   if(settingValues[i] == 255) {
  //     settingValues[i] = 0;
  //   }
  // }

  pinMode(FLASH_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(ARMED_LIGHT, OUTPUT);

  safetySwitch.attach(ARMING_PIN, INPUT_PULLUP);
  safetySwitch.interval(25);

  launchSwitch.attach(LAUNCH_PIN, INPUT_PULLUP);
  launchSwitch.interval(25);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }

  lightFsm.add_transition(&light_off, &light_on, LIGHT_ON, nullptr);
  lightFsm.add_transition(&light_on, &light_off, LIGHT_OFF, nullptr);

  lightFsm.add_transition(&light_off, &light_strobe_on, LIGHT_STROBE, [](void *)
                          { strobeCount = 3; });
  lightFsm.add_transition(&light_strobe_off, &light_off, LIGHT_OFF, nullptr);
  lightFsm.add_timed_transition(&light_strobe_off, &light_strobe_on, 300, nullptr);
  lightFsm.add_timed_transition(&light_strobe_on, &light_strobe_off, 300, nullptr);

  mainFsm.add_timed_transition(&state_init, &state_idle, 50, nullptr);

  mainFsm.add_transition(&state_idle, &state_prearmed, SAFETY_OFF, nullptr);

  mainFsm.add_timed_transition(&state_prearmed, &state_setting, 1000, nullptr);
  mainFsm.add_transition(&state_prearmed, &state_armed, ENTER_ARMED, nullptr);

  mainFsm.add_transition(&state_setting, &state_idle, SAFETY_ON, nullptr);
  mainFsm.add_transition(&state_setting, &state_settingname, LAUNCH_OFF, nullptr);

  mainFsm.add_transition(&state_settingname, &state_settingvalue, ENTER_SETTING_VALUE, nullptr);
  mainFsm.add_transition(&state_settingname, &state_idle, EXIT_SETTING, on_exit_setting);

  mainFsm.add_transition(&state_settingvalue, &state_settingname, ENTER_SETTING, nullptr);

  mainFsm.add_transition(&state_countdown, &state_abort, SAFETY_ON, [](void *)
                         { lightFsm.trigger(LIGHT_STROBE); });
  mainFsm.add_transition(&state_countdown, &state_launching, LAUNCH, nullptr);

  mainFsm.add_timed_transition(&state_abort, &state_idle, 3000, nullptr);

  mainFsm.add_transition(&state_armed, &state_countdown, LAUNCH_ON, nullptr);
  mainFsm.add_transition(&state_armed, &state_idle, SAFETY_ON, nullptr);

  mainFsm.add_timed_transition(&state_launching, &state_postlaunch, 300, nullptr);
  mainFsm.add_transition(&state_postlaunch, &state_idle, SAFETY_ON, nullptr);

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.clearDisplay();
  display.display();
}

void loop()
{
  launchSwitch.update();
  safetySwitch.update();

  mainFsm.run_machine();
  lightFsm.run_machine();
}
