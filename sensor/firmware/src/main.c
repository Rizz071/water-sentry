#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

// Подключаем наши Си-модули
#include "gpio_init.h"
#include "lora_handler.h"

static const char *TAG = "MAIN";

void app_main(void)
{
    // 1. Инициализация кнопок и светодиодов
    board_gpio_init();

    // 2. Инициализация LoRa (задаем устройству ID = 42)
    if (!board_lora_init(42))
    {
        ESP_LOGE(TAG, "Авария: Радиоканал недоступен!");
    }

    ESP_LOGI(TAG, "Система запущена. Переходим в цикл опроса датчиков...");

    while (1)
    {
        // Имитируем опрос: если кнопка нажата (физический 0) — значит пошла вода
        bool liquid_detected = (gpio_get_level(LINK_BUTTON_PIN) == 0);

        if (liquid_detected)
        {
            gpio_set_level(LED_PIN, 0); // Зажигаем диод тревоги
            // Шлем пакет с флагом тревоги (true) и признаком затопления (true)
            send_lora_data(true, true, 3600);
        }
        else
        {
            gpio_set_level(LED_PIN, 1);
            // Обычный плановый Heartbeat раз в 10 секунд (для теста)
            send_lora_data(false, false, 3600);
        }

        vTaskDelay(pdMS_TO_TICKS(10000)); // Ждем 10 секунд
    }
}