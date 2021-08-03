#include <Arduino.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_EEPROM_I2C.h>

#include <Bounce2.h>

#include "fsm.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Adafruit_EEPROM_I2C i2ceeprom;

#define FLASH_PIN LED_BUILTIN
#define ARMED_LIGHT  7
#define ARMING_PIN 9
#define LAUNCH_PIN 8
#define RELAY_PIN 10

enum Events {
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

struct SettingInfo {
  const char *Name;
  const char *values[5];
};

enum SettingName {
  VALVE_TIME,
  LAUNCH_MODE,
  COUNTDOWN_TIMER,
  AUDIO,
  SETTING_MAX
};



static const SettingInfo settings[] = {
  {
    "Valve Time", { "1000ms", "750ms", "500ms", "300ms", nullptr }
  },
  {
    "Lnch Mode", { "Instant", "Count",  nullptr }
  },
  {
    "Cnt Timer", { "10", "5", "3", nullptr }
  },
  {
    "Exit", { nullptr }
  }
};

uint8_t settingValues[SETTING_MAX];
uint8_t pendingValues[SETTING_MAX];

int currentSetting = 0;

const int NUM_SETTINGS = sizeof(settings)/sizeof(SettingInfo);

void showText(const arduino::__FlashStringHelper *text) {
  display.clearDisplay();
  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 0);
  display.println(text);
  display.display();      // Show initial text
}

void showCountdownText(const arduino::__FlashStringHelper *text) {
  display.clearDisplay();
  display.setTextSize(3); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(30, 20);
  display.println(text);
  display.display();      // Show initial text
}

void showText3(const arduino::__FlashStringHelper *text1, const arduino::__FlashStringHelper *text2, const arduino::__FlashStringHelper *text3) {
  display.clearDisplay();
  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  if(text1) {
    display.setCursor(10, 0);
    display.println(text1);
  }
  if(text2) {
    if(text3 == nullptr) {
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
    }
    display.setCursor(10, 21);
    display.println(text2);
  }
  if(text3) {
    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
    display.setCursor(10, 42);
    display.println(text3);
  }
  display.display();      // Show initial text
}

int getNumberValue(int setting) {
  int val = 0;
  sscanf(settings[setting].values[settingValues[setting]], "%d", &val);
  return val;
}

Bounce safetySwitch;
Bounce launchSwitch;

void on_init_enter(void *) {
  showText(F("init"));
}

State state_init(on_init_enter, nullptr, nullptr);
Fsm mainFsm(&state_init);


enum LightEvents {
  LIGHT_OFF, 
  LIGHT_ON,
  LIGHT_STROBE
};

State light_off([](void *) {digitalWrite(ARMED_LIGHT, LOW);}, nullptr, nullptr );
State light_on([](void *) {digitalWrite(ARMED_LIGHT, HIGH);}, nullptr, nullptr );
Fsm lightFsm(&light_off);

int strobeCount = 0;
State light_strobe_on( 
  [](void *) {digitalWrite(ARMED_LIGHT, HIGH); --strobeCount; }, 
  nullptr,
  nullptr);
State light_strobe_off( 
  [](void *) {digitalWrite(ARMED_LIGHT, LOW);}, 
  [](void *) { if(strobeCount<=0) {lightFsm.trigger(LIGHT_OFF);} }, 
  nullptr);


void on_light_on_enter(void *) {
  digitalWrite(FLASH_PIN, HIGH);
}

void on_light_off_enter(void *) {
  digitalWrite(FLASH_PIN, LOW);
}

void on_Launching_enter(void *) {
  digitalWrite(FLASH_PIN, HIGH);
  digitalWrite(RELAY_PIN, HIGH);
  showCountdownText(F("LAUNCH"));
}

void on_launching_exit(void *) {
  digitalWrite(FLASH_PIN, LOW);
  digitalWrite(RELAY_PIN, LOW);
}

void on_idle_enter(void *) {
  showText(F("idle"));
}

void on_armed_enter(void *) {
  lightFsm.trigger(LIGHT_ON);
  
  showText(F("armed"));
}

void on_armed_exit(void *) {
  lightFsm.trigger(LIGHT_OFF);
}

void on_postlaunch_enter(void *) {
  showText(F("postlaunch"));
}

void on_prearm_enter(void *) {
  showText(F("prearmed"));
}

void on_prearm(void *) {
  if(launchSwitch.read()) {
    mainFsm.trigger(ENTER_ARMED);
  }
}

void on_setting_enter(void *) {
  showText(F("setting"));
}

void on_abort_enter(void *) {
  showText(F("ABORT"));
}

void on_settingname_enter(void *) {
  if(currentSetting >= NUM_SETTINGS) {
    currentSetting = 0;
  }
  showText3(
    F("Setting"),
    F(settings[currentSetting].Name),
    nullptr
  );
}

void on_settingvalue_enter(void *) {
  uint8_t currentValue = pendingValues[currentSetting];
  if(settings[currentSetting].values[currentValue] == nullptr) {
    currentValue = 0;
    pendingValues[currentSetting] = 0;
  }
  showText3(
    F("Setting"),
    F(settings[currentSetting].Name),
    F(settings[currentSetting].values[currentValue])
  );
}

int countdownEnd = 0;
void on_countdown_enter(void *) {
  countdownEnd = millis() + (1000 * getNumberValue(COUNTDOWN_TIMER));
}

void on_countdown_update(void *) {
  int countdown = countdownEnd - millis();
  
  char buff[20];
  sprintf(buff, "%.2f", (float)countdown / 1000.0f);

  showCountdownText(F(buff));
  if(safetySwitch.rose()) {
    mainFsm.trigger(SAFETY_ON);
  }
  if(countdown <= 0) {
    mainFsm.trigger(LAUNCH);
  }
}

