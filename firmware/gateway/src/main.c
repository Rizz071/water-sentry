#include "buzzer_handler.h"
#include "led_handler.h"
#include "link_button_handle.h"
#include "lora_handler.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gpio_mapping.h"

static const char *TAG = "MAIN_BASE";

void app_main(void)
{
    // Будим USB-порт/CDC-консоль, чтобы успеть поймать первые логи в мониторе
    vTaskDelay(pdMS_TO_TICKS(2000));
    ESP_LOGI(TAG, "Старт системы контроля протечек Water Sentry (Базовая станция)");

    // 1. Инициализируем радиомодуль LoRa (внутри него включится режим RX)
    if (!lora_init())
    {
        ESP_LOGE(TAG, "Критическая ошибка: Старт LoRa провален! Система остановлена.");
        return;
    }

    // 2. Инициализация исполнительных устройств и интерфейса базы
    buzzer_init(BUZZER_PIN);      // Наш мощный бузер-сирена через AO3400
    led_init(LED_PIN);         // Светодиод индикации статуса/привязки
    link_button_init(LINK_BUTTON_PIN); // Кнопка для спаривания с новыми датчиками

    // 3. Запускаем бесконечный поток прослушивания эфира
    // Выделяем под него 4096 байт стека, так как ESP_LOG довольно прожорлив к памяти
    xTaskCreate(
        lora_listen_task,   // Функция таска
        "lora_listen_task", // Имя для дебага
        4096,               // Стек
        NULL,               // Параметры
        5,                  // Приоритет
        NULL                // Хэндл
    );

    ESP_LOGI(TAG, "Базовая станция Water Sentry успешно запущена и слушает датчики.");
}