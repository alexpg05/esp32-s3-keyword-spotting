/*
This task reads audio data using the i2s driver and places it in a streambuffer
*/
#include "audio_i2s.h"
#include "freertos/FreeRTOS.h"
#include "freertos/stream_buffer.h"
#include "driver/i2s_std.h"

#define SAMPLE_RATE     16000
#define FRAME_SAMPLES   (30 * SAMPLE_RATE / 1000) // we read by chunk of 30 ms

static i2s_chan_handle_t rx_chan;

static void init_i2s_receive(int pin_sck, int pin_ws, int pin_sd)
{
    i2s_chan_config_t rx_chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&rx_chan_cfg, NULL, &rx_chan));

    i2s_std_config_t rx_std_cfg = {
        .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = pin_sck,
            .ws   = pin_ws,
            .dout = I2S_GPIO_UNUSED,
            .din  = pin_sd,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
    };
    rx_std_cfg.slot_cfg.slot_mask = I2S_STD_SLOT_LEFT;           // L/R pin tied to GND
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx_chan, &rx_std_cfg));
}

static void i2s_read_task(void *args)
{
    StreamBufferHandle_t audioBuffer = (StreamBufferHandle_t)args;
    ESP_ERROR_CHECK(i2s_channel_enable(rx_chan));
    size_t r_bytes = 0;
    int32_t raw_data[FRAME_SAMPLES];
    int16_t pcm_data[FRAME_SAMPLES];
    while (1) {
        if (i2s_channel_read(rx_chan, raw_data, sizeof(raw_data), &r_bytes, 1000) == ESP_OK) {
            size_t n = r_bytes / sizeof(int32_t);
            for (size_t i = 0; i < n; ++i)
            {
                pcm_data[i] = raw_data[i] >> 16;
            }
            xStreamBufferSend(audioBuffer, pcm_data, n * sizeof(int16_t), 0);
        } else {
            printf("Read Task: i2s read failed\n");
        }
    }
}

void startAudioTask(int pin_sck, int pin_ws, int pin_sd, StreamBufferHandle_t audio_buffer, uint8_t priority)
{
    init_i2s_receive(pin_sck, pin_ws, pin_sd);
    xTaskCreate(i2s_read_task, "i2s_read_task", 4096, (void*)audio_buffer, priority, NULL);
}