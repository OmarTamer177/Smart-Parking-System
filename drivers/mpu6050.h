#ifndef MPU6050_H
#define MPU6050_H

#include <stdint.h>

void MPU6050_Init(void);
void MPU6050_ReadData(int16_t *accX, int16_t *accY, int16_t *accZ,
                      int16_t *temp, int16_t *gyroX, int16_t *gyroY, int16_t *gyroZ);

#endif
