#include <Arduino.h>
#include "MPU6050.h"
#include <Wire.h>

MPU6050::MPU6050() {

}

void MPU6050::init() {
  i2c_writeReg(address, 0x6B, 0x80);
  delay(50);
  i2c_writeReg(address, 0x6B, 0x03); 
  i2c_writeReg(address, 0x1B, 0x08); //setting gyro
  i2c_writeReg(address, 0x1C, 0x10); //setting accel
  i2c_writeReg(address, 0x1A, 0x03); //LPFsetting
}

void MPU6050::getRawData(int16_t* data) {
  readData();
  data[0] = gx;
  data[1] = gy;
  data[2] = gz;
  data[3] = ax;
  data[4] = ay;
  data[5] = az;
}

void MPU6050::getData(float* data) {
  readData();
  data[0] = gx / g_scaleFacter;
  data[1] = gy / g_scaleFacter;
  data[2] = gz / g_scaleFacter;
  data[3] = ax / a_scaleFacter;
  data[4] = ay / a_scaleFacter;
  data[5] = az / a_scaleFacter;
}

void MPU6050::readData() {
  Wire.beginTransmission(address);
  Wire.write(0x3B);
  Wire.endTransmission();
  Wire.requestFrom(address, 14);

  while (Wire.available() < 14);
  ax = Wire.read() << 8 | Wire.read();
  ay = Wire.read() << 8 | Wire.read();
  az = Wire.read() << 8 | Wire.read();
  temp = Wire.read() << 8 | Wire.read();
  gx = Wire.read() << 8 | Wire.read();
  gy = Wire.read() << 8 | Wire.read();
  gz = Wire.read() << 8 | Wire.read();
}

void MPU6050::i2c_writeReg(uint8_t addr, uint8_t reg, uint8_t val) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

