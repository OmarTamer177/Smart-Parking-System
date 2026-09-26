#include "tm4c123gh6pm.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

// Externals
extern QueueHandle_t xSpeedQueue;
extern SemaphoreHandle_t xLcdMutex;
extern SemaphoreHandle_t xBuzzerMutex;
extern QueueHandle_t xIgnitionEventQueue;

extern QueueHandle_t xLockQueue;
extern QueueHandle_t xDoorQueue;

// Definitions
#define SPEED_THRESHOLD_KPH     100

volatile bool doorClosed;
volatile bool doorLocked;

// Function prototypes
void GPIO_Init_DoorSystem(void);

// Initialize GPIOs
void GPIO_Init_DoorSystem(void) {
    SYSCTL_RCGCGPIO_R |= 0x33; // Enable clocks for GPIOA, GPIOB, GPIOE, GPIOF
    while ((SYSCTL_PRGPIO_R & 0x33) != 0x33); // Wait for them to be ready

    // PE2 = Lock Btn, PE3 = Unlock Btn, PE4 = Ignition, PE5 = Door Sensor
    GPIO_PORTE_DIR_R &= ~(0x3C);   // Inputs (PE2 to PE5)
    GPIO_PORTE_DEN_R |= 0x3C;
    GPIO_PORTE_PUR_R |= 0x3C;

    // PB4 = Door Lock Control, PB5 = Buzzer
    GPIO_PORTB_DIR_R |= (1 << 4) | (1 << 5);   // Outputs
    GPIO_PORTB_DEN_R |= (1 << 4) | (1 << 5);   // Enable digital
    GPIO_PORTB_DATA_R &= ~((1 << 4) | (1 << 5)); // Initially off

    // PF1 can be used for debug/LED if needed
    GPIO_PORTF_DIR_R |= (1 << 1) | (1 << 2);      // PF1 &PF2 as output (optional)
    GPIO_PORTF_DEN_R |= (1 << 1) | (1 << 2);
    GPIO_PORTF_DATA_R &= ~((1 << 1) | (1 << 2));    // Initially off
}

// Lock/unlock functions (moved to Port B, PB4)
void LockDoors(void) {
    GPIO_PORTF_DATA_R |= (1 << 1);  // Set PB4 to lock doors
}
void UnlockDoors(void) {
    GPIO_PORTF_DATA_R &= ~(1 << 1); // Clear PB4 to unlock doors
}

// Buzzer control (PB5)
void Buzzer_On(void) {
    GPIO_PORTB_DATA_R |= (1 << 5);     // Set PB5
}
void Buzzer_Off(void) {
    GPIO_PORTB_DATA_R &= ~(1 << 5);    // Clear PB5
}

// Door Control Task
void Task_DoorControl(void *pvParameters) {
    GPIO_Init_DoorSystem();

    uint16_t speed = 0;
    doorLocked = false;
    bool ignition;

    while (1) {
        // Manual controls
        bool lockBtn = !(GPIO_PORTE_DATA_R & (1 << 2)); 
        //bool unlockBtn = !(GPIO_PORTE_DATA_R & (1 << 3)); // Optional unlock button
        doorClosed = (GPIO_PORTE_DATA_R & (1 << 5));

        // Read latest speed and ignition state from queues
        xQueuePeek(xSpeedQueue, &speed, portMAX_DELAY);
        xQueuePeek(xIgnitionEventQueue, &ignition, portMAX_DELAY);
				
				xQueueOverwrite(xLockQueue, &doorLocked);
				xQueueOverwrite(xDoorQueue, &doorClosed);
        // Manual override (lock if button pressed and ignition on)
        if (lockBtn && !ignition) {
            LockDoors();
            doorLocked = true;
        } else {
            UnlockDoors();
            doorLocked = false;
        }

        // Buzzer alert if door is open and speed > threshold with ignition
        if (speed > SPEED_THRESHOLD_KPH && !doorClosed && !ignition) {
            if (xSemaphoreTake(xBuzzerMutex, portMAX_DELAY)) {
                Buzzer_On();
                vTaskDelay(pdMS_TO_TICKS(100));
                Buzzer_Off();
                vTaskDelay(pdMS_TO_TICKS(100));
                xSemaphoreGive(xBuzzerMutex);
            }
        }
        // Auto-lock if speed exceeds threshold and door not already locked
        else if (speed > SPEED_THRESHOLD_KPH && !doorLocked && !ignition) {
            LockDoors();
            doorLocked = true;
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
