#include "led_handler.h"
#include "esp_log.h"


static const char *TAG = "LED_INIT";

void led_init(gpio_num_t led_gpio)
{
    ESP_LOGI(TAG, "Конфигурация lED...");

    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << led_gpio),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE};
    gpio_config(&io_conf);

    gpio_set_level(led_gpio, 1);    // Выключаем инверсный диод

    ESP_LOGI(TAG, "...LED инициализирован.");
}