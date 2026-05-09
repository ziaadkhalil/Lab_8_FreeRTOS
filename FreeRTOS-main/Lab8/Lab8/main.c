#define F_CPU 8000000UL

#include <avr/io.h>
#include "FreeRTOS.h"
#include "task.h"
#include "uart.h"
#include "led.h"

/* ---------------- TASK 1: LED ---------------- */
void vTask_LED(void *pv)
{
	while (1)
	{
		LED_toggle();
		vTaskDelay(pdMS_TO_TICKS(500));
	}
}

/* ---------------- TASK 2: Hello World ---------------- */
void vTask_Hello(void *pv)
{
	while (1)
	{
		UART_sendString("Hello World\r\n");
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

/* ---------------- MAIN ---------------- */
int main(void)
{
	UART_init(9600);
	LED_init();

	xTaskCreate(vTask_LED, "LED", 80, NULL, 1, NULL);
	xTaskCreate(vTask_Hello, "SYS", 80, NULL, 1, NULL);

	vTaskStartScheduler();

	while (1);
}