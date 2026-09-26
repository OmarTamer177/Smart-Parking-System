#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "FreeRTOS.h"
#include "task.h"
#include "Task_SystemControl.h"


void Task_ReverseAssist_init(void);
void Task_ReverseAssist(void *pvParameters);
uint32_t Measure_distance(void);
void Delay_MicroSecond(uint32_t time);

volatile uint32_t distance = 0;
extern SemaphoreHandle_t xBuzzerMutex;
extern QueueHandle_t xDistanceQueue;


void Task_ReverseAssist(void *pvParameters) {

    Task_ReverseAssist_init();

    while (1) {
			bool ignition;
		  xQueuePeek(xIgnitionEventQueue, &ignition, portMAX_DELAY);
			
		  if (currentGear == GEAR_REVERSE && (!ignition)) {
        distance = Measure_distance();
				
				xQueueOverwrite(xDistanceQueue, &distance);
        if (distance > 30) {
            GPIOB->DATA = (1 << 1);        // Green LED
						if (xSemaphoreTake(xBuzzerMutex, 0)){
							GPIOB->DATA &= ~(1 << 5);      // Buzzer OFF
							xSemaphoreGive(xBuzzerMutex);
						}
        } else if (distance > 10) {
            GPIOB->DATA = (1 << 1);     // Blue LED
						GPIOB->DATA |= (1 << 0); 
						if (xSemaphoreTake(xBuzzerMutex, 0)){
							GPIOB->DATA |= (1 << 5);       // Buzzer ON
							vTaskDelay(pdMS_TO_TICKS(300));
							GPIOB->DATA &= ~(1 << 5);      // Buzzer OFF
							vTaskDelay(pdMS_TO_TICKS(300));
							xSemaphoreGive(xBuzzerMutex);
						}

        } else {
            GPIOB->DATA = (1 << 0);        // Red LED
						if (xSemaphoreTake(xBuzzerMutex, 0)){
							GPIOB->DATA |= (1 << 5);       // Buzzer ON
							vTaskDelay(pdMS_TO_TICKS(100));
							GPIOB->DATA &= ~(1 << 5);      // Buzzer OFF
							vTaskDelay(pdMS_TO_TICKS(100));
							xSemaphoreGive(xBuzzerMutex);
						}
         }
			 }
			 else{
						GPIOF->DATA &= ~((1 << 1)|(1 << 2)|(1 << 3)); 
						vTaskDelay(pdMS_TO_TICKS(100));
				}
	
	}
}

void Task_ReverseAssist_init(void) {
    SYSCTL->RCGCTIMER |= (1 << 0);  // Enable Timer0
    SYSCTL->RCGCGPIO |= (1 << 1);   // Enable GPIOB
    SYSCTL->RCGCGPIO |= (1 << 0);   // Enable GPIOA (for trigger)
    SYSCTL->RCGCGPIO |= (1 << 5);   // Enable GPIOF (for RGB)

    // PA4 as output for Trigger
    GPIOA->DIR |= (1 << 4);
    GPIOA->DEN |= (1 << 4);

    // PB6 as T0CCP0 input
    GPIOB->DIR &= ~(1 << 6);
    GPIOB->DEN |= (1 << 6);
    GPIOB->AFSEL |= (1 << 6);
    GPIOB->PCTL &= ~0x0F000000;
    GPIOB->PCTL |= 0x07000000;

    // PB0, PB1, PB2 as output (R, G, B)
    GPIOB->DIR |= (1 << 0) | (1 << 1) | (1 << 2);
    GPIOB->DEN |= (1 << 0) | (1 << 1) | (1 << 2);

    // PB5 as output for buzzer
    GPIOB->DIR |= (1 << 5);
    GPIOB->DEN |= (1 << 5);

    // Timer0A in edge time mode
    TIMER0->CTL &= ~(1 << 0);
    TIMER0->CFG = 0x04;
    TIMER0->TAMR = 0x17;
    TIMER0->CTL |= (1 << 2) | (1 << 3); // both edges
    TIMER0->CTL |= (1 << 0);
}

uint32_t Measure_distance(void) {
    uint32_t lastEdge, thisEdge, pulseWidth;

    // Trigger Ultrasonic Pulse
    GPIOA->DATA &= ~(1 << 4);
    Delay_MicroSecond(2);
    GPIOA->DATA |= (1 << 4);
    Delay_MicroSecond(10);
    GPIOA->DATA &= ~(1 << 4);

    // Capture pulse width
    TIMER0->ICR = 4; // clear capture flag
    while ((TIMER0->RIS & 4) == 0); // wait for rising edge block until a rising edge from the echo output
    lastEdge = TIMER0->TAR; //Records the timer value at the moment the ECHO pin went HIGH. This is the start of the echo pulse.

    TIMER0->ICR = 4;
    while ((TIMER0->RIS & 4) == 0);
    thisEdge = TIMER0->TAR; //Timer value when ECHO went LOW. This marks the end of the echo pulse.

    pulseWidth = thisEdge - lastEdge;
    return (pulseWidth * 34) / 100000;  // Distance calculation in cm
}

void Delay_MicroSecond(uint32_t time) {
    SYSCTL->RCGCTIMER |= (1 << 1);   // Enable Timer1
    TIMER1->CTL = 0;
    TIMER1->CFG = 0x04;
    TIMER1->TAMR = 0x02;
    TIMER1->TAILR = 16 - 1;  // 1us for 16MHz
    TIMER1->ICR = 1;
    TIMER1->CTL |= 1;

    for (uint32_t i = 0; i < time; i++) {
        while ((TIMER1->RIS & 1) == 0);
        TIMER1->ICR = 1;
    }
}