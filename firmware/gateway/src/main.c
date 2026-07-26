#include "buzzer_handler.h"
#include "led_strip_handler.h"
#include "link_button_handle.h"
#include "lora_handler.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gpio_mapping.h"
#include "nvs_storage_handler.h"
#include "system_events.h"
#include "system_manager.h"

#define MAX_REMOTE_SENSORS_COUNT 5

static const char *TAG = "MAIN_GATEWAY";

struct buzzer_t buzzer;
struct rgb_led_strip_handle_t led_strip;
struct sensor_slot_t slots[MAX_REMOTE_SENSORS_COUNT];

void app_main(void)
{
    vTaskDelay(pdMS_TO_TICKS(3000));
    ESP_LOGI(TAG, "Старт системы контроля протечек Water Sentry (Базовая станция)");

    // 1. Инициализация подсистемы Flash/NVS
    nvs_storage_init();

    // 2. Инициализация исполнительных устройств и интерфейсов
    buzzer_init(&buzzer, BUZZER_PIN);
    rgb_led_strip_init(&led_strip, LED_LINE_PIN, 5);
    link_buttons_init(LINK_BUTTONS_PIN);

    // 3. Инициализация системных событий
    system_events_init();

    // 4. Загрузка MAC-адресов из NVS
    uint8_t sensors_mac_list[MAX_REMOTE_SENSORS_COUNT];
    nvs_storage_load_sensors(sensors_mac_list, MAX_REMOTE_SENSORS_COUNT);

    for (size_t i = 0; i < MAX_REMOTE_SENSORS_COUNT; i++)
    {
        ESP_LOGI(TAG, "Загружен MAC[%d]: 0x%02X", (int)i, sensors_mac_list[i]);
    }

    // 5. Заполнение структурированных слотов и присвоение статусов
    for (size_t i = 0; i < MAX_REMOTE_SENSORS_COUNT; i++)
    {
        slots[i].mac_addr = sensors_mac_list[i];
        if (slots[i].mac_addr != 0)
        {
            slots[i].state = SLOT_OK; // ✅ Ключевое исправление!
            slots[i].last_seen_ms = xTaskGetTickCount();
        }
        else
        {
            slots[i].state = SLOT_EMPTY;
        }
    }

    // 6. Инициализация менеджера
    system_manager_init();

    // 7. Инициализация радиомодуля LoRa
    if (!lora_hw_init())
    {
        ESP_LOGE(TAG, "Критическая ошибка: Старт LoRa провален! Система остановлена.");
        return;
    }

    ESP_LOGI(TAG, "Базовая станция Water Sentry успешно запущена и слушает датчики.");
}

// TODO перестал поступать сигнал от датчика - постоянный писк либо сброс по кнопке