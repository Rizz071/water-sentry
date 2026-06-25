#include "gpio_init.h"
#include "esp_log.h"

static const char *TAG = "GPIO_INIT";

void board_gpio_init(void)
{
    ESP_LOGI(TAG, "Конфигурация GPIO портов (Чистый Си)...");

    // В Си мы можем перечислять поля структуры в любом порядке и пропускать ненужные!
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << LED_PIN) | (1ULL << BUZZER_PIN),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE};
    gpio_config(&io_conf);

    gpio_set_level(LED_PIN, 1);    // Выключаем инверсный диод
    gpio_set_level(BUZZER_PIN, 0); // Тишина для бузера

    // Настройка кнопки
    gpio_config_t input_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << LINK_BUTTON_PIN),
        .pull_up_en = GPIO_PULLUP_ENABLE, // Подтяжка к 3.3В
        .pull_down_en = GPIO_PULLDOWN_DISABLE};
    gpio_config(&input_conf);

    ESP_LOGI(TAG, "GPIO порты успешно настроены.");
}