#ifndef RDA5807M_SENSOR_H
#define RDA5807M_SENSOR_H

#include <Arduino.h>

#include "sensor_data.h"

void setupRDA5807M();
RDA5807M_DATA readRDA5807M();

bool radioBegin();
bool radioSetFrequency(float frequencyMHz);
bool radioSeekUp();
bool radioSeekDown();
bool radioSetVolume(uint8_t volume);
bool radioSetMute(bool muted);
bool radioSetPowered(bool powered);
bool radioSetAmplifierEnabled(bool enabled);
bool radioIsPowered();
int radioGetSignalStrength();
bool radioIsStereo();
String radioGetRdsStation();
String radioGetRdsText();

#endif
