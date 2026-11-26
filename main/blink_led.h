#ifndef BLINK_LED_H
#define BLINK_LED_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

void startLedTask(QueueHandle_t detectionQueue, uint8_t priority);

#endif