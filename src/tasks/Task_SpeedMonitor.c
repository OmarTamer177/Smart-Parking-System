// Includes
#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "tm4c123gh6pm.h"
#include "Task_SystemControl.h"

// External queue
extern QueueHandle_t xSpeedQueue;
volatile uint16_t speedKph = 0;

// Constants
#define POTENTIOMETER_ADC_CHANNEL 0  // AIN0 = PE3
#define SPEED_MAX_KPH 180            // Max speed assumed for potentiometer

// Function Prototypes
void ADC0_Init(void);
uint16_t ADC0_Read(void);

// ADC0 Initialization for AIN0 (PE3)
void ADC0_Init(void) {
    SYSCTL_RCGCADC_R |= 0x01;       // Enable ADC0 clock
    SYSCTL_RCGCGPIO_R |= 0x10;      // Enable clock for PORTE
    while ((SYSCTL_PRGPIO_R & 0x10) == 0) {}

    GPIO_PORTE_AFSEL_R |= 0x08;     // Enable alternate function on PE3
    GPIO_PORTE_DEN_R &= ~0x08;      // Disable digital I/O on PE3
    GPIO_PORTE_AMSEL_R |= 0x08;     // Enable analog function on PE3

    ADC0_ACTSS_R &= ~0x08;          // Disable SS3 during config
    ADC0_EMUX_R &= ~0xF000;         // Software trigger
    ADC0_SSMUX3_R = POTENTIOMETER_ADC_CHANNEL;  // AIN0 (PE3)
    ADC0_SSCTL3_R = 0x06;           // IE0 and END0
    ADC0_ACTSS_R |= 0x08;           // Enable SS3
}

// Read ADC value from AIN0
uint16_t ADC0_Read(void) {
    ADC0_PSSI_R = 0x08;             // Start SS3 conversion
    while ((ADC0_RIS_R & 0x08) == 0) {}
    uint16_t result = ADC0_SSFIFO3_R & 0xFFF;
    ADC0_ISC_R = 0x08;              // Clear completion flag
    return result;
}

// Task to monitor speed via potentiometer
void Task_SpeedMonitor(void *pvParameters) {
	
    ADC0_Init();
    uint16_t adcValue = 0;
    

    while (1) {
			bool ignition;
		  xQueuePeek(xIgnitionEventQueue, &ignition, 0);
			if (currentGear == GEAR_DRIVE && (!ignition)) {
        adcValue = ADC0_Read();  // Read ADC value (0 - 4095)

        // Map to speed in km/h
        speedKph = ((adcValue * SPEED_MAX_KPH) / 4095) - 37;
				
        // Send to queue (non-blocking)
        xQueueOverwrite(xSpeedQueue, &speedKph);
			}
			     vTaskDelay(pdMS_TO_TICKS(200));  // 200ms delay
	}
}
	
