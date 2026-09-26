#ifndef TASK_DISPLAY_H
#define TASK_DISPLAY_H

#include <stdint.h>
#include <stdbool.h>

// FreeRTOS includes
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

// Hardware register definitions
#include "tm4c123gh6pm.h"

// LCD control pin macros (connected to GPIOB)
#define LCD GPIOB
#define RS 0x01
#define RW 0x02
#define EN 0x04

// Public Task Function
void Task_Display(void *pvParameters);

// LCD Utility Functions
void LCD4bits_Init(void);
void LCD_Write4bits(unsigned char data, unsigned char control);
void LCD_WriteString(char *str);
void LCD4bits_Cmd(unsigned char cmd);
void LCD4bits_Data(unsigned char data);

// Delay Functions
void delayMs(int n);
void delayUs(int n);

#endif // TASK_DISPLAY_H
