/*
 * ============================================================
 *  LAB 8 - PROJECT 2: Mutex Demo — VERSION 2: THE SOLUTION
 * ============================================================
 *
 *  Same two tasks, but now protected by a mutex.
 *  Only one task can print at a time — the other BLOCKS
 *  until the mutex is released.
 *
 *  EXPECTED TERMINAL OUTPUT (clean):
 *  =====================================
 *  === WITH Mutex :) ===
 *  [Task1] Hello from Task1!
 *  [Task2] Hello from Task2!
 *  [Task2] Hello from Task2!
 *  [Task1] Hello from Task1!
 *  [Task2] Hello from Task2!
 *  =====================================
 *
 *  HARDWARE:
 *    UART TX → PE1 (pin 3)
 *    UART RX → PE0 (pin 2)
 * ============================================================
 */

#define F_CPU 8000000UL
#include <avr/io.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"    
#include "uart.h"

/* -------------------------------------------------------
 *  One global mutex — guards the shared UART resource.
 *  Any task that wants to print MUST take this first.
 * ------------------------------------------------------- */
SemaphoreHandle_t xUART_Mutex;

/* -------------------------------------------------------
 *  TASK 1 — prints every 400 ms
 * ------------------------------------------------------- */
void vTask1(void *pv)
{
    for (;;)
    {
        /*
         *  STEP 1: Take the mutex
         *  portMAX_DELAY = wait forever until it's free
         *  Task2 will BLOCK here if Task1 is printing
         */
        if (xSemaphoreTake(xUART_Mutex, portMAX_DELAY) == pdTRUE)
        {
            /*
             *  STEP 2: Safe to use UART now
             *  Only this task is here — no other task can
             *  enter this block until we release the mutex
             */
            UART_sendString("[Task1] Hello from Task1!\r\n");

            /*
             *  STEP 3: Give the mutex back
             *  This releases the UART for Task2 to use.
             *  NEVER forget this — forgetting causes deadlock!
             */
            xSemaphoreGive(xUART_Mutex);
        }

        vTaskDelay(pdMS_TO_TICKS(400));
    }
}

/* -------------------------------------------------------
 *  TASK 2 — prints every 250 ms
 * ------------------------------------------------------- */
void vTask2(void *pv)
{
    for (;;)
    {
        /* Same pattern: Take → Use → Give */
        if (xSemaphoreTake(xUART_Mutex, portMAX_DELAY) == pdTRUE)
        {
            UART_sendString("[Task2] Hello from Task2!\r\n");
            xSemaphoreGive(xUART_Mutex);
        }

        vTaskDelay(pdMS_TO_TICKS(400));
    }
}

/* -------------------------------------------------------
 *  MAIN
 * ------------------------------------------------------- */
int main(void)
{
    UART_init(9600);

    UART_sendString("\r\n=== WITH Mutex :) ===\r\n\r\n");

    /*
     *  Create the mutex BEFORE creating tasks.
     *  It starts in "given" state — available immediately.
     *  First task to call Take() will get it.
     */
    xUART_Mutex = xSemaphoreCreateMutex();
	/* Add this check! */
	if (xUART_Mutex == NULL)
	{
		UART_sendString("ERROR: Mutex failed!\r\n");
		while(1);   /* stop here so we know */
	}
	
    xTaskCreate(vTask1, "T1", 80, NULL, 2, NULL);
    xTaskCreate(vTask2, "T2", 80, NULL, 1, NULL);

    vTaskStartScheduler();
    while (1);
}
