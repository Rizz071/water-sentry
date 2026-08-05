#include "ui_controller.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"

static const char *TAG = "UI_CTRL";

#define UI_TASK_PERIOD_MS 50  // 50 ms render loop
#define PING_BLINK_OFF_MS 100 // LED off duration after ping

static led_strip_hal_t *g_led_strip = NULL;
static buzzer_hal_t *g_buzzer = NULL;
static const sensor_slot_t *g_slots = NULL;
static size_t g_slot_count = 0;

static void ui_task(void *pvParameters)
{
    esp_task_wdt_add(NULL);

    while (1)
    {
        esp_task_wdt_reset();

        bool has_alarm = false;
        bool has_unacked_offline = false;
        uint32_t now = xTaskGetTickCount();

        for (size_t i = 0; i < g_slot_count; i++)
        {
            led_color_t *led = &g_led_strip->leds[i];

            switch (g_slots[i].state)
            {
            case SLOT_EMPTY:
                led->red = 0;
                led->green = 0;
                led->blue = 0;
                break;

            case SLOT_PAIRING:
                led->red = 0;
                led->green = 0;
                led->blue = 255;
                break;

            case SLOT_OK:
                // Brief blink-off after ping: LED goes dark for PING_BLINK_OFF_MS
                if (g_slots[i].last_ping_ms != 0 &&
                    (now - g_slots[i].last_ping_ms) < pdMS_TO_TICKS(PING_BLINK_OFF_MS))
                {
                    led->red = 0;
                    led->green = 0;
                    led->blue = 0;
                }
                else
                {
                    led->red = 0;
                    led->green = 255;
                    led->blue = 0;
                }
                break;

            case SLOT_OFFLINE:
                led->red = 0;
                led->green = 255;
                led->blue = 255;
                if (!g_slots[i].offline_acked)
                {
                    has_unacked_offline = true;
                }
                break;

            case SLOT_ALARM:
                led->red = 255;
                led->green = 0;
                led->blue = 0;
                if (!g_slots[i].alarm_acked)
                {
                    has_alarm = true;
                }
                break;
            }
        }

        led_strip_hal_update(g_led_strip);

        if (has_alarm)
        {
            buzzer_hal_set_state(g_buzzer, BUZZER_STATE_ALARM);
        }
        else if (has_unacked_offline)
        {
            buzzer_hal_set_state(g_buzzer, BUZZER_STATE_WARNING);
        }
        else
        {
            buzzer_hal_set_state(g_buzzer, BUZZER_STATE_SILENCED);
        }

        vTaskDelay(pdMS_TO_TICKS(UI_TASK_PERIOD_MS));
    }
}

void ui_controller_init(led_strip_hal_t *led_strip, buzzer_hal_t *buzzer,
                        const sensor_slot_t *slots, size_t count)
{
    g_led_strip = led_strip;
    g_buzzer = buzzer;
    g_slots = slots;
    g_slot_count = count;

    xTaskCreate(ui_task, "ui_task", 3072, NULL, 3, NULL);
    ESP_LOGI(TAG, "UI controller started (render period=%d ms, ping blink=%d ms).",
             UI_TASK_PERIOD_MS, PING_BLINK_OFF_MS);
}