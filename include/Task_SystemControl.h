#ifndef SYSTEM_CONTROL_H
#define SYSTEM_CONTROL_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"

// Gear states
typedef enum {
    GEAR_PARK,
    GEAR_DRIVE,
    GEAR_REVERSE
} GearState_t;

// Global variables
extern volatile GearState_t currentGear;
extern volatile bool systemRunning;
extern QueueHandle_t xIgnitionEventQueue;

// Function declarations
void Task_SystemControl(void *pvParameters);
void GearControl_Init(void);

#endif // SYSTEM_CONTROL_H
