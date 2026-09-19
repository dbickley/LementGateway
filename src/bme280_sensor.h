#ifndef BME280_SENSOR_H
#define BME280_SENSOR_H

#include <Arduino.h>

#include "sensor_data.h"

void setupBME280();
BME280_DATA readBME280Values();

#endif