void on_settingname_update(void *ctx) {
  if(launchSwitch.rose()) {
    currentSetting++;
    on_settingname_enter(ctx);    
  }
  if(safetySwitch.rose()) {
    if(strcmp(settings[currentSetting].Name, "Exit")==0) {
      mainFsm.trigger(EXIT_SETTING);
    } else {
      mainFsm.trigger(ENTER_SETTING_VALUE);
    }
  }
}
void on_settingvalue_update(void *ctx) {
  if(launchSwitch.rose()) {
    pendingValues[currentSetting]++;
    on_settingvalue_enter(ctx);    
  }
  if(safetySwitch.fell()) {
    mainFsm.trigger(ENTER_SETTING);
  }
}



void on_standard_update(void*){
  if(safetySwitch.rose()) {
    mainFsm.trigger(SAFETY_ON);
  }
  if(safetySwitch.fell()) {
    mainFsm.trigger(SAFETY_OFF);
  }
  if(launchSwitch.fell()) {
    mainFsm.trigger(LAUNCH_ON);
  }
  if(launchSwitch.rose()) {
    mainFsm.trigger(LAUNCH_OFF);
  }
}

void on_enter_setting(void*) {
  memcpy(pendingValues, settingValues, SETTING_MAX * sizeof(uint8_t));
}

void on_exit_setting(void*) {
  if(memcmp(pendingValues, settingValues, SETTING_MAX * sizeof(uint8_t)) != 0) {
    Serial.println("Updating values");

    memcpy(settingValues, pendingValues, SETTING_MAX * sizeof(uint8_t));
    i2ceeprom.write(0, settingValues, sizeof(uint8_t) * SETTING_MAX);
  }
}

State state_idle(on_idle_enter, on_standard_update, nullptr);

State state_prearmed(on_prearm_enter, on_prearm, nullptr);

State state_setting(on_setting_enter, on_standard_update, nullptr);
State state_settingname(on_settingname_enter, on_settingname_update, nullptr);
State state_settingvalue(on_settingvalue_enter, on_settingvalue_update, nullptr);

State state_armed(on_armed_enter, on_standard_update, on_armed_exit);

State state_launching(on_Launching_enter, on_standard_update, on_launching_exit);

State state_countdown(on_countdown_enter, on_countdown_update, nullptr );
State state_abort(on_abort_enter, on_standard_update, nullptr);

State state_postlaunch(on_postlaunch_enter, on_standard_update, nullptr);


void setup() {

  Serial.begin(9600);

  if (i2ceeprom.begin(0x50)) {  // you can stick the new i2c addr in here, e.g. begin(0x51);
    Serial.println("Found I2C EEPROM");
  } else {
    Serial.println("I2C EEPROM not identified ... check your connections?\r\n");
    while (1) delay(10);
  }

  i2ceeprom.read(0, settingValues, sizeof(uint8_t) * SETTING_MAX);
  for(size_t i = 0; i < SETTING_MAX; ++i) {
    if(settingValues[i] == 255) {
      settingValues[i] = 0;
    }
  }

  bool read(uint16_t addr, uint8_t *buffer, uint16_t num);
  memset(settingValues, 0, SETTING_MAX * sizeof(uint8_t) );


  pinMode(FLASH_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(ARMED_LIGHT, OUTPUT);
  
  safetySwitch.attach(ARMING_PIN, INPUT_PULLUP);
  safetySwitch.interval(25);

  launchSwitch.attach(LAUNCH_PIN, INPUT_PULLUP);
  launchSwitch.interval(25);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  
  lightFsm.add_transition(&light_off, &light_on, LIGHT_ON, nullptr);
  lightFsm.add_transition(&light_on, &light_off, LIGHT_OFF, nullptr);

  lightFsm.add_transition(&light_off, &light_strobe_on, LIGHT_STROBE, [](void *) {strobeCount = 3;});
  lightFsm.add_transition(&light_strobe_off, &light_off, LIGHT_OFF, nullptr);
  lightFsm.add_timed_transition(&light_strobe_off, &light_strobe_on, 300, nullptr);
  lightFsm.add_timed_transition(&light_strobe_on, &light_strobe_off, 300, nullptr ) ;

  mainFsm.add_timed_transition(&state_init, &state_idle, 50, nullptr);

  mainFsm.add_transition(&state_idle, &state_prearmed, SAFETY_OFF, nullptr);

  mainFsm.add_timed_transition(&state_prearmed, &state_setting, 3000, nullptr);
  mainFsm.add_transition(&state_prearmed, &state_armed, ENTER_ARMED, nullptr);

  mainFsm.add_transition(&state_setting, &state_idle, SAFETY_ON, on_enter_setting);
  mainFsm.add_transition(&state_setting, &state_settingname, LAUNCH_OFF, nullptr);

  mainFsm.add_transition(&state_settingname, &state_settingvalue, ENTER_SETTING_VALUE, nullptr ); 
  mainFsm.add_transition(&state_settingname, &state_idle, EXIT_SETTING, on_exit_setting);

  mainFsm.add_transition(&state_settingvalue, &state_settingname, ENTER_SETTING, nullptr);

  mainFsm.add_transition(&state_countdown, &state_abort, SAFETY_ON, [](void *) { lightFsm.trigger(LIGHT_STROBE);});
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

void loop() {
  launchSwitch.update();
  safetySwitch.update();

  mainFsm.run_machine();
  lightFsm.run_machine();
}

