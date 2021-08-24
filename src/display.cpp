#include "display.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C //< 0x3D is another common address

static const unsigned char PROGMEM bitmap_cublogo[]  = {
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // ································································
  0x00,0x08,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // ······▌····················▐····································
  0x00,0x07,0x00,0x00,0x00,0x00,0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // ······▐█··················█▌····································
  0x00,0x51,0xC0,0x00,0x00,0x00,0x39,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // ····▐▐·▐█················█▌▐▌···································
  0x00,0x58,0xE0,0x00,0x00,0x00,0xF1,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // ····▐▐▌·█▌··············██·▐▌···································
  0x00,0x78,0x70,0x00,0x00,0x01,0xC3,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // ····▐█▌·▐█·············▐█··██···································
  0x00,0xFC,0x38,0x00,0x00,0x07,0x87,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // ····███··█▌···········▐█▌·▐█▐···································
  0x00,0xFE,0x1E,0x1F,0x38,0x0E,0x0F,0x41,0xFF,0xC0,0x1F,0x00,0x1F,0xC1,0xE0,0x70, // ····███▌·▐█▌·▐██·█▌···█▌··██▐··▐█████····▐██·····▐███··▐█▌··▐█··
  0x00,0xFE,0x0F,0xFF,0xFF,0xFC,0x3F,0x41,0xFF,0xE0,0x1F,0x00,0x7F,0xE1,0xE0,0xF0, // ····███▌··█████████████··███▐··▐█████▌···▐██····▐████▌·▐█▌··██··
  0x00,0x7F,0x87,0xE0,0x83,0xF8,0x3E,0x81,0xFF,0xF0,0x1F,0x80,0x78,0xF1,0xE0,0xE0, // ····▐███▌·▐██▌··▌··███▌··██▌▌··▐██████···▐██▌···▐█▌·██·▐█▌··█▌··
  0x00,0xDF,0xC3,0xE4,0xF3,0xF0,0x7E,0x81,0xE0,0x78,0x3F,0x80,0xF0,0x71,0xE1,0xC0, // ····█▐███··██▌▐·██·███··▐██▌▌··▐█▌··▐█▌··███▌···██··▐█·▐█▌·▐█···
  0x00,0x5F,0xC1,0xE6,0xF1,0xF9,0xFE,0x81,0xE0,0x78,0x3B,0x80,0xE0,0x79,0xE3,0xC0, // ····▐▐███··▐█▌▐▌██·▐██▌▐███▌▌··▐█▌··▐█▌··█▌█▌···█▌··▐█▌▐█▌·██···
  0x00,0x5F,0xC1,0xCE,0xB9,0xFC,0xFD,0x01,0xE0,0x78,0x33,0xC1,0xE0,0x39,0xE7,0x80, // ····▐▐███··▐█·█▌▌█▌▐███·███▐···▐█▌··▐█▌··█·██··▐█▌···█▌▐█▌▐█▌···
  0x00,0x6F,0xE3,0x9F,0x1F,0x7F,0xFD,0x81,0xE0,0x78,0x73,0xC1,0xE0,0x01,0xE7,0x80, // ····▐▌███▌·█▌▐██·▐██▐██████▐▌··▐█▌··▐█▌·▐█·██··▐█▌·····▐█▌▐█▌···
  0x00,0x6F,0xFF,0x1E,0x0F,0x1F,0xFB,0x81,0xE0,0xF0,0x71,0xC1,0xE0,0x01,0xEF,0x80, // ····▐▌██████·▐█▌··██·▐████▌█▌··▐█▌··██··▐█·▐█··▐█▌·····▐█▌██▌···
  0x00,0x27,0xFC,0xFE,0x07,0x87,0xFB,0x81,0xFF,0xF0,0x61,0xE1,0xC0,0x01,0xFF,0xC0, // ·····▌▐████·███▌··▐█▌·▐███▌█▌··▐██████··▐▌·▐█▌·▐█······▐█████···
  0x00,0x77,0xFF,0xFC,0x9F,0xE0,0xFF,0x81,0xFF,0xE0,0xE1,0xE1,0xC0,0x01,0xFD,0xC0, // ····▐█▐████████·▌▐███▌··████▌··▐█████▌··█▌·▐█▌·▐█······▐███▐█···
  0x00,0xFF,0xCF,0xF8,0xDF,0xF8,0x3F,0x81,0xFF,0x00,0xE0,0xE1,0xE0,0x01,0xF9,0xE0, // ····█████·████▌·█▐████▌··███▌··▐████····█▌··█▌·▐█▌·····▐██▌▐█▌··
  0x01,0xFE,0x1F,0xF9,0xDF,0xFE,0x1F,0x01,0xE0,0x00,0xFF,0xF1,0xE0,0x01,0xF1,0xE0, // ···▐███▌·▐████▌▐█▐█████▌·▐██···▐█▌······██████·▐█▌·····▐██·▐█▌··
  0x01,0xFC,0x7F,0xF9,0xDF,0xFF,0xC3,0x01,0xE0,0x01,0xFF,0xF1,0xE0,0x39,0xE0,0xE0, // ···▐███·▐█████▌▐█▐███████··█···▐█▌·····▐██████·▐█▌···█▌▐█▌··█▌··
  0x01,0xF8,0xFF,0xFB,0xCF,0xFF,0xC7,0xC1,0xE0,0x01,0xFF,0xF0,0xE0,0x71,0xE0,0xF0, // ···▐██▌·██████▌██·███████·▐██··▐█▌·····▐██████··█▌··▐█·▐█▌··██··
  0x00,0xE1,0xFF,0xFB,0xEF,0xFF,0x1F,0xC1,0xE0,0x01,0xC0,0x78,0xF0,0xF1,0xE0,0x70, // ····█▌·▐██████▌██▌██████·▐███··▐█▌·····▐█···▐█▌·██··██·▐█▌··▐█··
  0x02,0xC7,0xFF,0xF3,0xEF,0xFF,0xCC,0xE1,0xE0,0x03,0x80,0x78,0x7F,0xE1,0xE0,0x78, // ···▌█·▐███████·██▌███████·█·█▌·▐█▌·····█▌···▐█▌·▐████▌·▐█▌··▐█▌·
  0x06,0x0F,0xFF,0xF3,0xE7,0xFF,0xFC,0x31,0xE0,0x03,0x80,0x38,0x3F,0xE1,0xE0,0x78, // ··▐▌··████████·██▌▐████████··█·▐█▌·····█▌····█▌··████▌·▐█▌··▐█▌·
  0x06,0x0B,0xFF,0xF7,0xF7,0xFF,0xFC,0x39,0xE0,0x03,0x80,0x38,0x1F,0x80,0xE0,0x38, // ··▐▌··▌███████▐███▐████████··█▌▐█▌·····█▌····█▌··▐██▌···█▌···█▌·
  0x06,0x3F,0xFF,0xF7,0xF7,0xFF,0xFF,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // ··▐▌·█████████▐███▐█████████·▐▌·································
  0x1C,0x3F,0xFF,0xFF,0xF7,0xFF,0xFF,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // ·▐█··█████████████▐█████████·▐▌·································
  0x3C,0x7B,0xFF,0xFF,0xF1,0xFF,0xFF,0x1C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // ·██·▐█▌███████████·▐████████·▐█·································
  0x10,0x7B,0xFE,0x7F,0xF8,0x1F,0xFF,0xCC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // ·▐··▐█▌████▌▐█████▌··▐███████·█·································
  0x30,0x3F,0xF8,0x0F,0xFC,0x07,0xFF,0xC4,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // ·█···█████▌···█████···▐██████·▐·································
  0x70,0x1F,0xF0,0x0F,0xFC,0x02,0xEF,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // ▐█···▐████····█████····▌█▌█████▌································
  0x73,0xBB,0xE0,0x07,0xF8,0x00,0x00,0xEE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // ▐█·█▌█▌██▌····▐███▌·········█▌█▌································
  0x73,0xBB,0xC0,0x07,0xF8,0x7F,0xD8,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // ▐█·█▌█▌██·····▐███▌·▐████▐▌···█·································
  0x76,0x3F,0xFF,0x87,0xF9,0xFF,0xF8,0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // ▐█▐▌·███████▌·▐███▌▐██████▌···█▌································
  0xF6,0x7F,0xC7,0xE7,0x9B,0xF1,0x80,0x26,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // ██▐▌▐████·▐██▌▐█▌▐▌███·▐▌····▌▐▌································
  0x60,0x13,0xC7,0xF0,0x83,0xC3,0x80,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // ▐▌···▐·██·▐███··▌··██··█▌······▌································
  0x34,0x01,0xC1,0xF0,0x93,0x03,0x80,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // ·█▐····▐█··▐██··▌▐·█···█▌······▌································
  0x3E,0x01,0xE0,0x38,0xDA,0x03,0x80,0x13,0x00,0x0F,0x00,0x1F,0x00,0xFF,0xF0,0x00, // ·██▌···▐█▌···█▌·█▐▌▌···█▌····▐·█······██·····▐██····██████······
  0x3E,0x01,0xF8,0x10,0xFE,0x07,0x00,0x07,0x00,0x3F,0xC0,0x3F,0xC1,0xFF,0xF0,0x00, // ·██▌···▐██▌··▐··███▌··▐█······▐█·····████····████··▐██████······
  0x1E,0x01,0xBE,0x10,0xFE,0x1E,0x00,0x17,0x00,0x7F,0xE0,0x7F,0xE1,0xFF,0xF0,0x00, // ·▐█▌···▐▌██▌·▐··███▌·▐█▌·····▐▐█····▐████▌··▐████▌·▐██████······
  0x1E,0x00,0x7F,0xD0,0xFE,0xFC,0x00,0x1C,0x00,0xF1,0xF0,0xF1,0xF0,0x00,0x70,0x00, // ·▐█▌····▐████▐··███▌███······▐█·····██·▐██··██·▐██······▐█······
  0x0E,0x00,0x0F,0xE1,0xFE,0xF8,0x00,0x1C,0x00,0xE0,0xF1,0xE0,0xF0,0x00,0xE0,0x00, // ··█▌······███▌·▐███▌██▌······▐█·····█▌··██·▐█▌··██······█▌······
  0x07,0x80,0x03,0xE3,0xFF,0xE0,0x00,0x78,0x01,0xE0,0x71,0xC0,0x70,0x01,0xC0,0x00, // ··▐█▌······██▌·██████▌······▐█▌····▐█▌··▐█·▐█···▐█·····▐█·······
  0x07,0x80,0x00,0xF3,0xFF,0xC0,0x07,0xF8,0x00,0x40,0x70,0x40,0x70,0x01,0xC0,0x00, // ··▐█▌·······██·██████·····▐███▌·····▐···▐█··▐···▐█·····▐█·······
  0x01,0xF0,0x00,0xFB,0xFF,0x80,0x1F,0xC0,0x00,0x00,0x70,0x00,0xF0,0x03,0x80,0x00, // ···▐██······██▌█████▌····▐███···········▐█······██·····█▌·······
  0x00,0xF0,0x00,0xF3,0xFF,0x80,0x3F,0x00,0x00,0x00,0xF0,0x00,0xF0,0x07,0x00,0x00, // ····██······██·█████▌····███············██······██····▐█········
  0x00,0xFF,0x80,0x73,0xFF,0x82,0xFE,0x00,0x00,0x01,0xE0,0x01,0xE0,0x07,0x00,0x00, // ····████▌···▐█·█████▌··▌███▌···········▐█▌·····▐█▌····▐█········
  0x00,0x7F,0xFC,0x73,0xFF,0xBF,0xFE,0x00,0x00,0x03,0xE0,0x03,0xC0,0x0F,0x00,0x00, // ····▐██████·▐█·█████▌██████▌···········██▌·····██·····██········
  0x00,0x0F,0xFC,0x73,0xFF,0xFF,0xF8,0x00,0x00,0x07,0x80,0x07,0x80,0x0E,0x00,0x00, // ······█████·▐█·███████████▌···········▐█▌·····▐█▌·····█▌········
  0x00,0x03,0xFF,0x33,0xFF,0xFF,0xE0,0x00,0x00,0x1F,0x00,0x1F,0x00,0x0E,0x00,0x00, // ·······█████·█·██████████▌···········▐██·····▐██······█▌········
  0x00,0x03,0xFF,0x31,0xFF,0xFF,0x80,0x00,0x00,0x3E,0x00,0x3C,0x00,0x1E,0x00,0x00, // ·······█████·█·▐████████▌············██▌·····██······▐█▌········
  0x00,0x00,0xFF,0x01,0xFF,0xFE,0x00,0x00,0x00,0xF8,0x00,0xF8,0x00,0x1E,0x00,0x00, // ········████···▐███████▌············██▌·····██▌······▐█▌········
  0x00,0x00,0x3F,0x03,0xFE,0x7E,0x00,0x00,0x01,0xF0,0x01,0xF0,0x00,0x1E,0x00,0x00, // ·········███···████▌▐██▌···········▐██·····▐██·······▐█▌········
  0x00,0x00,0x3F,0x00,0x1E,0x70,0x00,0x00,0x01,0xFF,0xF1,0xFF,0xF0,0x1E,0x00,0x00, // ·········███·····▐█▌▐█·············▐██████·▐██████···▐█▌········
  0x00,0x00,0x07,0x0C,0x1E,0x20,0x00,0x00,0x01,0xFF,0xF1,0xFF,0xF0,0x1C,0x00,0x00, // ··········▐█··█··▐█▌·▌·············▐██████·▐██████···▐█·········
  0x00,0x00,0x01,0x0E,0x1C,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // ···········▐··█▌·▐█·▐···········································
  0x00,0x00,0x01,0x0F,0xFC,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // ···········▐··█████·▐···········································
  0x00,0x00,0x01,0x8F,0xFC,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // ···········▐▌·█████·▐···········································
  0x00,0x00,0x00,0x87,0xF8,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // ············▌·▐███▌·▌···········································
  0x00,0x00,0x00,0xC3,0xF0,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // ············█··███··▌···········································
  0x00,0x00,0x00,0x40,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // ············▐······▐············································
  0x00,0x00,0x00,0x39,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // ·············█▌▐███▌············································
  0x00,0x00,0x00,0x0F,0xFC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // ··············█████·············································
  0x00,0x00,0x00,0x07,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00  // ··············▐███··············································
};



Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void displayInit() {
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }

  showLogo();
  sleep_ms(3000);
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


void showLogo() {
  display.clearDisplay();
  display.drawBitmap(0,0, bitmap_cublogo, 128, 64, SSD1306_WHITE );
  display.display();
}