#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"


typedef enum {
    GEAR_PARK,
    GEAR_DRIVE,
    GEAR_REVERSE
} GearState_t;



extern SemaphoreHandle_t xBuzzerMutex;
volatile GearState_t currentGear;
extern volatile bool systemRunning;
extern QueueHandle_t xIgnitionEventQueue;


void Task_SystemControl(void *pvParameters);
void GearControl_Init(void);

void Task_SystemControl(void *pvParameters) {
    uint8_t lastIgnitionState = 0xFF;

    GearControl_Init();
    while (1) {
        // Read gear switch inputs: PA2, PA3, PA5
				// Read Ignition: PE4 
        uint8_t portAData = GPIO_PORTA_DATA_R;
				uint8_t portEData = GPIO_PORTE_DATA_R;
        bool gearPark    = portAData & (1 << 2);  // PA2
        bool gearDrive   = portAData & (1 << 3);  // PA3
        bool gearReverse = portAData & (1 << 5);  // PA5
				bool ignition = portEData & (1 << 4);  // PE4

			
        // Determine gear state
				if (!ignition){
					if (!gearPark)    currentGear = GEAR_PARK;
					else if (!gearDrive)   currentGear = GEAR_DRIVE;
					else if (!gearReverse) currentGear = GEAR_REVERSE;
				}
				xQueueOverwrite(xIgnitionEventQueue, &ignition);

        //lastIgnitionState = ignition;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}


void GearControl_Init(void) {
    SYSCTL_RCGCGPIO_R |= (1 << 0) | (1 << 4); // Enable clocks for Port A and Port E
    while ((SYSCTL_PRGPIO_R & ((1 << 0) | (1 << 4))) != ((1 << 0) | (1 << 4)));

    // PA2, PA3, PA4 = Gear Inputs (P, D, R), PA5 = Ignition
    GPIO_PORTA_DIR_R &= ~(0x3C);   // Bits 2 to 5 as inputs
    GPIO_PORTA_DEN_R |= 0x3C;      // Enable digital function
    GPIO_PORTA_PUR_R |= 0x3C;      // Enable pull-up resistors (optional but recommended)
	
	  GPIO_PORTE_DIR_R &= ~(1 << 4); // Input
    GPIO_PORTE_DEN_R |= (1 << 4);
}


