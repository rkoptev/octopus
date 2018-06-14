#ifndef UI_H
#define UI_H

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include "Adafruit_TFTLCD.h"
#include "Fonts/FreeMono12pt7b.h"
#include "Fonts/FreeSans18pt7b.h"

const uint16_t BACKGROUND = 0x0020;
const uint16_t BLACK      = 0x0000;
const uint16_t WHITE      = 0xFFFF;
const uint16_t GREEN      = 0x07E0;
const uint16_t AQUA       = 0x057C;

struct State {
  uint16_t waterLevel;
  bool pumpState;
  bool valveState;
  bool pumpFailure;
};

class UI {
public:
  UI(Adafruit_TFTLCD* tft) : tft(tft) {}

  void begin() {
    // Initialize LCD
    tft->reset();
    tft->begin(0x9481);
    tft->setRotation(3);
  }

  void setConsoleMode() {
    tft->fillScreen(BLACK);
    tft->setFont(&FreeMono12pt7b);
    tft->setTextColor(GREEN);
    tft->setCursor(0, 20);
  }

  template <typename T>
  void log(T data) {
    tft->println(data);
  }

  void setNormalMode() {
    bmpDraw("SCREEN.bmp", 0, 0);
    tft->setFont(&FreeSans18pt7b);
    tft->setTextColor(WHITE);
  }

  void update(State* state, bool force=false) {
    if (millis() - lastUpdateTime > UPDATE_INTERVAL || force) {
      // Pump slider
      if (!lastState || lastState->pumpState != state->pumpState) {
        bmpDraw(state->pumpState ? "EN_CH.bmp" : "EN_UNCH.bmp", SLIDER_X, PUMP_SLIDER_Y);
        bmpDraw(state->pumpState ? "DIS_UNCH.bmp" : "DIS_CH.bmp", SLIDER_X + SLIDER_DIS_OFFSET_X, PUMP_SLIDER_Y);
      }

      // Valve slider
      if (!lastState || lastState->valveState != state->valveState) {
        bmpDraw(state->valveState ? "EN_CH.bmp" : "EN_UNCH.bmp", SLIDER_X, VALVE_SLIDER_Y);
        bmpDraw(state->valveState ? "DIS_UNCH.bmp" : "DIS_CH.bmp", SLIDER_X + SLIDER_DIS_OFFSET_X, VALVE_SLIDER_Y);
      }

      // Pipes
      if (!lastState || lastState->pumpState != state->pumpState || lastState->valveState != state->valveState) {
        if (state->pumpState && state->valveState) {
          bmpDraw("PB_DRAIN.bmp", PIPE_BOTTOM_X, PIPE_BOTTOM_Y);
          if (lastState->pumpState == true) {
            bmpDraw("PT_DRAIN.bmp", PIPE_TOP_X, PIPE_TOP_Y);
          }
        } else if (state->pumpState) {
          bmpDraw("PB_PUMP.bmp", PIPE_BOTTOM_X, PIPE_BOTTOM_Y);
          bmpDraw("PT_PUMP.bmp", PIPE_TOP_X, PIPE_TOP_Y);
        } else {
          bmpDraw("PB_OFF.bmp", PIPE_BOTTOM_X, PIPE_BOTTOM_Y);
          bmpDraw("PT_OFF.bmp", PIPE_TOP_X, PIPE_TOP_Y);
        }
      }

      // Water level
      if (!lastState || lastState->waterLevel != state->waterLevel) {
        // Draw water level
        uint16_t waterHeight = TANK_H * state->waterLevel / 100;
        tft->fillRect(TANK_X, TANK_Y + (TANK_H - waterHeight), TANK_W, waterHeight, AQUA);
        tft->fillRect(TANK_X, TANK_Y, TANK_W, TANK_H - waterHeight, BLACK);

        // Print water level in tank
        char text[5];
        sprintf(text, "%d%%", state->waterLevel);
        int16_t  xt, yt;
        uint16_t wt, ht;
        tft->getTextBounds(text, 0, 0, &xt, &yt, &wt, &ht);
        tft->setCursor(TANK_X - xt + (TANK_W - wt) / 2, TANK_Y - yt + (TANK_H - ht) / 2);
        tft->print(text);
      }

      // Well (pump)
      if (!lastState || lastState->pumpFailure != state->pumpFailure) {
        if (state->pumpFailure) {
          tft->fillRect(WELL_X, WELL_Y, WELL_W, WELL_H, BACKGROUND);
          bmpDraw("TURBINEF.bmp", TURBINE_X, TURBINE_Y);
        } else {
          bmpDraw("WATER", WELL_X, WELL_Y);
        }
      }


      lastUpdateTime = millis();

      delete lastState;
      lastState = state;
      return;
    }
    delete state;
  }

private:
  static constexpr uint32_t UPDATE_INTERVAL = 1000;
  static constexpr uint8_t PIXEL_BUFFER_SIZE = 32;

  static constexpr uint16_t TANK_X = 315;
  static constexpr uint16_t TANK_Y = 12;
  static constexpr uint16_t TANK_W = 155;
  static constexpr uint16_t TANK_H = 93;
  static constexpr uint16_t SLIDER_X = 123;
  static constexpr uint16_t PUMP_SLIDER_Y = 39;
  static constexpr uint16_t VALVE_SLIDER_Y = 99;
  static constexpr uint16_t SLIDER_W = 36;
  static constexpr uint16_t SLIDER_H = 36;
  static constexpr uint16_t SLIDER_DIS_OFFSET_X = 67;
  static constexpr uint16_t PIPE_BOTTOM_X = 109;
  static constexpr uint16_t PIPE_BOTTOM_Y = 222;
  static constexpr uint16_t PIPE_TOP_X = 260;
  static constexpr uint16_t PIPE_TOP_Y = 55;
  static constexpr uint16_t WELL_X = 14;
  static constexpr uint16_t WELL_Y = 214;
  static constexpr uint16_t WELL_W = 90;
  static constexpr uint16_t WELL_H = 106;
  static constexpr uint16_t TURBINE_X = 33;
  static constexpr uint16_t TURBINE_Y = 244;

