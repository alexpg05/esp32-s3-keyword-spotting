#include "freertos/FreeRTOS.h"
#include "freertos/stream_buffer.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "detection.h"
#include "tinykw.h"
#include "keywords_encodings.h"

#define SAMPLE_RATE    16000
#define FRAME_MS       30
#define FRAME_SAMPLES  (FRAME_MS * SAMPLE_RATE / 1000)

#define DETECTION_THRESHOLD 128
#define NB_KEYWORDS         7

static QueueHandle_t detectionQueue;

static tkw_context tinykw_ctx;
static const uint8_t *keywords[] = {kw_yellow, kw_red, kw_green, kw_blink_slow, kw_blink_fast, kw_stop, kw_continuous};
static char *names[] = {"yellow", "red", "green", "blink_slow", "blink_fast", "stop", "continuous"};
static uint8_t keyword_ids[NB_KEYWORDS];

static void on_detection(uint8_t keyword_id, void* args)
{
    // we just send over the index of the keyword to the queue
    uint8_t index = 0;
    while((index < NB_KEYWORDS) && (keyword_ids[index] != keyword_id)) index++;
    xQueueSend(detectionQueue, &index, 0);
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
    StreamBufferHandle_t buffer = (StreamBufferHandle_t) args;
    while(1)
    {
        size_t nb_received = 0;
        while (nb_received < sizeof(pcm_data)) {
            nb_received += xStreamBufferReceive(buffer, (uint8_t *)pcm_data + nb_received, 
                                        sizeof(pcm_data) - nb_received, portMAX_DELAY);
        }
        tkw_status status;
        status = tkw_process_frame(pcm_data, FRAME_SAMPLES, &tinykw_ctx); // process one frame
        assert(status == TKW_OK);
    }
}

void startKeywordDetectionTask(StreamBufferHandle_t audio_buffer, uint8_t priority)
{
    detectionQueue = xQueueCreate(3, sizeof(uint8_t));
    initTinyKW();
    // Pin the processing task to core 1
    xTaskCreatePinnedToCore(processAudioTask, "processAudioTask", 4096, (void *)audio_buffer, priority, NULL, 1);
}

QueueHandle_t getDetectionQueue(void)
{
    return detectionQueue;
}