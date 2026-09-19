#include "apds_sensor.h"

#include <Arduino_APDS9960.h>

void setupAPDS9960()
{
  if (APDS.begin()) {
    APDS.setGestureSensitivity(50);
    APDS.setLEDBoost(3);
    Serial.println("APDS-9930 / APDS-9960 sensor initialized");
  } else {
    Serial.println("Error initializing APDS-9960 sensor.");
  }
}

APDS_DATA readAPDS() {
  APDS_DATA data = APDS_DATA{};
  int r = 0, g = 0, b = 0, c = 0;

  if (APDS.colorAvailable() && APDS.readColor(r, g, b, c)) {
    data.ambientLight = c;
  }

  data.gesture = APDS.gestureAvailable() ? APDS.readGesture() : -1;
  data.proximity = APDS.proximityAvailable() ? APDS.readProximity() : -1;

  return data;
}
