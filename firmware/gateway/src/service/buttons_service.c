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

    uint8_t current_button = 0;
    uint8_t stable_button = 0;
    uint8_t debounce_counter = 0;
    uint32_t press_start_ms = 0;
    bool long_press_sent = false;

    ESP_LOGI(TAG, "Buttons polling task started (debounce=%d cycles, long press=3s).", BUTTON_DEBOUNCE_CNT);

    esp_task_wdt_add(NULL);

    while (1)
    {
        esp_task_wdt_reset();

        uint8_t raw = buttons_hal_read();

        if (raw == current_button)
        {
            debounce_counter++;
            if (debounce_counter >= BUTTON_DEBOUNCE_CNT)
            {
                if (current_button != stable_button)
                {
                    if (current_button != 0)
                    {
                        // Button just pressed
                        ESP_LOGI(TAG, "Button [%d] pressed.", current_button);
                        press_start_ms = xTaskGetTickCount();
                        long_press_sent = false;
                    }
                    else
                    {
                        // Button just released
                        uint32_t held_ms = (xTaskGetTickCount() - press_start_ms) * portTICK_PERIOD_MS;
                        ESP_LOGI(TAG, "Button [%d] released (held %lu ms).", stable_button, held_ms);

                        if (!long_press_sent)
                        {
                            // Short press
                            event_t ev = {
                                .type = EVENT_BTN_PAIR_PRESSED,
                                .button_num = stable_button};
                            event_bus_post(&ev);
                        }
                    }
                    stable_button = current_button;
                }
                else if (current_button != 0 && !long_press_sent)
                {
                    // Button still held — check for long press
                    uint32_t held_ms = (xTaskGetTickCount() - press_start_ms) * portTICK_PERIOD_MS;
                    if (held_ms >= 3000)
                    {
                        ESP_LOGI(TAG, "Button [%d] long press detected.", current_button);
                        event_t ev = {
                            .type = EVENT_BTN_LONG_PRESS,
                            .button_num = current_button};
                        event_bus_post(&ev);
                        long_press_sent = true;
                    }
                }
                debounce_counter = BUTTON_DEBOUNCE_CNT; // Clamp
            }
        }
        else
        {
            current_button = raw;
            debounce_counter = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(BUTTON_POLL_MS));
    }
}

void buttons_service_init(void)
{
    xTaskCreate(buttons_polling_task, "buttons_polling_task", 3072, NULL, 5, NULL);
    ESP_LOGI(TAG, "Buttons service started.");
}