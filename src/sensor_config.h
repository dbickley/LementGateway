#ifndef SENSOR_CONFIG_H
#define SENSOR_CONFIG_H

namespace LementGatewayConfig {
  constexpr int PRIMARY_RCWL_TRIGGER_PIN = 47;
  constexpr int PRIMARY_RCWL_ECHO_PIN = 1;

  constexpr int BACKUP_SENSOR_COUNT = 5;
  constexpr int BACKUP_RCWL_TRIGGER_PINS[BACKUP_SENSOR_COUNT] = {4, 5, 18, 8, 9};
  constexpr int BACKUP_RCWL_ECHO_PINS[BACKUP_SENSOR_COUNT] = {3, 10, 11, 12, 13};

  constexpr int DS18B20_DATA_PIN = 16;
  constexpr int SHT20_SDA_PIN = 6;
  constexpr int SHT20_SCL_PIN = 7;
  constexpr int RDA5807M_I2C_ADDRESS = 0x60;
  constexpr int AMPLIFIER_ENABLE_PIN = 17;

  constexpr int OBD2_RX_PIN = 14;
  constexpr int OBD2_TX_PIN = 15;
  constexpr int DATA_OUT_RX_PIN = 38;
  constexpr int DATA_OUT_TX_PIN = 21;
}

#endif
