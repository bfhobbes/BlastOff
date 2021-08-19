#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

void displayInit();
void clearDisplay();
void showText(const arduino::__FlashStringHelper *text);
void showCountdownText(const arduino::__FlashStringHelper *text);
void showText3(const arduino::__FlashStringHelper *text1, const arduino::__FlashStringHelper *text2, const arduino::__FlashStringHelper *text3);

#endif // DISPLAY_H