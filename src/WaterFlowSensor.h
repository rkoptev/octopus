#ifndef WATER_FLOW_SENSOR_H
#define WATER_FLOW_SENSOR_H

#include <Arduino.h>
#include <EEPROM.h>

/* This class interfacing with water flow sensor. It use interrupts to precisely
handle sensor pulses. It also allow you to get flow rate in liters per minute
and counts amount of water that passes via sensor during all the time. It also
save counter to EEPROM to not lose counter. */

class WaterFlowSensor {
public:
  // A singletone instance needed because ISRs must be a static method
  static WaterFlowSensor* instance;

  WaterFlowSensor(uint8_t pin, int eeprom_address) : eepromAddress(eeprom_address) {
    // Assign singleton instance
    instance = this;
    // Attach interrupt to sensor pin
    attachInterrupt(digitalPinToInterrupt(pin), pulseISR, FALLING);
    // Read water counter value from EEPROM
    loadCounter();
  }

  /* Get flow rate in L/min */
  float getFlowRate() {
    // If a signal has not come from the sensor for a long time, or never come from start, then it is stopped.
    if (millis() - lastPulseTimeMs > 1000 || lastPulseTimeMs == 0) {
      return 0;
    }
    return 1000000UL / pulsePeriod / 7.5;
  }

  /* Get amount of all water used from beginning in liters */
  double getWaterAmount() {
    return waterCounter * litersPerPulse;
  }

private:
  static constexpr double litersPerPulse = 0.00222222222;
  static constexpr uint32_t eepromWriteInterval = 45000; // 45K pulses = 100 liters
  const int eepromAddress;

  // Variables, that needed for calculating flow
  volatile uint32_t lastPulseTime = 0, lastPulseTimeMs = 0;
  volatile uint32_t pulsePeriod;
  volatile uint32_t waterCounter = 0;

  // ISR for pulses from water sensor
  static void pulseISR() {
    instance->pulsePeriod = abs(micros() - instance->lastPulseTime);
    instance->lastPulseTime = micros();
    instance->lastPulseTimeMs = millis();
    instance->waterCounter++;
    // Save counter value to EEPROM if next 100 litres flowed
    if (instance->waterCounter % eepromWriteInterval == 0) {
      instance->saveCounter();
    }
  }

  /* Save water counter value to EEPROM */
  void saveCounter() {
    uint8_t* data = (uint8_t*)&waterCounter;
    int addr = eepromAddress;
    for (uint8_t i = 0; i < sizeof(waterCounter); i++) {
      EEPROM.write(addr++, *data++);
    }
  }

  /* Load water counter value from EEPROM */
  void loadCounter() {
    uint8_t* data = (uint8_t*)&waterCounter;
    int addr = eepromAddress;
    for (uint8_t i = 0; i < sizeof(waterCounter); i++) {
      *data++ = EEPROM.read(addr++);
    }
  }
};

WaterFlowSensor* WaterFlowSensor::instance = nullptr;

#endif //WATER_FLOW_SENSOR_H
