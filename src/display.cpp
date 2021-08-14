#include "display.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void displayInit() {
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }
}

void clearDisplay() {
    display.clearDisplay();
    display.display();
}

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
  display.setTextSize(3); // Draw 3X-scale text
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
