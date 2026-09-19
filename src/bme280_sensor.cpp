#include "bme280_sensor.h"

#include <Adafruit_BME280.h>

Adafruit_BME280 bme;

void setupBME280()
{
  bool status = bme.begin(0x76) || bme.begin(0x77);

  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    return;
  }

  bme.setSampling(Adafruit_BME280::MODE_NORMAL,
                  Adafruit_BME280::SAMPLING_X2,
                  Adafruit_BME280::SAMPLING_X16,
                  Adafruit_BME280::SAMPLING_X16,
                  Adafruit_BME280::FILTER_X16,
                  Adafruit_BME280::STANDBY_MS_1000);

  Serial.println("BME280 sensor initialized");
}

BME280_DATA readBME280Values() {
  BME280_DATA data = BME280_DATA{};

  data.temperature = bme.readTemperature();
  data.pressure = bme.readPressure() / 100.0F;
  data.humidity = bme.readHumidity();

  return data;
}
