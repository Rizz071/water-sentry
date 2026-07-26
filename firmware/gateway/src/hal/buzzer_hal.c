#include "buzzer_hal.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"

static const char *TAG = "BUZZER_HAL";

static void buzzer_task(void *pvParameters)
{
    buzzer_hal_t *buzzer = (buzzer_hal_t *)pvParameters;

    esp_task_wdt_add(NULL);
    ESP_LOGI(TAG, "Task watchdog subscribed for buzzer task.");

    while (1)
    {
        esp_task_wdt_reset();

        switch (buzzer->current_state)
        {
        case BUZZER_STATE_SILENCED:
            gpio_set_level(buzzer->gpio_num, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
            break;

        case BUZZER_STATE_WARNING:
            gpio_set_level(buzzer->gpio_num, 1);
            vTaskDelay(pdMS_TO_TICKS(200));
            gpio_set_level(buzzer->gpio_num, 0);
            vTaskDelay(pdMS_TO_TICKS(200));
            break;

        case BUZZER_STATE_ALARM:
            gpio_set_level(buzzer->gpio_num, 1);
            vTaskDelay(pdMS_TO_TICKS(1000));
            gpio_set_level(buzzer->gpio_num, 0);
            vTaskDelay(pdMS_TO_TICKS(1000));
            break;
        }
    }
}

void buzzer_hal_init(buzzer_hal_t *buzzer, gpio_num_t gpio_num)
{
    ESP_LOGI(TAG, "Configuring buzzer hardware...");

    buzzer->gpio_num = gpio_num;
    buzzer->current_state = BUZZER_STATE_SILENCED;

    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << buzzer->gpio_num),
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE};
    gpio_config(&io_conf);

    xTaskCreate(buzzer_task, "buzzer_task", 3072, (void *)buzzer, 3, NULL);

    ESP_LOGI(TAG, "Buzzer on pin %d initialized.", buzzer->gpio_num);
}

void buzzer_hal_set_state(buzzer_hal_t *buzzer, buzzer_state_t new_state)
{
    if (buzzer->current_state == new_state)
        return;

    buzzer->current_state = new_state;
}