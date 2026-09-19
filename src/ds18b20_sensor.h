#ifndef DS18B20_SENSOR_H
#define DS18B20_SENSOR_H

#include <Arduino.h>

#include "sensor_data.h"

void setupDS18B20();
void readDS18B20(SENSOR_DATA& sensorData);

#endif
