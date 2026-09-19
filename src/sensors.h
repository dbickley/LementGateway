#ifndef SENSORS_H
#define SENSORS_H

#include <Adafruit_BME280.h>
#include <Arduino_APDS9960.h>

#include "sensor_data.h"

void setupAPDS9960();
void setupBME280();

// RCWL-1655 (motion/pulse) sensor using trigger+echo pins
void setupRCWL();
void setupRCWL(int triggerPin, int echoPin, const char* label = "rcwl");

// Reads RCWL and returns RCWL_DATA
RCWL_DATA readRCWL();
RCWL_DATA readRCWL(int triggerPin, int echoPin);

void setupBackupSensors();
void readBackupSensors(SENSOR_DATA& sensorData);
void initOBD2Interface();
void initDataOut();
void writeDataOut(const SENSOR_DATA& sensorData, const String& timeStamp);

APDS_DATA readAPDS();
BME280_DATA readBME280Values();

#endif