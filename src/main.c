/* main.c - Automotive Smart Safety System with FreeRTOS + Bare-metal + Reverse Assist task */

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "Task_SystemControl.h"

// Task prototypes (assume all are implemented)
void Task_SpeedMonitor(void *pvParameters);
void Task_DoorControl(void *pvParameters);
void Task_Display(void *pvParameters);
void Task_ReverseAssist(void *pvParameters);

// RTOS shared resources
QueueHandle_t xSpeedQueue;
QueueHandle_t xDistanceQueue;
QueueHandle_t xIgnitionEventQueue;
QueueHandle_t xLockQueue;
QueueHandle_t xDoorQueue;

SemaphoreHandle_t xLcdMutex;
SemaphoreHandle_t xBuzzerMutex;


int main(void) {
    xSpeedQueue = xQueueCreate(1, sizeof(uint16_t));
    xDistanceQueue = xQueueCreate(1, sizeof(uint32_t));
    xIgnitionEventQueue = xQueueCreate(1, sizeof(bool));
		xLockQueue = xQueueCreate(1, sizeof(bool));
		xDoorQueue = xQueueCreate(1, sizeof(bool));
	
	  xLcdMutex = xSemaphoreCreateMutex();
		xBuzzerMutex = xSemaphoreCreateMutex();
	
	
		xTaskCreate(Task_SystemControl, "SystemControl", 128, NULL, 2, NULL); 		   
    xTaskCreate(Task_SpeedMonitor, "SpeedMonitor", 128, NULL, 2, NULL);
    xTaskCreate(Task_DoorControl, "DoorControl", 128, NULL, 2, NULL);
    //xTaskCreate(Task_Display, "Display", 128, NULL, 2, NULL);
		xTaskCreate(Task_ReverseAssist, "ReverseAssist", 256, NULL, 2, NULL);

    vTaskStartScheduler();

    while (1) {}
}