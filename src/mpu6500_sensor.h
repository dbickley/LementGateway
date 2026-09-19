#ifndef MPU6500_SENSOR_H
#define MPU6500_SENSOR_H

#include <Arduino.h>

#include "sensor_data.h"

void setupMPU6500();
MPU6500_DATA readMPU6500Values();

#endif
