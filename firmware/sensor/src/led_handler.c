#include "led_handler.h"
#include "esp_log.h"
#include "gpio_mapping.h"


static const char *TAG = "LED";

void led_init()
{
    ESP_LOGI(TAG, "Конфигурация lED...");

    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << LED_PIN),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE};
    gpio_config(&io_conf);

    gpio_set_level(LED_PIN, 1);    // Выключаем инверсный диод

    ESP_LOGI(TAG, "...LED инициализирован.");
}

void led_light(uint8_t seconds) {
    gpio_set_level(LED_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(seconds * 1000));
    gpio_set_level(LED_PIN, 1);
}