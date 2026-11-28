#ifndef AUDIO_I2S_H
#define AUDIO_I2S_H

#include "freertos/FreeRTOS.h"
#include "freertos/stream_buffer.h"

void startAudioTask(int pin_sck, int pin_ws, int pin_sd, StreamBufferHandle_t audio_buffer, uint8_t priority);

#endif