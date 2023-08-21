#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

void displayInit();
void clearDisplay();
void showText(const arduino::__FlashStringHelper *text);
void showCountdownText(const arduino::__FlashStringHelper *text, const arduino::__FlashStringHelper *title = nullptr);
void showText3(const arduino::__FlashStringHelper *text1, const arduino::__FlashStringHelper *text2, const arduino::__FlashStringHelper *text3, int invertLine = -1);
void showLogo();
void centerText(const arduino::__FlashStringHelper *text);

#endif // DISPLAY_H