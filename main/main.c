/*
This application demonstrates keyword spotting on ESP32-S3. Three tasks are started:
- `startAudioTask`: reads the audio from microphone using the i2s driver and sends the pcm data to a stream buffer.
- `startKeywordDetectionTask`: reads a frame of pcm data from the stream buffer and processes it. 
                               If a keyword is detected, its id is sent over to a queue.
- `startLedTask`: reads the queue of detected keywords and activates the led accordingly.
*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <stdio.h>

#include "audio_i2s.h"
#include "detection.h"
#include "blink_led.h"

void app_main(void)
{
    startAudioTask(GPIO_NUM_4, GPIO_NUM_5, GPIO_NUM_6, 5);
    startKeywordDetectionTask(getAudioStreamBuffer(), 4);
    startLedTask(getDetectionQueue(), 3);
}

