#ifndef DATA_OUT_H
#define DATA_OUT_H

#include <Arduino.h>

#include "sensor_data.h"

void initDataOut();
void writeDataOut(const SENSOR_DATA& sensorData, const String& timeStamp);

#endif
