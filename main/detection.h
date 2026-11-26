#ifndef DETECTION_H
#define DETECTION_H

#include "freertos/FreeRTOS.h"
#include "freertos/stream_buffer.h"

void startKeywordDetectionTask(StreamBufferHandle_t audio_buffer, uint8_t priority);
QueueHandle_t getDetectionQueue(void);

#endif