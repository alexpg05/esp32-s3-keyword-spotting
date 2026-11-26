#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "led_strip.h"
#include "blink_led.h"

#define BLINK_GPIO 48

static led_strip_handle_t led_strip;

static void configure_led(void)
{
    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = {
        .strip_gpio_num = BLINK_GPIO,
        .max_leds = 1, // at least one LED on board
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    /* Set all LED off to clear all pixels */
    led_strip_clear(led_strip);
}


static void ledTask(void* args)
{
    QueueHandle_t detectionQueue = (QueueHandle_t)args;
    while(1)
    {
        uint8_t command;
        xQueueReceive(detectionQueue, &command, portMAX_DELAY);

        // each command corresponds to a keyword. But here for sake 
        // of simplicity we just create random color for each
        switch(command)
        {
            case 0:
                led_strip_set_pixel(led_strip, 0, 16, 0, 0);
                break;
            case 1:
                led_strip_set_pixel(led_strip, 0, 0, 16, 0);
                break;
            case 2:
                led_strip_set_pixel(led_strip, 0, 0, 0, 16);
                break;
            case 3:
                led_strip_set_pixel(led_strip, 0, 16, 16, 0);
                break;
            case 4:
                led_strip_set_pixel(led_strip, 0, 0, 16, 16);
                break;
            case 5:
                led_strip_set_pixel(led_strip, 0, 16, 0, 16);
                break;
            case 6:
                led_strip_set_pixel(led_strip, 0, 16, 16, 16);
                break;
            default:
                led_strip_clear(led_strip);
                break;
        }
        led_strip_refresh(led_strip);
    }
}


void startLedTask(QueueHandle_t detectionQueue, uint8_t priority)
{
    configure_led();
    xTaskCreate(ledTask, "ledTask", 4096, (void*)detectionQueue, priority, NULL);
}