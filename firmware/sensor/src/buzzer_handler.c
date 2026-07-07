#include "buzzer_handler.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gpio_mapping.h"

static const char *TAG = "BUZZER";

void buzzer_init()
{
    ESP_LOGI(TAG, "Конфигурация BUZZER...");

    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << BUZZER_PIN),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE};
    gpio_config(&io_conf);

    gpio_set_level(BUZZER_PIN, 0); // Тишина для бузера

    ESP_LOGI(TAG, "...BUZZER инициализирован.");
}

void buzz(uint16_t seconds)
{
    gpio_set_level(BUZZER_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(seconds * 1000));
    gpio_set_level(BUZZER_PIN, 0);
}