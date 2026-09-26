#include "i2c3_driver.h"
#include "tm4c123gh6pm.h"
#include "FreeRTOS.h"
#include "task.h"


#define I2C3_TIMEOUT_MS 50

void I2C3_Init(void) {
    // 1. Enable clock to I2C3 and GPIO Port D
    SYSCTL->RCGCI2C |= (1 << 3);   // Enable I2C3 clock
    SYSCTL->RCGCGPIO |= (1 << 3);  // Enable GPIO Port D clock
    while((SYSCTL->PRGPIO & (1 << 3)) == 0); // Wait for GPIO Port D ready

    // 2. Configure PD0 (SCL) and PD1 (SDA) for I2C3
    GPIOD->AFSEL |= (1 << 0) | (1 << 1);  // Enable alternate function
    GPIOD->PCTL &= ~0x000000FF;           // Clear PCTL bits for PD0 and PD1
    GPIOD->PCTL |= 0x00000033;            // Configure PD0 and PD1 for I2C3
    GPIOD->ODR |= (1 << 1);               // SDA (PD1) open drain
    GPIOD->DEN |= (1 << 0) | (1 << 1);    // Digital enable

    // 3. Initialize I2C3 master
    I2C3->MCR = 0x10;                     // Enable I2C master
    I2C3->MTPR = 0x07;                    // 100kHz @ 16MHz (TPR = (1/(2*100000) - 1)*16MHz = 7)

    // Small delay to ensure proper initialization
    vTaskDelay(pdMS_TO_TICKS(10));
}

int I2C3_Wr(uint8_t slave_addr, uint8_t reg, uint8_t data) {
    TickType_t xStartTime = xTaskGetTickCount();
    
    // 1. Send start condition
    I2C3->MSA = (slave_addr << 1);        // Slave address + write bit (0)
    I2C3->MCS = 0x7;                      // Start, Run, Stop
    
    // Wait until transmission completes or timeout
    while((I2C3->MCS & 1) && 
          (xTaskGetTickCount() - xStartTime < pdMS_TO_TICKS(I2C3_TIMEOUT_MS)));
    
    if(I2C3->MCS & 0x2) {                 // Check for error
        I2C3->MCS = 0x4;                  // Send stop if error occurred
        return -1;
    }

    // 2. Send register address
    I2C3->MDR = reg;
    I2C3->MCS = 0x3;                      // Run, Stop
    
    xStartTime = xTaskGetTickCount();
    while((I2C3->MCS & 1) && 
          (xTaskGetTickCount() - xStartTime < pdMS_TO_TICKS(I2C3_TIMEOUT_MS)));
    
    if(I2C3->MCS & 0x2) {
        I2C3->MCS = 0x4;
        return -1;
    }

    // 3. Send data
    I2C3->MDR = data;
    I2C3->MCS = 0x3;                      // Run, Stop
    
    xStartTime = xTaskGetTickCount();
    while((I2C3->MCS & 1) && 
          (xTaskGetTickCount() - xStartTime < pdMS_TO_TICKS(I2C3_TIMEOUT_MS)));
    
    if(I2C3->MCS & 0x2) {
        I2C3->MCS = 0x4;
        return -1;
    }

    return 0;
}

int I2C3_Rd(uint8_t slave_addr, uint8_t reg, uint8_t len, char *data) {
    TickType_t xStartTime = xTaskGetTickCount();
    
    // 1. Send register address first (write operation)
    I2C3->MSA = (slave_addr << 1);        // Slave address + write bit (0)
    I2C3->MCS = 0x7;                      // Start, Run, Stop
    
    while((I2C3->MCS & 1) && 
          (xTaskGetTickCount() - xStartTime < pdMS_TO_TICKS(I2C3_TIMEOUT_MS)));
    
    if(I2C3->MCS & 0x2) {
        I2C3->MCS = 0x4;
        return -1;
    }

    I2C3->MDR = reg;
    I2C3->MCS = 0x3;                      // Run, Stop
    
    xStartTime = xTaskGetTickCount();
    while((I2C3->MCS & 1) && 
          (xTaskGetTickCount() - xStartTime < pdMS_TO_TICKS(I2C3_TIMEOUT_MS)));
    
    if(I2C3->MCS & 0x2) {
        I2C3->MCS = 0x4;
        return -1;
    }

    // 2. Now read data
    I2C3->MSA = (slave_addr << 1) | 1;    // Slave address + read bit (1)
    
    if(len == 1) {
        I2C3->MCS = 0x7;                  // Start, Run, Stop (single byte)
    } else {
        I2C3->MCS = 0x3;                  // Start, Run (first byte)
    }
    
    uint8_t i;
    for(i = 0; i < len; i++) {
        xStartTime = xTaskGetTickCount();
        while((I2C3->MCS & 1) && 
              (xTaskGetTickCount() - xStartTime < pdMS_TO_TICKS(I2C3_TIMEOUT_MS)));
        
        if(I2C3->MCS & 0x2) {
            I2C3->MCS = 0x4;
            return -1;
        }
        
        data[i] = I2C3->MDR;
        
        if(i == len - 2) {
            I2C3->MCS = 0x5;              // Run, Stop (last byte)
        } else if(i < len - 1) {
            I2C3->MCS = 0x1;              // Run (continue)
        }
    }

    return 0;
}