#include "data_out.h"

void initDataOut()
{
  Serial1.begin(115200, SERIAL_8N1, LementGatewayConfig::DATA_OUT_RX_PIN, LementGatewayConfig::DATA_OUT_TX_PIN);
  Serial.println("Data out interface initialized");
}

void writeDataOut(const SENSOR_DATA& sensorData, const String& timeStamp)
{
  if (!Serial1) {
    return;
  }

  Serial1.printf("{\"time\":\"%s\",\"bme_temperature\":%.2f,\"bme_pressure\":%.2f,\"bme_humidity\":%.2f,\"sht20_temperature\":%.2f,\"sht20_humidity\":%.2f,\"distance\":%.2f}\n",
                 timeStamp.c_str(),
                 sensorData.bme280.temperature,
                 sensorData.bme280.pressure,
                 sensorData.bme280.humidity,
                 sensorData.sht20.temperatureC,
                 sensorData.sht20.humidityPercent,
                 sensorData.rcwl.distanceCm);
}
