#ifndef I2C3_DRIVER_H
#define I2C3_DRIVER_H

#include <stdint.h>

void I2C3_Init(void);
int I2C3_Wr(uint8_t slave_addr, uint8_t reg, uint8_t data);
int I2C3_Rd(uint8_t slave_addr, uint8_t reg, uint8_t len, char *data);

#endif // I2C3_DRIVER_H