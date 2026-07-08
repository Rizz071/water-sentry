#include "led_handler.h"
#include "gpio_mapping.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "LED_INIT";

void led_init(gpio_num_t gpio_num)
{
    // ESP_LOGI(TAG, "Конфигурация lED...");

    // gpio_config_t io_conf = {
    //     .intr_type = GPIO_INTR_DISABLE,
    //     .mode = GPIO_MODE_OUTPUT,
    //     .pin_bit_mask = (1ULL << LED_LINE_PIN),
    //     .pull_down_en = GPIO_PULLDOWN_DISABLE,
    //     .pull_up_en = GPIO_PULLUP_DISABLE};
    // gpio_config(&io_conf);

    // gpio_set_level(LED_LINE_PIN, 1); // Выключаем инверсный диод

    // ESP_LOGI(TAG, "...LED инициализирован.");
}

void led_light(gpio_num_t gpio_num, uint8_t seconds)
{
    // gpio_set_level(gpio_num, 0);
    // vTaskDelay(pdMS_TO_TICKS(seconds * 1000));
    // gpio_set_level(gpio_num, 1);
}