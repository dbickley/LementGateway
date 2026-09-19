#include <Arduino.h>
#include <Wire.h>

#include "networkUtils.h"
#include "sensor_data.h"
#include "apds_sensor.h"
#include "bme280_sensor.h"
#include "rcwl_sensor.h"
#include "obd2_interface.h"
#include "data_out.h"
#include "ds18b20_sensor.h"
#include "sht20_sensor.h"
#include "mpu6500_sensor.h"
#include "rda5807m_sensor.h"
#include "state_event.h"

const int logDataUpdateSpeedMillis = 10000;
unsigned long lastLogDataMillis = 0;

const int apdsUpdateSpeedMillis = 100;
const int bmeUpdateSpeedMillis = 5000;
const int rcwlUpdateSpeedMillis = 500;
const int radioUpdateSpeedMillis = 2000;
const int mpuUpdateSpeedMillis = 200;

SENSOR_DATA sensor_data;
SENSOR_DATA lastPublishedState;
bool hasPublishedState = false;
StateEventBus stateEventBus;
time_t current_time = 0;
bool data_logged_successfully = false;

void setupTime() {
  current_time = getCurrentTime();
  struct timeval tv;
  tv.tv_sec = current_time;
  settimeofday(&tv, NULL);
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("\n\n======================================");
  Serial.println("LementGateway Startup");
  Serial.println("======================================\n");

  Wire.begin(LementGatewayConfig::SHT20_SDA_PIN, LementGatewayConfig::SHT20_SCL_PIN);

  Serial.println("Setting timezone...");
  setenv("TZ", "PST8PDT", 1);

  Serial.println("Initializing sensors...");
  setupAPDS9960();
  setupBME280();
  setupSHT20();
  setupMPU6500();
  setupDS18B20();
  setupRDA5807M();
  setupRCWL();
  setupBackupSensors();
  initOBD2Interface();
  initDataOut();

  Serial.println("Initializing network...");
  setupNetworkUtils();
  setupOTA();

  Serial.println("Initializing time...");
  setupTime();

  Serial.println("\nSetup Complete\n");
}

String formatTime(tm *localtime) {
  char timebuffer[20];
  strftime(timebuffer, sizeof(timebuffer), "%Y-%m-%d %H:%M:%S", localtime);
  return String(timebuffer);
}

void loop() {
  delay(10);

  time_t now = time(nullptr);
  struct tm *timeInfo = localtime(&now);
  unsigned long currentMillis = millis();

  if (currentMillis - sensor_data.apds.lastSensorReadingMillis >= apdsUpdateSpeedMillis) {
    sensor_data.apds = readAPDS();
    sensor_data.apds.lastSensorReadingMillis = currentMillis;
  }

  if (currentMillis - sensor_data.rcwl.lastSensorReadingMillis >= rcwlUpdateSpeedMillis) {
    sensor_data.rcwl = readRCWL();
    sensor_data.rcwl.lastSensorReadingMillis = currentMillis;
    Serial.printf("RCWL: pulse=%lu us motion=%d | distance=%.2f cm\n",
                  sensor_data.rcwl.pulseWidth,
                  sensor_data.rcwl.motionDetected ? 1 : 0,
                  sensor_data.rcwl.distanceCm);
  }

  if (currentMillis - sensor_data.bme280.lastSensorReadingMillis >= bmeUpdateSpeedMillis) {
    sensor_data.bme280 = readBME280Values();
    sensor_data.bme280.lastSensorReadingMillis = currentMillis;
  }

  if (currentMillis - sensor_data.sht20.lastSensorReadingMillis >= bmeUpdateSpeedMillis) {
    sensor_data.sht20 = readSHT20Values();
    sensor_data.sht20.lastSensorReadingMillis = currentMillis;
  }

  if (currentMillis - sensor_data.mpu6500.lastSensorReadingMillis >= mpuUpdateSpeedMillis) {
    sensor_data.mpu6500 = readMPU6500Values();
    sensor_data.mpu6500.lastSensorReadingMillis = currentMillis;
  }

  if (currentMillis - sensor_data.radio.lastSensorReadingMillis >= radioUpdateSpeedMillis) {
    sensor_data.radio = readRDA5807M();
    sensor_data.radio.lastSensorReadingMillis = currentMillis;
  }

  if (currentMillis - sensor_data.engineTemp.lastSensorReadingMillis >= bmeUpdateSpeedMillis) {
    readDS18B20(sensor_data);
  }

  if (currentMillis - sensor_data.backupSensors[0].lastSensorReadingMillis >= rcwlUpdateSpeedMillis * 2) {
    readBackupSensors(sensor_data);
  }

  if (!hasPublishedState || sensorStateChanged(lastPublishedState, sensor_data)) {
    stateEventBus.publishState(sensor_data);
    lastPublishedState = sensor_data;
    hasPublishedState = true;
  }

  if (currentMillis - lastLogDataMillis >= logDataUpdateSpeedMillis) {
    data_logged_successfully = logData(sensor_data);
    String timeStamp = formatTime(timeInfo);
    writeDataOut(sensor_data, timeStamp);
    Serial.println(timeStamp);
    Serial.println();
    lastLogDataMillis = currentMillis;
  }

  handleOTA();
  handleNetworkPortal();

  static bool lastProvisioningState = false;
  bool currentProvisioningState = isProvisioningModeActive();

  if (currentProvisioningState != lastProvisioningState) {
    lastProvisioningState = currentProvisioningState;
    if (currentProvisioningState) {
      Serial.println("*** PROVISIONING MODE ACTIVE ***");
      Serial.println("1. Connect your phone/PC WiFi to: LementGateway_Setup");
      Serial.println("2. Open a browser and go to http://192.168.4.1");
    } else {
      Serial.println("Provisioning mode inactive");
    }
  }
}
