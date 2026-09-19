#include "state_event.h"

#include "networkUtils.h"

namespace {
String serializeNumber(float value) {
  if (isnan(value) || isinf(value)) {
    return "null";
  }
  return String(value, 4);
}

bool floatChanged(float a, float b, float threshold) {
  if (isnan(a) != isnan(b)) return true;
  if (isnan(a) && isnan(b)) return false;
  return fabs(a - b) >= threshold;
}

bool pulseChanged(unsigned long a, unsigned long b, unsigned long threshold) {
  return (a > b ? a - b : b - a) >= threshold;
}

bool backupSensorsChanged(const SENSOR_DATA& previous, const SENSOR_DATA& current) {
  for (int i = 0; i < LementGatewayConfig::BACKUP_SENSOR_COUNT; ++i) {
    if (previous.backupSensors[i].motionDetected != current.backupSensors[i].motionDetected ||
        pulseChanged(previous.backupSensors[i].pulseWidth, current.backupSensors[i].pulseWidth, 50) ||
        floatChanged(previous.backupSensors[i].distanceCm, current.backupSensors[i].distanceCm, 1.0f)) {
      return true;
    }
  }
  return false;
}
}

StateEventBus::StateEventBus() : listeners{}, listenerCount(0) {}

void StateEventBus::registerListener(DeviceListener* listener) {
  if (listener == nullptr || listenerCount >= 8) {
    return;
  }

  for (size_t i = 0; i < listenerCount; ++i) {
    if (listeners[i] == listener) {
      return;
    }
  }

  listeners[listenerCount++] = listener;
}

void StateEventBus::unregisterListener(DeviceListener* listener) {
  if (listener == nullptr) {
    return;
  }

  for (size_t i = 0; i < listenerCount; ++i) {
    if (listeners[i] == listener) {
      for (size_t j = i + 1; j < listenerCount; ++j) {
        listeners[j - 1] = listeners[j];
      }
      listeners[--listenerCount] = nullptr;
      return;
    }
  }
}

void StateEventBus::publishState(const SENSOR_DATA& state) {
  String stateJson = serializeSensorState(state);

  for (size_t i = 0; i < listenerCount; ++i) {
    if (listeners[i] != nullptr) {
      listeners[i]->onStateEvent("state_changed", stateJson);
    }
  }

  broadcastStateEvent("state_changed", stateJson);
}

bool sensorStateChanged(const SENSOR_DATA& previous, const SENSOR_DATA& current) {
  return previous.apds.proximity != current.apds.proximity ||
         previous.apds.gesture != current.apds.gesture ||
         previous.apds.ambientLight != current.apds.ambientLight ||
         floatChanged(previous.bme280.temperature, current.bme280.temperature, 0.2f) ||
         floatChanged(previous.bme280.pressure, current.bme280.pressure, 0.5f) ||
         floatChanged(previous.bme280.humidity, current.bme280.humidity, 0.5f) ||
         floatChanged(previous.sht20.temperatureC, current.sht20.temperatureC, 0.2f) ||
         floatChanged(previous.sht20.humidityPercent, current.sht20.humidityPercent, 0.5f) ||
         previous.sht20.valid != current.sht20.valid ||
         previous.radio.powered != current.radio.powered ||
         floatChanged(previous.radio.frequencyMHz, current.radio.frequencyMHz, 0.05f) ||
         previous.radio.volume != current.radio.volume ||
         previous.radio.muted != current.radio.muted ||
         previous.radio.stereo != current.radio.stereo ||
         abs(previous.radio.signalStrength - current.radio.signalStrength) > 2 ||
         previous.radio.rdsStation != current.radio.rdsStation ||
         previous.radio.amplifierEnabled != current.radio.amplifierEnabled ||
         previous.radio.valid != current.radio.valid ||
         floatChanged(previous.engineTemp.temperatureC, current.engineTemp.temperatureC, 0.25f) ||
         previous.engineTemp.valid != current.engineTemp.valid ||
         previous.rcwl.motionDetected != current.rcwl.motionDetected ||
         floatChanged(previous.rcwl.distanceCm, current.rcwl.distanceCm, 1.0f) ||
         pulseChanged(previous.rcwl.pulseWidth, current.rcwl.pulseWidth, 50) ||
         previous.obd2.enabled != current.obd2.enabled ||
         previous.obd2.packetsReceived != current.obd2.packetsReceived ||
         previous.obd2.rpm != current.obd2.rpm ||
         previous.obd2.vehicleSpeedKph != current.obd2.vehicleSpeedKph ||
         floatChanged(previous.obd2.coolantTempC, current.obd2.coolantTempC, 0.5f) ||
         floatChanged(previous.obd2.intakeTempC, current.obd2.intakeTempC, 0.5f) ||
         floatChanged(previous.obd2.throttlePositionPct, current.obd2.throttlePositionPct, 1.0f) ||
         floatChanged(previous.obd2.fuelLevelPct, current.obd2.fuelLevelPct, 1.0f) ||
         floatChanged(previous.obd2.batteryVoltage, current.obd2.batteryVoltage, 0.1f) ||
         floatChanged(previous.obd2.engineLoadPct, current.obd2.engineLoadPct, 1.0f) ||
         floatChanged(previous.obd2.oilTempC, current.obd2.oilTempC, 0.5f) ||
         floatChanged(previous.obd2.mafGramsPerSec, current.obd2.mafGramsPerSec, 0.5f) ||
         previous.dataOut.enabled != current.dataOut.enabled ||
         previous.dataOut.packetsWritten != current.dataOut.packetsWritten ||
         backupSensorsChanged(previous, current);
}

