#ifndef WATER_LEVEL_SENSOR_H
#define WATER_LEVEL_SENSOR_H

#include <Arduino.h>
#include <NewPing.h>

class WaterLevelSensor {
public:
  WaterLevelSensor(NewPing* ultrasonic, uint32_t min_distance, uint32_t max_distance)
  : ultrasonic(ultrasonic),
    MIN_DISTANCE(min_distance),
    MAX_DISTANCE(max_distance) {}

  /* Update measurements.
     Returns false if sensor can't measure water level.
   */
  bool update() {
    if (millis() - lastUpdateTime > UPDATE_INTERVAL || lastUpdateTime == 0) {
      lastUpdateTime = millis();
      distance = ultrasonic->convert_cm(ultrasonic->ping_median(RETRIES_COUNT, MAX_DISTANCE));

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

  NewPing* ultrasonic;
  const uint32_t MIN_DISTANCE;
  const uint32_t MAX_DISTANCE;

  uint32_t lastUpdateTime = 0;
  unsigned int distance = 0;
};

#endif //WATER_LEVEL_SENSOR_H
