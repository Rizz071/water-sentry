#include "water_sensor_handler.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "link_button_handle.h" // Для функции generate_unique_id()
#include "lora_handler.h"       // Для отправки пакетов пинга и тревоги

static const char *TAG = "WATER_SENS";

#define SENSOR_POLL_PERIOD_MS 200  // Проверяем воду каждые 200 мс (быстрая реакция)
#define HEARTBEAT_INTERVAL_MS 5000 // Интервал отправки пинга (5 секунд для теста)

#define TICKS_FOR_HEARTBEAT (HEARTBEAT_INTERVAL_MS / SENSOR_POLL_PERIOD_MS) // 25 циклов

static void water_sensor_task(void *pvParameters)
{
    gpio_num_t sensor_pin = (gpio_num_t)pvParameters;
    uint32_t unique_id = generate_unique_id();

    uint32_t heartbeat_counter = 0;
    bool is_flooded = false;

    ESP_LOGI(TAG, "Задача мониторинга протечки запущена на GPIO %d. Мой ID: 0x%08X", sensor_pin, unique_id);

    while (1)
    {
        // Считываем текущее состояние сенсора (0 - сухо, 1 - вода)
        bool current_water_state = (gpio_get_level(sensor_pin) == 1);

        if (current_water_state)
        {
            // =================================================================
            // СИТУАЦИЯ: ОБНАРУЖЕНА ВОДА
            // =================================================================
            if (!is_flooded)
            {
                // Вода обнаружена только что (смена состояния)
                is_flooded = true;
                ESP_LOGE(TAG, "Обнаружена протечка! Мгновенный выстрел тревоги в эфир!");
                lora_send_real_alarm_packet(unique_id);
                heartbeat_counter = 0; // Сбрасываем счетчик, чтобы контролировать шаг повторов
            }
            else
            {
                // Вода стоит уже давно. Чтобы не сжечь эфир, шлем аварийный пакет
                // раз в период heartbeat_counter (каждые 5 секунд), напоминая базе о беде
                heartbeat_counter++;
                if (heartbeat_counter >= TICKS_FOR_HEARTBEAT)
                {
                    ESP_LOGW(TAG, "Протечка продолжается. Повторная отправка сигнала тревоги...");
                    lora_send_real_alarm_packet(unique_id);
                    heartbeat_counter = 0;
                }
            }
        }
        else
        {
            // =================================================================
            // СИТУАЦИЯ: СУХО
            // =================================================================
            if (is_flooded)
            {
                // Вода ушла (высохло / датчик убрали из лужи)
                is_flooded = false;
                ESP_LOGW(TAG, "Уровень воды пришел в норму (высохло).");
                // Отправляем один штатный пинг, чтобы база сразу поняла, что авария снята
                lora_send_ping_packet(unique_id);
                heartbeat_counter = 0;
            }
            else
            {
                // Всё спокойно, штатный режим. Считаем время до следующего Heartbeat
                heartbeat_counter++;
                if (heartbeat_counter >= TICKS_FOR_HEARTBEAT)
                {
                    lora_send_ping_packet(unique_id);
                    heartbeat_counter = 0; // Обнуляем таймер пинга
                }
            }
        }

        // Засыпаем на 200 мс. Процессор отдыхает, FreeRTOS переключается на другие задачи
        vTaskDelay(pdMS_TO_TICKS(SENSOR_POLL_PERIOD_MS));
    }
}

void water_sensor_init(gpio_num_t sensor_gpio)
{
    ESP_LOGI(TAG, "Конфигурация датчика уровня жидкости...");

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << sensor_gpio),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE // Опрос в цикле
    };
    gpio_config(&io_conf);

    // Создаем независимый поток для контроля воды
    xTaskCreate(water_sensor_task, "water_sensor_task", 3072, (void *)sensor_gpio, 12, NULL);

    ESP_LOGI(TAG, "...датчик уровня жидкости инициализирован.");
}