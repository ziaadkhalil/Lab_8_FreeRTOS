/*
 * ============================================================
 *  LAB 8 - PROJECT 2: Mutex Demo — VERSION 1: THE PROBLEM
 * ============================================================
 *
 *  Two tasks both print to UART with NO protection.
 *  The scheduler can switch between them mid-print,
 *  causing characters from both tasks to mix together.
 *
 *  EXPECTED TERMINAL OUTPUT (garbled):
 *  =====================================
 *  === WITHOUT Mutex ===
 *  [Task1] Hell[Task2] Hello from Task2!
 *  o from Task1!
 *  [Task2] Hello [Task1] Hello from Task1!
 *  from Task2!
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
#include "uart.h"

/* -------------------------------------------------------
 *  TASK 1 — prints every 400 ms
 * ------------------------------------------------------- */
void vTask1(void *pv)
{
    while(1)
    {
        UART_sendString("[Task1] Hello from Task1!\r\n");

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/* -------------------------------------------------------
 *  TASK 2 — prints every 250 ms
 * ------------------------------------------------------- */
void vTask2(void *pv)
{
    for (;;)
    {
        UART_sendString("[Task2] Hello from Task2!\r\n");
        vTaskDelay(pdMS_TO_TICKS(450));
    }
}


int main(void)
{
    UART_init(9600);

    UART_sendString("\r\n=== WITHOUT Mutex :( ===\r\n\r\n");

    xTaskCreate(vTask1, "T1", 80, NULL, 1, NULL);
    xTaskCreate(vTask2, "T2", 80, NULL, 1, NULL);

    vTaskStartScheduler();
    while (1);
}