  Adafruit_TFTLCD* tft;
  State* lastState = nullptr;
  uint32_t lastUpdateTime = 0;

  void bmpDraw(const char *filename, int x, int y) {
    File     bmpFile;
    int      bmpWidth, bmpHeight;   // W+H in pixels
    uint8_t  bmpDepth;              // Bit depth (currently must be 24)
    uint32_t bmpImageoffset;        // Start of image data in file
    uint32_t rowSize;               // Not always = bmpWidth; may have padding
    uint8_t  sdbuffer[3*PIXEL_BUFFER_SIZE]; // pixel in buffer (R+G+B per pixel)
    uint16_t lcdbuffer[PIXEL_BUFFER_SIZE];  // pixel out buffer (16-bit per pixel)
    uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
    boolean  goodBmp = false;       // Set to true on valid header parse
    boolean  flip    = true;        // BMP is stored bottom-to-top
    int      w, h, row, col;
    uint8_t  r, g, b;
    uint32_t pos = 0, startTime = millis();
    uint8_t  lcdidx = 0;
    boolean  first = true;

    // Check display coordinates
    if((x >= tft->width()) || (y >= tft->height())) return;

    Serial.println();
    Serial.print(F("Loading image '"));
    Serial.print(filename);
    Serial.println('\'');
    // Open requested file on SD card
    if (!(bmpFile = SD.open(filename))) {
      Serial.println(F("File not found"));
      return;
    }

    // Parse BMP header
    if(read16(bmpFile) == 0x4D42) { // BMP signature
      Serial.println(F("File size: ")); Serial.println(read32(bmpFile));
      (void)read32(bmpFile); // Read & ignore creator bytes
      bmpImageoffset = read32(bmpFile); // Start of image data
      Serial.print(F("Image Offset: ")); Serial.println(bmpImageoffset, DEC);
      // Read DIB header
      Serial.print(F("Header size: ")); Serial.println(read32(bmpFile));
      bmpWidth  = read32(bmpFile);
      bmpHeight = read32(bmpFile);
      if(read16(bmpFile) == 1) { // # planes -- must be '1'
        bmpDepth = read16(bmpFile); // bits per pixel
        Serial.print(F("Bit Depth: ")); Serial.println(bmpDepth);
        if((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed

          goodBmp = true; // Supported BMP format -- proceed!
          Serial.print(F("Image size: "));
          Serial.print(bmpWidth);
          Serial.print('x');
          Serial.println(bmpHeight);

          // BMP rows are padded (if needed) to 4-byte boundary
          rowSize = (bmpWidth * 3 + 3) & ~3;

          // If bmpHeight is negative, image is in top-down order.
          // This is not canon but has been observed in the wild.
          if(bmpHeight < 0) {
            bmpHeight = -bmpHeight;
            flip      = false;
          }

          // Crop area to be loaded
          w = bmpWidth;
          h = bmpHeight;
          if((x+w-1) >= tft->width())  w = tft->width()  - x;
          if((y+h-1) >= tft->height()) h = tft->height() - y;

          // Set TFT address window to clipped image bounds
          tft->setAddrWindow(x, y, x+w-1, y+h-1);

          for (row=0; row<h; row++) { // For each scanline...
            // Seek to start of scan line.  It might seem labor-
            // intensive to be doing this on every line, but this
            // method covers a lot of gritty details like cropping
            // and scanline padding.  Also, the seek only takes
            // place if the file position actually needs to change
            // (avoids a lot of cluster math in SD library).
            if(flip) // Bitmap is stored bottom-to-top order (normal BMP)
              pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
            else     // Bitmap is stored top-to-bottom
              pos = bmpImageoffset + row * rowSize;
            if(bmpFile.position() != pos) { // Need seek?
              bmpFile.seek(pos);
              buffidx = sizeof(sdbuffer); // Force buffer reload
            }

            for (col=0; col<w; col++) { // For each column...
              // Time to read more pixel data?
              if (buffidx >= sizeof(sdbuffer)) { // Indeed
                // Push LCD buffer to the display first
                if(lcdidx > 0) {
                  tft->pushColors(lcdbuffer, lcdidx, first);
                  lcdidx = 0;
                  first  = false;
                }
                bmpFile.read(sdbuffer, sizeof(sdbuffer));
                buffidx = 0; // Set index to beginning
              }

              // Convert pixel from BMP to TFT format
              b = sdbuffer[buffidx++];
              g = sdbuffer[buffidx++];
              r = sdbuffer[buffidx++];
              lcdbuffer[lcdidx++] = tft->color565(r,g,b);
            } // end pixel
          } // end scanline
          // Write any remaining data to LCD
          if(lcdidx > 0) {
            tft->pushColors(lcdbuffer, lcdidx, first);
          }
          Serial.print(F("Loaded in "));
          Serial.print(millis() - startTime);
          Serial.println(" ms");
        } // end goodBmp
      }
    }

    bmpFile.close();
    if(!goodBmp) Serial.println(F("BMP format not recognized."));
  }

uint16_t read16(File f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}
};

#endif // UI_H
