#include "ds18b20_sensor.h"

#include <OneWire.h>
#include <DallasTemperature.h>

OneWire oneWire(LementGatewayConfig::DS18B20_DATA_PIN);
DallasTemperature ds18b20(&oneWire);

void setupDS18B20()
{
  ds18b20.begin();
  ds18b20.setWaitForConversion(false);
  ds18b20.requestTemperatures();
  Serial.printf("DS18B20 initialized on GPIO %d (non-blocking)\n", LementGatewayConfig::DS18B20_DATA_PIN);
  if (ds18b20.getDeviceCount() == 0) {
    Serial.println("No DS18B20 devices found");
  }
}

void readDS18B20(SENSOR_DATA& sensorData)
{
  float tempC = ds18b20.getTempCByIndex(0);
  ds18b20.requestTemperatures();

  sensorData.engineTemp.temperatureC = tempC;
  sensorData.engineTemp.valid = (tempC != DEVICE_DISCONNECTED_C);
  sensorData.engineTemp.lastSensorReadingMillis = millis();

  if (sensorData.engineTemp.valid) {
    Serial.printf("DS18B20 engine temp: %.2f C\n", sensorData.engineTemp.temperatureC);
  } else {
    Serial.println("DS18B20 read failed or disconnected");
  }
}