String serializeSensorState(const SENSOR_DATA& sensorData) {
  String json;
  json.reserve(1024);
  json = "{";
  json += "\"apds\":{";
  json += "\"proximity\":" + String(sensorData.apds.proximity) + ",";
  json += "\"gesture\":" + String(sensorData.apds.gesture) + ",";
  json += "\"ambientLight\":" + String(sensorData.apds.ambientLight);
  json += "},";

  json += "\"bme280\":{";
  json += "\"temperature\":" + serializeNumber(sensorData.bme280.temperature) + ",";
  json += "\"pressure\":" + serializeNumber(sensorData.bme280.pressure) + ",";
  json += "\"humidity\":" + serializeNumber(sensorData.bme280.humidity);
  json += "},";

  json += "\"sht20\":{";
  json += "\"temperatureC\":" + serializeNumber(sensorData.sht20.temperatureC) + ",";
  json += "\"humidityPercent\":" + serializeNumber(sensorData.sht20.humidityPercent) + ",";
  json += "\"valid\":" + String(sensorData.sht20.valid ? 1 : 0);
  json += "},";

  json += "\"radio\":{";
  json += "\"powered\":" + String(sensorData.radio.powered ? 1 : 0) + ",";
  json += "\"frequencyMHz\":" + serializeNumber(sensorData.radio.frequencyMHz) + ",";
  json += "\"volume\":" + String(sensorData.radio.volume) + ",";
  json += "\"muted\":" + String(sensorData.radio.muted ? 1 : 0) + ",";
  json += "\"stereo\":" + String(sensorData.radio.stereo ? 1 : 0) + ",";
  json += "\"signalStrength\":" + String(sensorData.radio.signalStrength) + ",";
  json += "\"rdsStation\":\"" + String(sensorData.radio.rdsStation) + "\",";
  json += "\"rdsText\":\"" + String(sensorData.radio.rdsText) + "\",";
  json += "\"amplifierEnabled\":" + String(sensorData.radio.amplifierEnabled ? 1 : 0) + ",";
  json += "\"valid\":" + String(sensorData.radio.valid ? 1 : 0);
  json += "},";

  json += "\"engineTemp\":{";
  json += "\"temperatureC\":" + serializeNumber(sensorData.engineTemp.temperatureC) + ",";
  json += "\"valid\":" + String(sensorData.engineTemp.valid ? 1 : 0);
  json += "},";

  json += "\"rcwl\":{";
  json += "\"pulseWidth\":" + String(sensorData.rcwl.pulseWidth) + ",";
  json += "\"motionDetected\":" + String(sensorData.rcwl.motionDetected ? 1 : 0) + ",";
  json += "\"distanceCm\":" + serializeNumber(sensorData.rcwl.distanceCm);
  json += "},";

  json += "\"backupSensors\":[";
  for (int i = 0; i < LementGatewayConfig::BACKUP_SENSOR_COUNT; ++i) {
    if (i > 0) {
      json += ",";
    }
    json += "{";
    json += "\"pulseWidth\":" + String(sensorData.backupSensors[i].pulseWidth) + ",";
    json += "\"motionDetected\":" + String(sensorData.backupSensors[i].motionDetected ? 1 : 0) + ",";
    json += "\"distanceCm\":" + serializeNumber(sensorData.backupSensors[i].distanceCm);
    json += "}";
  }
  json += "],";

  json += "\"obd2\":{";
  json += "\"enabled\":" + String(sensorData.obd2.enabled ? 1 : 0) + ",";
  json += "\"packetsReceived\":" + String(sensorData.obd2.packetsReceived) + ",";
  json += "\"rpm\":" + String(sensorData.obd2.rpm) + ",";
  json += "\"vehicleSpeedKph\":" + String(sensorData.obd2.vehicleSpeedKph) + ",";
  json += "\"coolantTempC\":" + serializeNumber(sensorData.obd2.coolantTempC) + ",";
  json += "\"intakeTempC\":" + serializeNumber(sensorData.obd2.intakeTempC) + ",";
  json += "\"throttlePositionPct\":" + serializeNumber(sensorData.obd2.throttlePositionPct) + ",";
  json += "\"fuelLevelPct\":" + serializeNumber(sensorData.obd2.fuelLevelPct) + ",";
  json += "\"batteryVoltage\":" + serializeNumber(sensorData.obd2.batteryVoltage) + ",";
  json += "\"engineLoadPct\":" + serializeNumber(sensorData.obd2.engineLoadPct) + ",";
  json += "\"oilTempC\":" + serializeNumber(sensorData.obd2.oilTempC) + ",";
  json += "\"mafGramsPerSec\":" + serializeNumber(sensorData.obd2.mafGramsPerSec);
  json += "},";

  json += "\"dataOut\":{";
  json += "\"enabled\":" + String(sensorData.dataOut.enabled ? 1 : 0) + ",";
  json += "\"packetsWritten\":" + String(sensorData.dataOut.packetsWritten);
  json += "}";

  json += "}";
  return json;
}
