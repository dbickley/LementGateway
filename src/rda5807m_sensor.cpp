#include "rda5807m_sensor.h"

#include <Wire.h>
#include <RDA5807M.h>

namespace {
RDA5807M radio;
bool radioPresent = false;
bool radioPowered = false;
bool radioMuted = true;
bool radioStereo = false;
int radioVolume = 0;
float radioFrequencyMHz = 98.10f;
int radioSignalStrength = 0;
char psBuffer[9] = "        ";
String radioRdsStation = "";
String radioRdsText = "";
bool amplifierEnabled = false;

void setRadioVolumeFromHardware(uint8_t volume) {
  if (volume > 15) {
    volume = 15;
  }

  const uint16_t currentRegister = radio.getRegister(RDA5807M_REG_VOLUME);
  const uint16_t masked = currentRegister & ~RDA5807M_VOLUME_MASK;
  radio.setRegister(RDA5807M_REG_VOLUME, masked | static_cast<uint16_t>(volume));
}

void updateRadioStatus() {
  if (!radioPresent) {
    return;
  }

  const uint16_t status = radio.getRegister(RDA5807M_REG_STATUS);
  radioStereo = (status & RDA5807M_STATUS_ST) != 0;
  radioSignalStrength = static_cast<int>(radio.getRSSI());
  radioFrequencyMHz = radio.getFrequency() / 100.0f;

  // Process RDS data when a block is synchronized and ready
  if (status & RDA5807M_STATUS_RDSS) {
    const uint16_t rdsB = radio.getRegister(RDA5807M_REG_RDSB);
    const uint16_t rdsD = radio.getRegister(RDA5807M_REG_RDSD);

    // Group 0A / 0B: Program Service (PS) name - 8 characters transmitted in 4 segments
    uint8_t groupType = static_cast<uint8_t>((rdsB >> 12) & 0x0F);
    if (groupType == 0) {
      uint8_t segment = static_cast<uint8_t>(rdsB & 0x03);
      char c1 = static_cast<char>((rdsD >> 8) & 0xFF);
      char c2 = static_cast<char>(rdsD & 0xFF);

      if (c1 >= 32 && c1 <= 126) psBuffer[segment * 2] = c1;
      if (c2 >= 32 && c2 <= 126) psBuffer[segment * 2 + 1] = c2;
      psBuffer[8] = '\0';

      String candidate = String(psBuffer);
      candidate.trim();
      if (candidate.length() > 0) {
        radioRdsStation = candidate;
      }
    }
  }
  // Note: We retain radioRdsStation / radioRdsText across polls instead of
  // clearing on ticks where a new RDS block is not immediately ready.
}
}  // namespace

void setupRDA5807M() {
  pinMode(LementGatewayConfig::AMPLIFIER_ENABLE_PIN, OUTPUT);
  digitalWrite(LementGatewayConfig::AMPLIFIER_ENABLE_PIN, LOW);
  amplifierEnabled = false;

  radioPresent = false;
  radioPowered = false;
  radioMuted = true;
  radioStereo = false;
  radioSignalStrength = 0;
  radioVolume = 0;
  radioRdsStation = "";
  radioRdsText = "";
  memset(psBuffer, ' ', 8);
  psBuffer[8] = '\0';

  // Probe I2C to see if RDA5807M is physically present (0x10 or 0x11)
  Wire.beginTransmission(0x10);
  bool found = (Wire.endTransmission() == 0);
  if (!found) {
    Wire.beginTransmission(0x11);
    found = (Wire.endTransmission() == 0);
  }

  if (!found) {
    Serial.println("RDA5807M radio not detected on I2C bus");
    return;
  }

  radio.begin(RDA5807M_BAND_WEST);
  radioPresent = true;
  radioPowered = true;
  radioSetFrequency(98.10f);
  radioSetVolume(10);
  radioSetMute(true);

  updateRadioStatus();
  Serial.println("RDA5807M initialized on GPIO6/GPIO7 I2C bus");
}

RDA5807M_DATA readRDA5807M() {
  RDA5807M_DATA data = RDA5807M_DATA{};
  data.lastSensorReadingMillis = millis();
  if (!radioPresent) {
    data.valid = false;
    return data;
  }

  updateRadioStatus();
  data.powered = radioPowered;
  data.frequencyMHz = radioFrequencyMHz;
  data.volume = radioVolume;
  data.muted = radioMuted;
  data.stereo = radioStereo;
  data.signalStrength = radioSignalStrength;
  data.rdsStation = radioRdsStation;
  data.rdsText = radioRdsText;
  data.amplifierEnabled = amplifierEnabled;
  data.valid = true;
  return data;
}

bool radioBegin() {
  setupRDA5807M();
  return radioPresent;
}

bool radioSetFrequency(float frequencyMHz) {
  if (!radioPresent) {
    return false;
  }

  const uint16_t frequency = static_cast<uint16_t>(frequencyMHz * 100.0f + 0.5f);
  if (!radio.setFrequency(frequency)) {
    return false;
  }

  radioFrequencyMHz = frequencyMHz;
  radioRdsStation = "";
  radioRdsText = "";
  memset(psBuffer, ' ', 8);
  psBuffer[8] = '\0';
  return true;
}

bool radioSeekUp() {
  if (!radioPresent) {
    return false;
  }

  radio.seekUp(true);
  updateRadioStatus();
  return true;
}

bool radioSeekDown() {
  if (!radioPresent) {
    return false;
  }

  radio.seekDown(true);
  updateRadioStatus();
  return true;
}

bool radioSetVolume(uint8_t volume) {
  if (!radioPresent) {
    return false;
  }

  if (volume > 15) {
    return false;
  }

  setRadioVolumeFromHardware(volume);
  radioVolume = static_cast<int>(volume);
  return true;
}

bool radioSetMute(bool muted) {
  if (!radioPresent) {
    return false;
  }

  if (muted) {
    radio.mute();
  } else {
    radio.unMute(false);
  }

  radioMuted = muted;
  return true;
}

bool radioSetPowered(bool powered) {
  if (!radioPresent && powered) {
    radio.begin(RDA5807M_BAND_WEST);
    radioPresent = true;
  }

  if (!radioPresent && !powered) {
    return false;
  }

  if (powered) {
    radio.begin(RDA5807M_BAND_WEST);
    radioPowered = true;
    radioMuted = true;
    radioSetVolume(radioVolume > 0 ? static_cast<uint8_t>(radioVolume) : 10);
    radioSetMute(true);
    return true;
  }

  radio.end();
  radioPowered = false;
  radioMuted = true;
  return true;
}

bool radioSetAmplifierEnabled(bool enabled) {
  digitalWrite(LementGatewayConfig::AMPLIFIER_ENABLE_PIN, enabled ? HIGH : LOW);
  amplifierEnabled = enabled;
  return true;
}

bool radioIsPowered() {
  return radioPowered;
}

int radioGetSignalStrength() {
  return radioSignalStrength;
}

bool radioIsStereo() {
  return radioStereo;
}

String radioGetRdsStation() {
  return radioRdsStation;
}

String radioGetRdsText() {
  return radioRdsText;
}
