#include "freertos/FreeRTOS.h"
#include "freertos/stream_buffer.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "detection.h"
#include "tinykw.h"
#include "keywords_encodings.h"
#include "led_state.h"

#define SAMPLE_RATE    16000
#define FRAME_MS       30
#define FRAME_SAMPLES  (FRAME_MS * SAMPLE_RATE / 1000)

#define DETECTION_THRESHOLD 128

static StreamBufferHandle_t audioStreamBuffer;
static QueueHandle_t ledCmdQueue;

static tkw_context tinykw_ctx;
static const uint8_t *keywords[] = {kw_yellow, kw_red, kw_green, kw_hey_computer, kw_stop};
static char *names[] = {"yellow", "red", "green", "hey computer", "stop"}; // for printing
static uint8_t led_state[] = {CMD_YELLOW, CMD_RED, CMD_GREEN, CMD_BLINK, CMD_ALL_OFF};

#define NB_KEYWORDS (sizeof(keywords) / sizeof(keywords[0]))
static uint8_t keyword_ids[NB_KEYWORDS];

static void on_detection(uint8_t keyword_id, void* args)
{
    // we send over the led command associated with the keyword
    uint8_t index = 0;
    while((index < NB_KEYWORDS) && (keyword_ids[index] != keyword_id)) index++;
    xQueueSend(ledCmdQueue, &led_state[index], 0);
    printf("Keyword detected: %s\n", names[index]);
}

static void initTinyKW(void)
{
    // Initializes TinyKW context
    tkw_status status;
    status = tkw_init(&tinykw_ctx);
    assert(status == TKW_OK);

    // Set callback function
    status = tkw_set_callback(on_detection, NULL, &tinykw_ctx);
    assert(status == TKW_OK);

    // Add the keywords to the engine and get corresponding ids
    for (int i = 0; i < NB_KEYWORDS; ++i)
    {
        status = tkw_add_keyword(keywords[i], (uint8_t)DETECTION_THRESHOLD, &(keyword_ids[i]), &tinykw_ctx);
        assert(status == TKW_OK);
    }
}


static void processAudioTask(void* args)
{
    int16_t pcm_data[FRAME_SAMPLES];
    while(1)
    {
        size_t nb_received = 0;
        while (nb_received < sizeof(pcm_data)) {
            nb_received += xStreamBufferReceive(audioStreamBuffer, (uint8_t *)pcm_data + nb_received, 
                                        sizeof(pcm_data) - nb_received, portMAX_DELAY);
        }
        tkw_status status;
        status = tkw_process_frame(pcm_data, FRAME_SAMPLES, &tinykw_ctx); // process one frame
        assert(status == TKW_OK);
    }
}

void startKeywordDetectionTask(StreamBufferHandle_t audio_buffer, QueueHandle_t led_cmd_queue, uint8_t priority)
{
    audioStreamBuffer = audio_buffer;
    ledCmdQueue = led_cmd_queue;
    initTinyKW();
    xTaskCreatePinnedToCore(processAudioTask, "processAudioTask", 4096, NULL, priority, NULL, 1); // Pin to core 1
}