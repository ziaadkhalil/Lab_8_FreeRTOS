/*
 * ============================================================
 *  LAB 8 - Queue Demo on ATmega169PA
 * ============================================================
 *
 *  CONCEPT:
 *  Task1 (Producer) → increments a counter → sends to queue
 *  Task2 (Consumer) → receives from queue  → prints over UART
 *
 *  EXPECTED TERMINAL OUTPUT:
 *  ================================
 *  === Queue Demo ===
 *  [Consumer] Received: 1
 *  [Consumer] Received: 2
 *  [Consumer] Received: 3
 *  [Consumer] Received: 4
 *  ================================
 *  HARDWARE:
 *    UART TX → PE1 (pin 3)
 *    UART RX → PE0 (pin 2)
 * ============================================================
 */

#define F_CPU 8000000UL
#include <avr/io.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"     /* needed for queue functions */
#include "uart.h"

/* -------------------------------------------------------
 *  Queue handle
 *  Holds up to 5 uint8_t values (our counter)
 *  If full → producer blocks until consumer reads one out
 * ------------------------------------------------------- */
QueueHandle_t xCounterQueue;

/* -------------------------------------------------------
 *  TASK 1 — Producer
 *
 *  Increments a counter every 500 ms and puts it
 *  into the queue. Does NOT print anything itself —
 *  it just produces data and moves on.
 * ------------------------------------------------------- */
void vTask_Producer(void *pv)
{
    uint8_t counter = 0;

    for (;;)
    {
        counter++;

        /*
         *  xQueueSend(queue, &data, timeout)
         *  Puts a COPY of counter into the queue.
         *  portMAX_DELAY = wait forever if queue is full.
         */
        xQueueSend(xCounterQueue, &counter, portMAX_DELAY);

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/* -------------------------------------------------------
 *  TASK 2 — Consumer
 *
 *  Waits for data to appear in the queue, then prints it.
 *  Blocks automatically when queue is empty — uses zero CPU.
 *
 * ------------------------------------------------------- */
void vTask_Consumer(void *pv)
{
    uint8_t received;

    for (;;)
    {
        /*
         *  xQueueReceive(queue, &buffer, timeout)
         *  Waits for an item and copies it into 'received'.
         *  portMAX_DELAY = block forever until data arrives.
         *  Consumer sleeps here when queue is empty.
         */
        if (xQueueReceive(xCounterQueue, &received, portMAX_DELAY) == pdTRUE)
        {
            UART_sendString("[Consumer] Received: ");
            uart_print_number(received);
            UART_sendString("\r\n");
        }
    }
}

/* -------------------------------------------------------
 *  MAIN
 * ------------------------------------------------------- */
int main(void)
{
    UART_init(9600);

    UART_sendString("\r\n=== Queue Demo ===\r\n\r\n");

    /*
     *  Create queue BEFORE creating tasks.
     *
     *  xQueueCreate(length, itemSize)
     *  length   = max number of items queue can hold
     *  itemSize = size of each item in bytes
     *
     *  Here: holds up to 5 uint8_t values (5 x 1 byte)
     */
    xCounterQueue = xQueueCreate(5, sizeof(uint8_t));

    if (xCounterQueue == NULL)
    {
        UART_sendString("ERROR: Queue creation failed!\r\n");
        while (1);
    }

    /*
     *  Producer priority 1, Consumer priority 2
     *
     *  Consumer has higher priority so it processes
     *  items immediately when producer sends — the queue
     *  never fills up in this case.
     *
     *  TRY THIS: Make both priority 1 → items accumulate
     *  in the queue before consumer gets CPU time.
     */
    xTaskCreate(vTask_Producer, "PROD", 80, NULL, 1, NULL);
    xTaskCreate(vTask_Consumer, "CONS", 80, NULL, 2, NULL);

    vTaskStartScheduler();
    while (1);
}