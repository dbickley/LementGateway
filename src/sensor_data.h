#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#include <Arduino.h>

#include "sensor_config.h"

struct RCWL_DATA {
  unsigned long lastSensorReadingMillis = 0;
  unsigned long pulseWidth = 0;
  bool motionDetected = false;
  float distanceCm = -1.0f;
};

struct APDS_DATA {
  unsigned long lastSensorReadingMillis = 0;
  int proximity = -1;
  int gesture = -1;
  int ambientLight = -1;
};

struct BME280_DATA {
  unsigned long lastSensorReadingMillis = 0;
  float temperature = 0;
  float pressure = 0;
  float humidity = 0;
};

struct DS18B20_DATA {
  unsigned long lastSensorReadingMillis = 0;
  float temperatureC = -127.0f;
  bool valid = false;
};

struct SHT20_DATA {
  unsigned long lastSensorReadingMillis = 0;
  float temperatureC = -127.0f;
  float humidityPercent = -1.0f;
  bool valid = false;
};

struct MPU6500_DATA {
  unsigned long lastSensorReadingMillis = 0;
  float accelX = 0.0f;
  float accelY = 0.0f;
  float accelZ = 0.0f;
  float gyroX = 0.0f;
  float gyroY = 0.0f;
  float gyroZ = 0.0f;
  bool valid = false;
};

struct RDA5807M_DATA {
  unsigned long lastSensorReadingMillis = 0;
  bool powered = false;
  float frequencyMHz = 0.0f;
  int volume = 0;
  bool muted = false;
  bool stereo = false;
  int signalStrength = 0;
  String rdsStation = "";
  String rdsText = "";
  bool amplifierEnabled = false;
  bool valid = false;
};

struct OBD2_INTERFACE_DATA {
  bool enabled = false;
  unsigned long lastReadMillis = 0;
  uint32_t packetsReceived = 0;
  uint32_t rpm = 0;
  uint32_t vehicleSpeedKph = 0;
  float coolantTempC = -127.0f;
  float intakeTempC = -127.0f;
  float throttlePositionPct = -1.0f;
  float fuelLevelPct = -1.0f;
  float batteryVoltage = 0.0f;
  float engineLoadPct = -1.0f;
  float oilTempC = -127.0f;
  float mafGramsPerSec = -1.0f;
};

struct DATA_OUT_DATA {
  bool enabled = false;
  unsigned long lastWriteMillis = 0;
  uint32_t packetsWritten = 0;
};

struct SENSOR_DATA {
  APDS_DATA apds;
  BME280_DATA bme280;
  SHT20_DATA sht20;
  MPU6500_DATA mpu6500;
  RDA5807M_DATA radio;
  DS18B20_DATA engineTemp;
  RCWL_DATA rcwl;
  RCWL_DATA backupSensors[LementGatewayConfig::BACKUP_SENSOR_COUNT];
  OBD2_INTERFACE_DATA obd2;
  DATA_OUT_DATA dataOut;
};

#endif