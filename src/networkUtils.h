#ifndef NETWORKUTILS_H  // Header guard to prevent multiple inclusions
#define NETWORKUTILS_H
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <sensor_data.h>
#include <Preferences.h>

void setupNetworkUtils();
void setupOTA();
void handleNetworkPortal();
void handleOTA();
void broadcastStateEvent(const String& eventType, const String& stateJson);
bool isProvisioningModeActive();
bool logData(const SENSOR_DATA& sensor_data);
time_t getCurrentTime();
#endif 