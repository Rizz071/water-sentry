#include "system_events.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_log.h"

static const char *TAG = "SYS_EVENTS";
#define QUEUE_LENGTH 10

static QueueHandle_t sys_event_queue = NULL;

void system_events_init(void)
{
    if (sys_event_queue == NULL)
    {
        sys_event_queue = xQueueCreate(QUEUE_LENGTH, sizeof(struct system_event_t));
        configASSERT(sys_event_queue); // Падаем при старте, если не хватило RAM
        ESP_LOGI(TAG, "Очередь системных событий создана.");
    }
}

bool system_event_post(const struct system_event_t *ev)
{
    if (sys_event_queue == NULL)
        return false;

    if (xQueueSend(sys_event_queue, ev, pdMS_TO_TICKS(10)) != pdPASS)
    {
        ESP_LOGW(TAG, "Очередь событий переполнена! Событие %d потеряно", ev->type);
        return false;
    }
    return true;
}

// Вызов из главной задачи менеджера
bool system_event_receive(struct system_event_t *ev, uint32_t wait_ms)
{
    if (sys_event_queue == NULL)
        return false;

    TickType_t ticks = (wait_ms == portMAX_DELAY) ? portMAX_DELAY : pdMS_TO_TICKS(wait_ms);
    return (xQueueReceive(sys_event_queue, ev, ticks) == pdTRUE);
}