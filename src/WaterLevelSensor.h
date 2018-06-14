#ifndef WATER_LEVEL_SENSOR_H
#define WATER_LEVEL_SENSOR_H

#include <Arduino.h>
#include <NewPing.h>

class WaterLevelSensor : public NewPing {
public:
  WaterLevelSensor(uint8_t trigger_pin, uint8_t echo_pin, uint32_t min_distance, uint32_t max_distance)
    : NewPing(trigger_pin, echo_pin, max_distance),
    MIN_DISTANCE(min_distance), 
    MAX_DISTANCE(max_distance) {}

  /* Update measurements.
     Returns false if sensor can't measure water level.
   */
  bool update() {
    if (millis() - lastUpdateTime > UPDATE_INTERVAL || lastUpdateTime == 0) {
      lastUpdateTime = millis();
      distance = convert_cm(ping_median(RETRIES_COUNT, MAX_DISTANCE));

      // If sensor failed - set full tank and failure flag
      if (distance == 0) {
        return false;
      }
    }
    return true;
  }

  unsigned int getWaterLevel() {
    // Convert cm to %
    return map(constrain(distance, MIN_DISTANCE, MAX_DISTANCE), MIN_DISTANCE, MAX_DISTANCE, 100, 0);
  }

private:
  static constexpr uint32_t UPDATE_INTERVAL = 1000;
  static constexpr int RETRIES_COUNT = 10;

  const uint32_t MIN_DISTANCE;
  const uint32_t MAX_DISTANCE;

  uint32_t lastUpdateTime = 0;
  unsigned int distance = 0;
};

#endif //WATER_LEVEL_SENSOR_H
