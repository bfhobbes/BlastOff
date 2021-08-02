#include <Arduino.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <Bounce2.h>

#include "fsm.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define FLASH_PIN LED_BUILTIN
#define ARMED_LIGHT  7
#define ARMING_PIN 9
#define LAUNCH_PIN 8
#define RELAY_PIN 10

enum Events {
  ARMING_DOWN = 0,
  ARMING_UP,
  LAUNCH_DOWN,
  LAUNCH_UP,
  ENTER_SETTING,
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
  AUDIO
};

static const SettingInfo settings[] = {
  {
    "Option1", { "o1_val1", "o1_val2", nullptr }
  },
  {
    "Option2", { "o2_val1", "o2_val2", "o2_val3", nullptr }
  },
  {
    "Option3", { "o3_val1", "o3_val2", "o3_val3", "o3_val4", nullptr }
  },
  {
    "Exit", { nullptr }
  }
};

int currentSetting = 0;
int currentValue = 0;

const int NUM_SETTINGS = sizeof(settings)/sizeof(SettingInfo);
int settingVals[NUM_SETTINGS];

void testdrawline();
void testdrawchar();
void testdrawstyles();
void testscrolltext();

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

Bounce safetySwitch;
Bounce launchSwitch;

void on_init_enter(void) {
  showText(F("init"));
}

State state_init(on_init_enter, nullptr, nullptr);
Fsm mainFsm(&state_init);


enum LightEvents {
  LIGHT_OFF, 
  LIGHT_ON,
  LIGHT_STROBE
};

State light_off([]() {digitalWrite(ARMED_LIGHT, LOW);}, nullptr, nullptr );
State light_on([]() {digitalWrite(ARMED_LIGHT, HIGH);}, nullptr, nullptr );
Fsm lightFsm(&light_off);

int strobeCount = 0;
State light_strobe_on( 
  []() {digitalWrite(ARMED_LIGHT, HIGH); --strobeCount; }, 
  nullptr,
  nullptr);
State light_strobe_off( 
  []() {digitalWrite(ARMED_LIGHT, LOW);}, 
  []() { if(strobeCount<=0) {lightFsm.trigger(LIGHT_OFF);} }, 
  nullptr);


void on_light_on_enter(void) {
  digitalWrite(FLASH_PIN, HIGH);
}

void on_light_off_enter(void) {
  digitalWrite(FLASH_PIN, LOW);
}

void on_Launching_enter(void) {
  digitalWrite(FLASH_PIN, HIGH);
  digitalWrite(RELAY_PIN, HIGH);
  showCountdownText(F("LAUNCH"));
}

void on_launching_exit(void) {
  digitalWrite(FLASH_PIN, LOW);
  digitalWrite(RELAY_PIN, LOW);
}

void on_idle_enter(void) {
  showText(F("idle"));
}

void on_armed_enter(void) {
  lightFsm.trigger(LIGHT_ON);
  
  showText(F("armed"));
}

void on_armed_exit(void) {
  lightFsm.trigger(LIGHT_OFF);
}

void on_postlaunch_enter(void) {
  showText(F("postlaunch"));
}

void on_prearm_enter(void) {
  showText(F("prearmed"));
}

void on_prearm(void) {
  if(!launchSwitch.read()) {
    mainFsm.trigger(ENTER_SETTING);
  } else {
    mainFsm.trigger(ENTER_ARMED);
  }
}

void on_setting_enter(void) {
  showText(F("setting"));
}

void on_abort_enter(void) {
  showText(F("ABORT"));
}

void on_settingname_enter(void) {
  if(currentSetting >= NUM_SETTINGS) {
    currentSetting = 0;
  }
  showText3(
    F("Setting"),
    F(settings[currentSetting].Name),
    nullptr
  );
}

void on_settingvalue_enter(void) {
  if(settings[currentSetting].values[currentValue] == nullptr) {
    currentValue = 0;
  }
  showText3(
    F("Setting"),
    F(settings[currentSetting].Name),
    F(settings[currentSetting].values[currentValue])
  );
}

int countdownStart = 0;
void on_countdown_enter(void) {
  countdownStart = millis();
}

void on_countdown_update(void) {
  int countdown = 10000 + countdownStart - millis();
  
  char buff[20];
  sprintf(buff, "%.2f", (float)countdown / 1000.0f);

  showCountdownText(F(buff));
  if(countdown <= 0) {
    mainFsm.trigger(LAUNCH);
  }
}


State state_idle(on_idle_enter, nullptr, nullptr);

State state_prearmed(on_prearm_enter, on_prearm, nullptr);

State state_setting(on_setting_enter, nullptr, nullptr);
State state_settingname(on_settingname_enter, nullptr, nullptr);
State state_settingvalue(on_settingvalue_enter, nullptr, nullptr);

State state_armed(on_armed_enter, nullptr, on_armed_exit);

State state_launching(on_Launching_enter, nullptr, on_launching_exit);

State state_countdown(on_countdown_enter, on_countdown_update, nullptr );
State state_abort(on_abort_enter, nullptr, nullptr);

State state_postlaunch(on_postlaunch_enter, nullptr, nullptr);


