#ifndef STATE_EVENT_H
#define STATE_EVENT_H

#include <Arduino.h>

#include "sensor_data.h"

class DeviceListener {
public:
  virtual ~DeviceListener() = default;
  virtual void onStateEvent(const String& eventType, const String& stateJson) = 0;
};

class StateEventBus {
public:
  StateEventBus();

  void registerListener(DeviceListener* listener);
  void unregisterListener(DeviceListener* listener);
  void publishState(const SENSOR_DATA& state);

private:
  DeviceListener* listeners[8];
  size_t listenerCount;
};

bool sensorStateChanged(const SENSOR_DATA& previous, const SENSOR_DATA& current);
String serializeSensorState(const SENSOR_DATA& sensorData);

#endif
