#include "rcwl_sensor.h"

void setupRCWL()
{
  setupRCWL(LementGatewayConfig::PRIMARY_RCWL_TRIGGER_PIN, LementGatewayConfig::PRIMARY_RCWL_ECHO_PIN, "primary");
}

void setupRCWL(int triggerPin, int echoPin, const char* label)
{
  pinMode(triggerPin, OUTPUT);
  digitalWrite(triggerPin, LOW);
  pinMode(echoPin, INPUT);
  Serial.printf("RCWL %s set: trigger=%d echo=%d\n", label, triggerPin, echoPin);
}

RCWL_DATA readRCWL()
{
  return readRCWL(LementGatewayConfig::PRIMARY_RCWL_TRIGGER_PIN, LementGatewayConfig::PRIMARY_RCWL_ECHO_PIN);
}

RCWL_DATA readRCWL(int triggerPin, int echoPin)
{
  RCWL_DATA data = RCWL_DATA{};

  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);

  // 15ms timeout corresponds to ~2.5m distance, preventing long freezes on missing echoes
  unsigned long duration = pulseIn(echoPin, HIGH, 15000);
  data.pulseWidth = duration;
  data.motionDetected = (duration > 0);

  if (duration > 0) {
    data.distanceCm = (float)duration / 58.0f;
  } else {
    data.distanceCm = -1.0f;
  }

  return data;
}

void setupBackupSensors()
{
  for (int i = 0; i < LementGatewayConfig::BACKUP_SENSOR_COUNT; ++i) {
    setupRCWL(LementGatewayConfig::BACKUP_RCWL_TRIGGER_PINS[i], LementGatewayConfig::BACKUP_RCWL_ECHO_PINS[i], "backup");
  }
}

void readBackupSensors(SENSOR_DATA& sensorData)
{
  for (int i = 0; i < LementGatewayConfig::BACKUP_SENSOR_COUNT; ++i) {
    sensorData.backupSensors[i] = readRCWL(
      LementGatewayConfig::BACKUP_RCWL_TRIGGER_PINS[i],
      LementGatewayConfig::BACKUP_RCWL_ECHO_PINS[i]);
    sensorData.backupSensors[i].lastSensorReadingMillis = millis();
  }
}
