#ifndef RCWL_SENSOR_H
#define RCWL_SENSOR_H

#include <Arduino.h>

#include "sensor_data.h"

void setupRCWL();
void setupRCWL(int triggerPin, int echoPin, const char* label = "rcwl");
RCWL_DATA readRCWL();
RCWL_DATA readRCWL(int triggerPin, int echoPin);
void setupBackupSensors();
void readBackupSensors(SENSOR_DATA& sensorData);

#endif
