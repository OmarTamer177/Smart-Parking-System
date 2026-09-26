#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "tm4c123gh6pm.h"
#include "Task_SystemControl.h"

// LCD Definitions for PORTD
#define LCD GPIOB
#define RS  0x01  // PB0
#define EN  0x02  // PB1

// externals
extern QueueHandle_t xSpeedQueue;
extern QueueHandle_t xDistanceQueue;
extern QueueHandle_t xIgnitionEventQueue;
extern QueueHandle_t xLockQueue;
extern QueueHandle_t xDoorQueue;
extern SemaphoreHandle_t xLcdMutex;

// Function Prototypes
void LCD4bits_Init(void);
void LCD_Write4bits(unsigned char, unsigned char);
void LCD_WriteString(char *);
void LCD4bits_Cmd(unsigned char);
void LCD4bits_Data(unsigned char);
void LCD_DisplayLine1(char *);
void LCD_DisplayLine2(char *);
void delayMs(int);
void delayUs(int);

void LED_Init(void);
void LED_Blink(uint8_t times);

// FreeRTOS
extern SemaphoreHandle_t lcdMutex;

// ========================= LCD Functions =========================

void LCD4bits_Init(void)
{
       SYSCTL->RCGCGPIO |= (1 << 1);  // Enable clock for PORTB
    while ((SYSCTL->PRGPIO & (1 << 1)) == 0);
    delayMs(10);

    // Set PB0–PB7 as outputs except PB6
    LCD->DIR = 0xFF & ~(1 << 6);   // PB6 is excluded
    LCD->DEN = 0xFF & ~(1 << 6);   // Digital enable, PB6 excluded

    LCD4bits_Cmd(0x28); // 4-bit mode, 2-line
    LCD4bits_Cmd(0x06); // Increment cursor
    LCD4bits_Cmd(0x01); // Clear display
    LCD4bits_Cmd(0x0F); // Display on, cursor on
}

void LCD_Write4bits(unsigned char data, unsigned char control)
{
    data &= 0xF0;       // Upper nibble only
    control &= 0x0F;    // RS, EN only
    LCD->DATA = data | control;
    LCD->DATA = data | control | EN;
    delayUs(1);
    LCD->DATA = data | control;
    LCD->DATA = 0;
}

void LCD4bits_Cmd(unsigned char cmd)
{
    LCD_Write4bits(cmd & 0xF0, 0);      // Upper nibble
    LCD_Write4bits(cmd << 4, 0);        // Lower nibble

    if (cmd < 4) delayMs(2);
    else delayUs(40);
}

void LCD4bits_Data(unsigned char data)
{
    LCD_Write4bits(data & 0xF0, RS);     // Upper nibble + RS
    LCD_Write4bits(data << 4, RS);       // Lower nibble + RS
    delayUs(40);
}

void LCD_WriteString(char *str)
{
    int i = 0;
    while (str[i] != '\0')
    {
        LCD4bits_Data(str[i]);
        i++;
    }
}

void LCD_DisplayLine1(char *msg)
{
    LCD4bits_Cmd(0x80);     // Set cursor to beginning of line 1
    LCD_WriteString(msg);
}

void LCD_DisplayLine2(char *msg)
{
		LCD4bits_Cmd(0xC0);     // Set cursor to beginning of line 2
		LCD_WriteString(msg);	
}

void Task_Display(void *pvParameters){
    LCD4bits_Init();
    uint16_t speed = 0;
    uint32_t distance = 0;
    bool ignition = false;
    bool lock = false;
    bool door = false;
    
    while (1){
        xQueuePeek(xSpeedQueue, &speed, 0);
        xQueuePeek(xDistanceQueue, &distance, 0);
        xQueuePeek(xIgnitionEventQueue, &ignition, 0);
        xQueuePeek(xLockQueue, &lock, 0);
        xQueuePeek(xDoorQueue, &door, 0);
        
        // Take the LCD mutex before accessing the LCD
        if (xSemaphoreTake(xLcdMutex, portMAX_DELAY) == pdTRUE) {
            // Adjust Line 1 for lock(locked/unlocked) and door(open/closed)
            char msg1[16] = "";
            
            if(lock){
                if(door){
                    sprintf(msg1, "LOCKED CLOSED");
                }
                else{
                    sprintf(msg1, "LOCKED OPENED");
                }
            }
            else{
                if(door){
                    sprintf(msg1, "UNLOCKED CLOSED");
                }
                else{
                    sprintf(msg1, "UNLOCKED OPENED");
                }
            }
            LCD_DisplayLine1(msg1);
            
            if(currentGear == GEAR_DRIVE){
                char msg2[16] = "";
                sprintf(msg2, "Gear:D S:%dkm/h", speed);
                LCD_DisplayLine2(msg2);
            }
            else if (currentGear == GEAR_PARK){
                char msg2[16] = "";
                if (ignition)
                    sprintf(msg2, "Gear:P Ign:ON");
                else
                    sprintf(msg2, "Gear:P Ign:OFF");
                LCD_DisplayLine2(msg2);
            }
            else{
                char msg2[16] = "";
                sprintf(msg2, "Gear:R D:%dm", distance);
                LCD_DisplayLine2(msg2);
            }
            
            // Release the mutex when done
            xSemaphoreGive(xLcdMutex);
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// ========================= Delay Functions =========================

void delayMs(int n)
{
    volatile int i, j;
    for (i = 0; i < n; i++)
        for (j = 0; j < 3180; j++);
}

void delayUs(int n)
{
    volatile int i, j;
    for (i = 0; i < n; i++)
        for (j = 0; j < 3; j++);
}


void LED_Init(void) {
    SYSCTL->RCGCGPIO |= 0x20;       // Enable clock to Port F
    GPIOF->DIR |= 0x02;             // PF1 output
    GPIOF->DEN |= 0x02;             // Digital enable PF1
}

void LED_Blink(uint8_t times) {
    for (uint8_t i = 0; i < times; i++) {
        GPIOF->DATA ^= 0x02;        // Toggle PF1
        vTaskDelay(pdMS_TO_TICKS(200));
        GPIOF->DATA ^= 0x02;
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}