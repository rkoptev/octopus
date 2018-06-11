#ifndef UI_H
#define UI_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_TFTLCD.h>
#include "Fonts/FreeSerif12pt7b.h"

class UI {
public:
  UI(Adafruit_TFTLCD* tft) {
    this->tft = tft;
  }

  void begin() {
    // Initialize LCD
    tft->reset();
    tft->begin(0x9481);
    tft->setRotation(3);
  }

  void setConsoleMode() {
    tft->fillScreen(0x0000);
    tft->setFont(&FreeSerif12pt7b);
    tft->setTextColor(0x07E0);
    tft->setCursor(0, 20);
  }

  template <typename T>
  void log(T data) {
    tft->println(data);
  }
private:
  Adafruit_TFTLCD* tft;
};

#endif // UI_H
