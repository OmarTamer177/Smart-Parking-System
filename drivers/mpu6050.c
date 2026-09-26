#include "mpu6050.h"
#include "i2c.h"

#define MPU6050_ADDR 0x68

void MPU6050_Init(void)
{
    I2C3_Write(MPU6050_ADDR, 0x6B, 0x00); // Wake up MPU6050
    I2C3_Write(MPU6050_ADDR, 0x1A, 0x00); // Set DLPF
    I2C3_Write(MPU6050_ADDR, 0x1B, 0x18); // Set gyro range to ±2000°/s
    I2C3_Write(MPU6050_ADDR, 0x1C, 0x00); // Set accel range to ±2g
}

void MPU6050_ReadData(int16_t *accX, int16_t *accY, int16_t *accZ,
                      int16_t *temp, int16_t *gyroX, int16_t *gyroY, int16_t *gyroZ)
{
    uint8_t data[14];
    I2C3_Read(MPU6050_ADDR, 0x3B, data, 14);

    *accX = (data[0] << 8) | data[1];
    *accY = (data[2] << 8) | data[3];
    *accZ = (data[4] << 8) | data[5];
    *temp = (data[6] << 8) | data[7];
    *gyroX = (data[8] << 8) | data[9];
    *gyroY = (data[10] << 8) | data[11];
    *gyroZ = (data[12] << 8) | data[13];
}
