#ifndef BLINK_LED_H
#define BLINK_LED_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

void startLedTask(QueueHandle_t led_cmd_queue, uint8_t priority);

#endif