void setup() {
  Serial.begin(9600);

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

  lightFsm.add_transition(&light_off, &light_strobe_on, LIGHT_STROBE, []() {strobeCount = 3;});
  lightFsm.add_transition(&light_strobe_off, &light_off, LIGHT_OFF, nullptr);
  lightFsm.add_timed_transition(&light_strobe_off, &light_strobe_on, 300, nullptr);
  lightFsm.add_timed_transition(&light_strobe_on, &light_strobe_off, 300, nullptr ) ;

  mainFsm.add_timed_transition(&state_init, &state_idle, 50, nullptr);

  mainFsm.add_transition(&state_idle, &state_prearmed, ARMING_DOWN, nullptr);

  mainFsm.add_transition(&state_prearmed, &state_setting, ENTER_SETTING, nullptr);
  mainFsm.add_transition(&state_prearmed, &state_armed, ENTER_ARMED, nullptr);

  mainFsm.add_transition(&state_setting, &state_idle, ARMING_UP, nullptr);
  mainFsm.add_transition(&state_setting, &state_settingname, LAUNCH_UP, nullptr);

  mainFsm.add_transition(&state_settingname, &state_settingvalue, ARMING_UP, nullptr);
  mainFsm.add_transition(&state_settingname, &state_settingname, LAUNCH_DOWN, []() { currentSetting++; } ); 

  mainFsm.add_transition(&state_settingvalue, &state_settingname, ARMING_DOWN, nullptr);
  mainFsm.add_transition(&state_settingvalue, &state_settingvalue, LAUNCH_DOWN, []() { currentValue++; } ); 

  mainFsm.add_transition(&state_countdown, &state_abort, ARMING_UP, []() { lightFsm.trigger(LIGHT_STROBE);});
  mainFsm.add_transition(&state_countdown, &state_launching, LAUNCH, nullptr);


  mainFsm.add_timed_transition(&state_abort, &state_idle, 3000, nullptr);

  mainFsm.add_transition(&state_armed, &state_countdown, LAUNCH_DOWN, nullptr);
  mainFsm.add_transition(&state_armed, &state_idle, ARMING_UP, nullptr);

  mainFsm.add_timed_transition(&state_launching, &state_postlaunch, 300, nullptr);
  mainFsm.add_transition(&state_postlaunch, &state_idle, ARMING_UP, nullptr);

  
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  
  display.display();
  delay(1000);
  display.clearDisplay();
  display.display();

  // testdrawline();
  // testdrawchar();
  // testdrawstyles();
  // testscrolltext();
}

void loop() {
  launchSwitch.update();
  safetySwitch.update();

  if(launchSwitch.fell()) {
    mainFsm.trigger(LAUNCH_DOWN);
  }
  
  if(launchSwitch.rose()) {
    mainFsm.trigger(LAUNCH_UP);
  }

  if(safetySwitch.fell()) {
    mainFsm.trigger(ARMING_DOWN);
  }

  if(safetySwitch.rose()) {
    mainFsm.trigger(ARMING_UP);
  }
  
  // put your main code here, to run repeatedly:

  mainFsm.run_machine();
  lightFsm.run_machine();
}


void testdrawline() {
  int16_t i;

  display.clearDisplay(); // Clear display buffer

  for(i=0; i<display.width(); i+=4) {
    display.drawLine(0, 0, i, display.height()-1, SSD1306_WHITE);
    display.display(); // Update screen with each newly-drawn line
    delay(1);
  }
  for(i=0; i<display.height(); i+=4) {
    display.drawLine(0, 0, display.width()-1, i, SSD1306_WHITE);
    display.display();
    delay(1);
  }
  delay(250);

  display.clearDisplay();

  for(i=0; i<display.width(); i+=4) {
    display.drawLine(0, display.height()-1, i, 0, SSD1306_WHITE);
    display.display();
    delay(1);
  }
  for(i=display.height()-1; i>=0; i-=4) {
    display.drawLine(0, display.height()-1, display.width()-1, i, SSD1306_WHITE);
    display.display();
    delay(1);
  }
  delay(250);

  display.clearDisplay();

  for(i=display.width()-1; i>=0; i-=4) {
    display.drawLine(display.width()-1, display.height()-1, i, 0, SSD1306_WHITE);
    display.display();
    delay(1);
  }
  for(i=display.height()-1; i>=0; i-=4) {
    display.drawLine(display.width()-1, display.height()-1, 0, i, SSD1306_WHITE);
    display.display();
    delay(1);
  }
  delay(250);

  display.clearDisplay();

  for(i=0; i<display.height(); i+=4) {
    display.drawLine(display.width()-1, 0, 0, i, SSD1306_WHITE);
    display.display();
    delay(1);
  }
  for(i=0; i<display.width(); i+=4) {
    display.drawLine(display.width()-1, 0, i, display.height()-1, SSD1306_WHITE);
    display.display();
    delay(1);
  }

  delay(2000); // Pause for 2 seconds
}

void testdrawchar(void) {
  display.clearDisplay();

  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  // Not all the characters will fit on the display. This is normal.
  // Library will draw what it can and the rest will be clipped.
  for(int16_t i=0; i<256; i++) {
    if(i == '\n') display.write(' ');
    else          display.write(i);
  }

  display.display();
  delay(2000);
}

void testdrawstyles(void) {
  display.clearDisplay();

  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F("Hello, world!"));

  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
  display.println(3.141592);

  display.setTextSize(2);             // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.print(F("0x")); display.println(0xDEADBEEF, HEX);

  display.display();
  delay(2000);
}

void testscrolltext(void) {
  display.clearDisplay();

  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 0);
  display.println(F("scroll"));
  display.display();      // Show initial text
  delay(100);

  // Scroll in various directions, pausing in-between:
  display.startscrollright(0x00, 0x0F);
  delay(2000);
  display.stopscroll();
  delay(1000);
  display.startscrollleft(0x00, 0x0F);
  delay(2000);
  display.stopscroll();
  delay(1000);
  display.startscrolldiagright(0x00, 0x07);
  delay(2000);
  display.startscrolldiagleft(0x00, 0x07);
  delay(2000);
  display.stopscroll();
  delay(1000);
}
