#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <NewPing.h>
#include <Adafruit_GFX.h>
#include <Adafruit_TFTLCD.h>
#include "WaterLevelSensor.h"
#include "WaterFlowSensor.h"
#include "UI.h"

#define VERSION "0.0.1"
#define SERIAL_BAUDRATE 115200

// Water flow sensor
#define WATER_FLOW_SENSOR_PIN 21 // Only pins with interrupts are supported (!)
#define WATER_COUNTER_EEPROM_ADDRESS 0x0 // It uses 4 bytes to store data

// Ultrasonic sensor
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
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 23   // can be a digital pin
#define XP 22   // can be a digital pin

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
NewPing ultrasonic(ULTRASONIC_TRIG_PIN, ULTRASONIC_ECHO_PIN);

WaterFlowSensor waterFlowSensor(WATER_FLOW_SENSOR_PIN, WATER_COUNTER_EEPROM_ADDRESS);
WaterLevelSensor waterLevelSensor(&ultrasonic, ULTRASONIC_MIN_DISTANCE, ULTRASONIC_MAX_DISTANCE);
UI ui(&tft);

// Runtime variables
bool pumpState = true;
bool valveState = false;

void setup() {
  // Initialize serial connection
  Serial.begin(SERIAL_BAUDRATE);

  // Initialize SD card
  Serial.print(F("Initializing SD card..."));
  pinMode(SD_CS, OUTPUT);
  if (!SD.begin(SD_CS)) {
    Serial.println(F("failed!"));
    return;
  }
  Serial.println(F("OK!"));

  // Initialize UI
  ui.begin();
  ui.setConsoleMode();
  ui.log(String("OCTOPUS v") + VERSION);

  ui.log(String("Total water used: ") + waterFlowSensor.getWaterAmount() + " L");

  // Check water level sensor
  if (waterLevelSensor.update()) {
    ui.log("Ultrasonic check succeed");
  } else {
    ui.log("Ultrasonic check FAILED!");
  }

  ui.setNormalMode();
}

void loop() {
  // Update sensors
  waterLevelSensor.update();

  State* state = new State{};
  state->waterLevel = waterLevelSensor.getWaterLevel();
  state->pumpState = pumpState;
  state->valveState = valveState;
  state->pumpFailure = true;

  ui.update(state);
}
