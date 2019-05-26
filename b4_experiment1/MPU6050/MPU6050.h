#ifndef MPU6050_H_
#define MPU6050_H_

class MPU6050 {
private:
  static const uint8_t address = 0x68;
  int16_t ax, ay, az;
  int16_t gx, gy, gz;
  float temp;
  const float a_scaleFacter = 4096.0f;
  const float g_scaleFacter = 65.5f;

public: 
  MPU6050();
  void init();
  void getRawData(int16_t* data);
  void getData(float* data);
  void readData();
  void i2c_writeReg(uint8_t addr, uint8_t reg, uint8_t val);
};

#endif

