#include "buzzer_handler.h"
#include "led_handler.h"
#include "link_button_handle.h"
#include "lora_handler.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gpio_mapping.h"

static const char *TAG = "MAIN_GATEWAY";

struct buzzer_t buzzer;

void app_main(void)
{
    // Будим USB-порт/CDC-консоль, чтобы успеть поймать первые логи в мониторе
    vTaskDelay(pdMS_TO_TICKS(3000));
    ESP_LOGI(TAG, "Старт системы контроля протечек Water Sentry (Базовая станция)");

    // 1. Инициализация исполнительных устройств и интерфейса базы
    buzzer_init(&buzzer, BUZZER_PIN);    // Пищалка
    led_init(LED_LINE_PIN);              // Светодиод индикации статуса/привязки
    link_buttons_init(LINK_BUTTONS_PIN); // Кнопка для спаривания с новыми датчиками

    // 2. Инициализируем радиомодуль LoRa (внутри него включится режим RX)
    if (!lora_hw_init())
    {
        ESP_LOGE(TAG, "Критическая ошибка: Старт LoRa провален! Система остановлена.");
        return;
    }

    ESP_LOGI(TAG, "Базовая станция Water Sentry успешно запущена и слушает датчики.");
}

// TODO перестал поступать сигнал от датчика - постоянный писк либо сброс по кнопке