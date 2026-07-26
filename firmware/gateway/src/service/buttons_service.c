#include "buttons_service.h"
#include "hal/buttons_hal.h"
#include "event_bus.h"
#include "app_config.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"

static const char *TAG = "BTN_SVC";

static void buttons_polling_task(void *pvParameters)
{
    buttons_hal_init();

    uint8_t prev_button = 0;
    uint32_t press_start_ms = 0;
    bool long_press_sent = false;

    ESP_LOGI(TAG, "Buttons polling task started (long press=3s).");

    esp_task_wdt_add(NULL);

    while (1)
    {
        esp_task_wdt_reset();

        uint8_t raw = buttons_hal_read();

        if (raw != 0 && prev_button == 0)
        {
            // Button just pressed (hardware-debounced)
            ESP_LOGI(TAG, "Button [%d] pressed.", raw);
            press_start_ms = xTaskGetTickCount();
            long_press_sent = false;
        }
        else if (raw == 0 && prev_button != 0)
        {
            // Button just released
            uint32_t held_ms = (xTaskGetTickCount() - press_start_ms) * portTICK_PERIOD_MS;
            ESP_LOGI(TAG, "Button [%d] released (held %lu ms).", prev_button, held_ms);

            if (!long_press_sent)
            {
                // Short press
                event_t ev = {
                    .type = EVENT_BTN_PAIR_PRESSED,
                    .button_num = prev_button};
                event_bus_post(&ev);
            }
        }
        else if (raw != 0 && !long_press_sent)
        {
            // Button still held — check for long press
            uint32_t held_ms = (xTaskGetTickCount() - press_start_ms) * portTICK_PERIOD_MS;
            if (held_ms >= 3000)
            {
                ESP_LOGI(TAG, "Button [%d] long press detected.", raw);
                event_t ev = {
                    .type = EVENT_BTN_LONG_PRESS,
                    .button_num = raw};
                event_bus_post(&ev);
                long_press_sent = true;
            }
        }

        prev_button = raw;

        vTaskDelay(pdMS_TO_TICKS(BUTTON_POLL_MS));
    }
}

void buttons_service_init(void)
{
    xTaskCreate(buttons_polling_task, "buttons_polling_task", 3072, NULL, 5, NULL);
    ESP_LOGI(TAG, "Buttons service started.");
}