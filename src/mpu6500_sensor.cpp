#include "mpu6500_sensor.h"

#include <Wire.h>

namespace {
const uint8_t MPU6500_ADDRESS = 0x68;
const uint8_t MPU6500_REG_WHO_AM_I = 0x75;
const uint8_t MPU6500_REG_ACCEL_XOUT_H = 0x3B;
const uint8_t MPU6500_REG_GYRO_XOUT_H = 0x43;
const uint8_t MPU6500_REG_PWR_MGMT_1 = 0x6B;
const uint8_t MPU6500_REG_CONFIG = 0x1A;
const uint8_t MPU6500_REG_GYRO_CONFIG = 0x1B;
const uint8_t MPU6500_REG_ACCEL_CONFIG = 0x1C;
const uint8_t MPU6500_REG_ACCEL_CONFIG_2 = 0x1D;

bool mpu6500Present = false;

int16_t readInt16LE(uint8_t msbReg, uint8_t lsbReg) {
  uint8_t msb = 0;
  uint8_t lsb = 0;

  Wire.beginTransmission(MPU6500_ADDRESS);
  Wire.write(msbReg);
  Wire.endTransmission(false);
  Wire.requestFrom(static_cast<int>(MPU6500_ADDRESS), 1);
  if (Wire.available()) {
    msb = Wire.read();
  }

  Wire.beginTransmission(MPU6500_ADDRESS);
  Wire.write(lsbReg);
  Wire.endTransmission(false);
  Wire.requestFrom(static_cast<int>(MPU6500_ADDRESS), 1);
  if (Wire.available()) {
    lsb = Wire.read();
  }

  return static_cast<int16_t>((msb << 8) | lsb);
}

void configureAccelerometerAndGyro() {
  Wire.beginTransmission(MPU6500_ADDRESS);
  Wire.write(MPU6500_REG_PWR_MGMT_1);
  Wire.write(0x00);
  Wire.endTransmission();

  delay(10);

  Wire.beginTransmission(MPU6500_ADDRESS);
  Wire.write(MPU6500_REG_CONFIG);
  Wire.write(0x03);
  Wire.endTransmission();

  Wire.beginTransmission(MPU6500_ADDRESS);
  Wire.write(MPU6500_REG_GYRO_CONFIG);
  Wire.write(0x08);
  Wire.endTransmission();

  Wire.beginTransmission(MPU6500_ADDRESS);
  Wire.write(MPU6500_REG_ACCEL_CONFIG);
  Wire.write(0x10);
  Wire.endTransmission();

  Wire.beginTransmission(MPU6500_ADDRESS);
  Wire.write(MPU6500_REG_ACCEL_CONFIG_2);
  Wire.write(0x03);
  Wire.endTransmission();
}
}

void setupMPU6500() {
  Wire.beginTransmission(MPU6500_ADDRESS);
  if (Wire.endTransmission() != 0) {
    Serial.println("MPU6500 not detected on I2C bus");
    mpu6500Present = false;
    return;
  }

  delay(10);

  Wire.beginTransmission(MPU6500_ADDRESS);
  Wire.write(MPU6500_REG_WHO_AM_I);
  Wire.endTransmission(false);
  Wire.requestFrom(static_cast<int>(MPU6500_ADDRESS), 1);

  if (Wire.available()) {
    uint8_t whoami = Wire.read();
    if (whoami == 0x70 || whoami == 0x68) {
      configureAccelerometerAndGyro();
      mpu6500Present = true;
      Serial.println("MPU6500 sensor initialized");
      return;
    }
  }

  Serial.println("MPU6500 initialization failed");
  mpu6500Present = false;
}

MPU6500_DATA readMPU6500Values() {
  MPU6500_DATA data = MPU6500_DATA{};
  data.lastSensorReadingMillis = millis();

  if (!mpu6500Present) {
    data.valid = false;
    return data;
  }

  const float accelScale = 2048.0f;
  const float gyroScale = 16.4f;

  int16_t ax = readInt16LE(MPU6500_REG_ACCEL_XOUT_H, MPU6500_REG_ACCEL_XOUT_H + 1);
  int16_t ay = readInt16LE(MPU6500_REG_ACCEL_XOUT_H + 2, MPU6500_REG_ACCEL_XOUT_H + 3);
  int16_t az = readInt16LE(MPU6500_REG_ACCEL_XOUT_H + 4, MPU6500_REG_ACCEL_XOUT_H + 5);

  int16_t gx = readInt16LE(MPU6500_REG_GYRO_XOUT_H, MPU6500_REG_GYRO_XOUT_H + 1);
  int16_t gy = readInt16LE(MPU6500_REG_GYRO_XOUT_H + 2, MPU6500_REG_GYRO_XOUT_H + 3);
  int16_t gz = readInt16LE(MPU6500_REG_GYRO_XOUT_H + 4, MPU6500_REG_GYRO_XOUT_H + 5);

  data.accelX = static_cast<float>(ax) / accelScale;
  data.accelY = static_cast<float>(ay) / accelScale;
  data.accelZ = static_cast<float>(az) / accelScale;
  data.gyroX = static_cast<float>(gx) / gyroScale;
  data.gyroY = static_cast<float>(gy) / gyroScale;
  data.gyroZ = static_cast<float>(gz) / gyroScale;
  data.valid = true;

  Serial.printf("MPU6500 accel: %.3f %.3f %.3f g | gyro: %.2f %.2f %.2f deg/s\n",
                data.accelX,
                data.accelY,
                data.accelZ,
                data.gyroX,
                data.gyroY,
                data.gyroZ);

  return data;
}
