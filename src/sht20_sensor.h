#ifndef SHT20_SENSOR_H
#define SHT20_SENSOR_H

#include <Arduino.h>

#include "sensor_data.h"

void setupSHT20();
SHT20_DATA readSHT20Values();

#endif
