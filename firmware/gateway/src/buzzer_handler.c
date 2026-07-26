#include "buzzer_handler.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gpio_mapping.h"

static const char *TAG = "BUZZER";

void buzzer_task(void *pvParameters)
{
    struct buzzer_t *buzzer = (struct buzzer_t *)pvParameters;

    while (1)
    {
        switch (buzzer->current_state)
        {
        case BUZZER_SILENCED:
            // Молчим: выключаем пин и спим 100 мс (не забиваем CPU)
            gpio_set_level(buzzer->gpio_num, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
            break;

        case BUZZER_WARNING:
            // Предупреждение: короткие пики (200 мс звук / 200 мс пауза)
            gpio_set_level(buzzer->gpio_num, 1);
            vTaskDelay(pdMS_TO_TICKS(200));
            gpio_set_level(buzzer->gpio_num, 0);
            vTaskDelay(pdMS_TO_TICKS(200));
            break;

        case BUZZER_ALARM:
            // Тревога: длинный громкий сигнал (1000 мс звук / 1000 мс пауза)
            gpio_set_level(buzzer->gpio_num, 1);
            vTaskDelay(pdMS_TO_TICKS(1000));
            gpio_set_level(buzzer->gpio_num, 0);
            vTaskDelay(pdMS_TO_TICKS(1000));
            break;
        }
    };
}

void buzzer_init(struct buzzer_t* buzzer, gpio_num_t gpio_num)
{
    ESP_LOGI(TAG, "Конфигурация BUZZER...");

    buzzer->gpio_num = gpio_num;
    buzzer->current_state = BUZZER_SILENCED;

    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << buzzer->gpio_num),
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE};
    gpio_config(&io_conf);

    xTaskCreate(buzzer_task, "buzzer_task", 3072, (void *)buzzer, 3, NULL);

    ESP_LOGI(TAG, "...BUZZER на пине %d инициализирован.", buzzer->gpio_num);
}

void buzzer_set_state(struct buzzer_t *buzzer, enum buzzer_state_t new_state)
{
    if (buzzer->current_state == new_state)
        return;

    buzzer->current_state = new_state;
}