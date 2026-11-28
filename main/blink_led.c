#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "led_strip.h"
#include "blink_led.h"
#include "led_state.h"

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
    QueueHandle_t ledCmdQueue = (QueueHandle_t)args;
    bool is_blinking = false;
    uint8_t command = CMD_ALL_OFF;
    uint8_t r = 0, g = 0, b = 0;
    while(1)
    {
        if (xQueueReceive(ledCmdQueue, &command, 200 / portTICK_PERIOD_MS) == pdTRUE)
        {
            switch(command)
            {
                case CMD_YELLOW:
                    r = 16; g = 16; b = 0;
                    break;
                case CMD_RED:
                    r = 16; g = 0; b = 0;
                    break;
                case CMD_GREEN:
                    r = 0; g = 16; b = 0;
                    break;
                case CMD_BLINK:
                    r = 16; g = 16; b = 0;
                    is_blinking = true;
                    break;
                case CMD_ALL_OFF:
                    r = 0; g = 0; b = 0;
                    break;
                default:
                    r = 0; g = 0; b = 0;
                    break;
            }
            led_strip_set_pixel(led_strip, 0, r, g, b);
            led_strip_refresh(led_strip);
        }
        else
        {
            if (command == CMD_BLINK)
            {
                if (is_blinking)
                    led_strip_set_pixel(led_strip, 0, 0, 0, 0);
                else
                    led_strip_set_pixel(led_strip, 0, r, g, b);
                led_strip_refresh(led_strip);
                is_blinking = !is_blinking;
            }
        }
    }
}


void startLedTask(QueueHandle_t led_cmd_queue, uint8_t priority)
{
    configure_led();
    xTaskCreate(ledTask, "ledTask", 4096, (void*)led_cmd_queue, priority, NULL);
}