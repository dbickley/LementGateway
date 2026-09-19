#include "sht20_sensor.h"

#include <Wire.h>

namespace {
const uint8_t SHT20_ADDRESS = 0x40;
const uint8_t SHT20_CMD_MEASURE_HUMIDITY = 0xF3;
const uint8_t SHT20_CMD_MEASURE_TEMPERATURE = 0xF5;
const uint8_t SHT20_CMD_SOFT_RESET = 0xFE;
bool sht20Present = false;

float convertHumidity(uint16_t rawHumidity) {
  return -6.0f + 125.0f * (static_cast<float>(rawHumidity) / 65536.0f);
}

float convertTemperature(uint16_t rawTemperature) {
  return -46.85f + 175.72f * (static_cast<float>(rawTemperature) / 65536.0f);
}

bool readSHT20Measurement(uint8_t command, uint16_t& rawValue) {
  Wire.beginTransmission(SHT20_ADDRESS);
  Wire.write(command);
  if (Wire.endTransmission() != 0) {
    return false;
  }

  const uint8_t waitMs = (command == SHT20_CMD_MEASURE_HUMIDITY) ? 25 : 20;
  delay(waitMs);

  if (Wire.requestFrom(static_cast<int>(SHT20_ADDRESS), 2) != 2) {
    return false;
  }

  uint8_t msb = Wire.read();
  uint8_t lsb = Wire.read();
  // Per Sensirion SHT20 datasheet, bits 0 and 1 of LSB are status bits and must be cleared
  rawValue = static_cast<uint16_t>((msb << 8) | (lsb & ~0x03));

  return true;
}
}

void setupSHT20() {
  delay(10);
  Wire.beginTransmission(SHT20_ADDRESS);
  if (Wire.endTransmission() != 0) {
    Serial.println("SHT20 not detected on I2C bus");
    sht20Present = false;
    return;
  }

  Wire.beginTransmission(SHT20_ADDRESS);
  Wire.write(SHT20_CMD_SOFT_RESET);
  if (Wire.endTransmission() == 0) {
    delay(15);
    sht20Present = true;
    Serial.println("SHT20 sensor initialized");
  } else {
    sht20Present = false;
    Serial.println("SHT20 initialization failed");
  }
}

SHT20_DATA readSHT20Values() {
  SHT20_DATA data = SHT20_DATA{};
  data.lastSensorReadingMillis = millis();

  if (!sht20Present) {
    data.valid = false;
    return data;
  }

  uint16_t rawHumidity = 0;
  uint16_t rawTemperature = 0;

  if (!readSHT20Measurement(SHT20_CMD_MEASURE_HUMIDITY, rawHumidity) ||
      !readSHT20Measurement(SHT20_CMD_MEASURE_TEMPERATURE, rawTemperature)) {
    Serial.println("SHT20 read failed");
    data.valid = false;
    return data;
  }

  data.humidityPercent = convertHumidity(rawHumidity);
  data.temperatureC = convertTemperature(rawTemperature);
  data.valid = true;

  Serial.printf("SHT20 temp: %.2f C | humidity: %.2f %%\n",
                data.temperatureC,
                data.humidityPercent);

  return data;
}
