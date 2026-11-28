/*
This application demonstrates keyword spotting on ESP32-S3. Three tasks are started:
- `startAudioTask`: reads the audio from microphone using the i2s driver and sends the pcm data to a stream buffer.
- `startKeywordDetectionTask`: reads a frame of pcm data from the stream buffer and processes it. 
                               If a keyword is detected, the associated led command is sent over to a queue.
- `startLedTask`: reads the queue of detected keywords and activates the led accordingly.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "audio_i2s.h"
#include "detection.h"
#include "blink_led.h"
#include "tinykw.h"

#define FRAME_BYTES (TKW_AUDIO_FRAME_SIZE * sizeof(int16_t)) // 30 ms of audio at 16 kHz

static StreamBufferHandle_t audioStreamBuffer;
static QueueHandle_t ledCommandQueue;

void app_main(void)
{
    audioStreamBuffer = xStreamBufferCreate(4 * FRAME_BYTES, FRAME_BYTES);
    ledCommandQueue = xQueueCreate(3, sizeof(uint8_t));

    startAudioTask(GPIO_NUM_4, GPIO_NUM_5, GPIO_NUM_6, audioStreamBuffer, 5);
    startKeywordDetectionTask(audioStreamBuffer, ledCommandQueue,  4);
    startLedTask(ledCommandQueue, 3);
}

