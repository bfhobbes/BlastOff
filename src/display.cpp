#include "display.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C //< 0x3D is another common address

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

void showText3(const arduino::__FlashStringHelper *text1, const arduino::__FlashStringHelper *text2, const arduino::__FlashStringHelper *text3, int invertLine)
{
  if(invertLine < 0 ) {
    invertLine = text3 == nullptr ? 1 : 2;
  }

  display.clearDisplay();
  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);

  if(invertLine>=0) {
    display.fillRoundRect(2, (22*invertLine) - 2, 124, 22, 3, SSD1306_WHITE);
  }

  if (text1)
  {
    if (invertLine == 0)
    {
      display.setTextColor(SSD1306_BLACK); // Draw 'inverse' text
    }

    display.setCursor(2, 0);
    display.println(text1);

    if( invertLine == 0) {
      display.setTextColor(SSD1306_WHITE);
    }
  }
  if (text2)
  {
    if (invertLine == 1)
    {
      display.setTextColor(SSD1306_BLACK); // Draw 'inverse' text
    }
    display.setCursor(10, 22);
    display.println(text2);
    if( invertLine == 1) {
      display.setTextColor(SSD1306_WHITE);
    }
  }
  if (text3)
  {
    if (invertLine == 2)
    {
      display.setTextColor(SSD1306_BLACK); // Draw 'inverse' text
    }
    display.setCursor(10, 44);
    display.println(text3);
  }
  display.display(); // Show initial text
}
