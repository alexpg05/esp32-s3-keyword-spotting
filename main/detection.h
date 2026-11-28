#ifndef DETECTION_H
#define DETECTION_H

#include "freertos/FreeRTOS.h"
#include "freertos/stream_buffer.h"
#include "freertos/queue.h"

void startKeywordDetectionTask(StreamBufferHandle_t audio_buffer, QueueHandle_t led_cmd_queue, uint8_t priority);

#endif
