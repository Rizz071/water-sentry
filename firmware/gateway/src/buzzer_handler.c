#include "buzzer_handler.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "BUZZER_INIT";
static gpio_num_t BUZZER_PIN;

void buzzer_init(gpio_num_t buzzer_gpio)
{
    ESP_LOGI(TAG, "Конфигурация BUZZER...");

    BUZZER_PIN = buzzer_gpio;

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