#include "event_bus.h"
#include "app_config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_log.h"

static const char *TAG = "EVENT_BUS";
static QueueHandle_t sys_event_queue = NULL;

void event_bus_init(void)
{
    if (sys_event_queue == NULL)
    {
        sys_event_queue = xQueueCreate(EVENT_QUEUE_LENGTH, sizeof(event_t));
        configASSERT(sys_event_queue);
        ESP_LOGI(TAG, "Event queue created (capacity %d).", EVENT_QUEUE_LENGTH);
    }
}

bool event_bus_post(const event_t *ev)
{
    if (sys_event_queue == NULL)
        return false;

    if (xQueueSend(sys_event_queue, ev, pdMS_TO_TICKS(10)) != pdPASS)
    {
        ESP_LOGW(TAG, "Event queue overflow! Event %d dropped.", ev->type);
        return false;
    }
    return true;
}

bool event_bus_receive(event_t *ev, uint32_t timeout_ms)
{
    if (sys_event_queue == NULL)
        return false;

    TickType_t ticks = (timeout_ms == portMAX_DELAY) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return (xQueueReceive(sys_event_queue, ev, ticks) == pdTRUE);
}