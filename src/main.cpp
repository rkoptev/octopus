#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <NewPing.h>
#include <Adafruit_GFX.h>
#include <TouchScreen.h>
#include "Adafruit_TFTLCD.h"
#include "WaterLevelSensor.h"
#include "WaterFlowSensor.h"
#include "UI.h"

#define VERSION "0.2.0"
#define SERIAL_BAUDRATE 115200

// Water flow sensor
#define WATER_FLOW_SENSOR_PIN 21 // Only pins with interrupts are supported (!)
#define WATER_COUNTER_EEPROM_ADDRESS 0x0 // It uses 4 bytes to store data

// Water level sensor
#define ULTRASONIC_TRIG_PIN 4
#define ULTRASONIC_ECHO_PIN 5
#define ULTRASONIC_MIN_DISTANCE 20
#define ULTRASONIC_MAX_DISTANCE 200

// LCD pins
#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET A4
#define SD_CS 53

// Touch screen pins
#define YP A3  // must be an analog pin, use "An" notation!
#define XN A2  // must be an analog pin, use "An" notation!
#define YN 23   // can be a digital pin
#define XP 22   // can be a digital pin

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
TouchScreen ts = TouchScreen(XP, YP, XN, YN, 300);

WaterFlowSensor waterFlowSensor(WATER_FLOW_SENSOR_PIN, WATER_COUNTER_EEPROM_ADDRESS);
WaterLevelSensor waterLevelSensor(ULTRASONIC_TRIG_PIN, ULTRASONIC_ECHO_PIN, ULTRASONIC_MIN_DISTANCE, ULTRASONIC_MAX_DISTANCE);

// Runtime variables
bool pumpState = true;
bool valveState = false;

void onPumpButtonPressed() {
  pumpState = !pumpState;
}

void onValveButtonPressed() {
  valveState = !valveState;
  if (valveState) {
    pumpState = true;
  }
}

UI ui(&tft, &ts , onPumpButtonPressed, onValveButtonPressed);

void setup() {
  // Configure GPIO
  pinMode(SD_CS, OUTPUT);

  // Initialize serial connection
  Serial.begin(SERIAL_BAUDRATE);

  // Initialize UI
  ui.begin();
  ui.setConsoleMode();

  ui.info(String(F("OCTOPUS v")) + VERSION);
  ui.info(String(F("Total water used: ")) + waterFlowSensor.getWaterAmount() + " L");

  // Check water level sensor
  if (waterLevelSensor.update()) {
    ui.info(F("Ultrasonic check succeed"));
  } else {
    ui.error(F("Ultrasonic check failed!"));
  }

  // Initialize SD card
  ui.info(F("Initializing SD card... "));
  if (!SD.begin(SD_CS)) {
    ui.error(F("SD card initialization failed!"));
    return;
  }
  ui.info(F("SD card - OK!"));

  // Draw GUI
  ui.setNormalMode();
}

void loop() {
  // Update sensors
  waterLevelSensor.update();

  State* state = new State{};
  state->waterLevel = waterLevelSensor.getWaterLevel();
  state->pumpState = pumpState;
  state->valveState = valveState;
  state->pumpFailure = false;

  ui.update(state);
}
