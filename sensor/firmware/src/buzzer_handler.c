#include "buzzer_handler.h"
#include "esp_log.h"


static const char *TAG = "BUZZER_INIT";

void buzzer_init(gpio_num_t buzzer_gpio)
{
    ESP_LOGI(TAG, "Конфигурация BUZZER...");

    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << buzzer_gpio),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE};
    gpio_config(&io_conf);

    gpio_set_level(buzzer_gpio, 0); // Тишина для бузера

    ESP_LOGI(TAG, "...BUZZER инициализирован.");
}