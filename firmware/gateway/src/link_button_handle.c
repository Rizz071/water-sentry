#include "link_button_handle.h"
#include "lora_handler.h"
#include "buzzer_handler.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"

static const char *TAG = "LINK_BUTTONS";

#define BUTTONS_ADC_UNIT ADC_UNIT_1
#define BUTTON_ADC_CHANNEL ADC_CHANNEL_1 // GPIO1
#define BUTTON_POLL_INTERVAL_MS 20       // Опрос каждые 20 мс

// Функция перевода сырых попугаев АЦП в номер физической кнопки
static uint8_t get_button_from_adc(int adc_raw)
{
    if (adc_raw > 3500)
        return 0; // Ничего не нажато (подтяжка к 3.3В)

    if (adc_raw >= 0 && adc_raw < 300)
        return 1; // BTN1 (0 Ом)
    if (adc_raw >= 500 && adc_raw < 1000)
        return 2; // BTN2 (2.2к)
    if (adc_raw >= 1200 && adc_raw < 1700)
        return 3; // BTN3 (5.5к)
    if (adc_raw >= 2000 && adc_raw < 2500)
        return 4; // BTN4 (12.3к)
    if (adc_raw >= 2700 && adc_raw < 3200)
        return 5; // BTN5 (27.3к)

    return 0; // Какая-то дикая помеха
}

void link_buttons_polling_task(void *pvParameters)
{
    // 1. Инициализация юнита АЦП (ADC1)
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = BUTTONS_ADC_UNIT,
        .clk_src = ADC_DIGI_CLK_SRC_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    // 2. Конфигурация канала (12-битное разрешение и максимальное ослабление)
    // В ESP-IDF v5.x вместо 11dB используется макрос ADC_ATTEN_DB_12 (для замера до ~2.8-3.3В)
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT, // Для ESP32-C3 это жестко 12 бит (0..4095)
        .atten = ADC_ATTEN_DB_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, BUTTON_ADC_CHANNEL, &config));

    int last_stable_button = 0;

    ESP_LOGI(TAG, "Задача опроса кнопок успешно запущена.");

    // 3. Бесконечный цикл опроса
    while (1)
    {
        int adc_raw = 0;
        // Читаем сырое значение с пина GPIO1
        esp_err_t r = adc_oneshot_read(adc1_handle, BUTTON_ADC_CHANNEL, &adc_raw);

        if (r == ESP_OK)
        {
            uint8_t current_button = get_button_from_adc(adc_raw);

            // Если состояние изменилось
            if (current_button != last_stable_button)
            {
                if (current_button != 0)
                {
                    ESP_LOGI(TAG, "Нажата кнопка [%d] (ADC: %d)", current_button, adc_raw);

                    // Сюда вызовы функций/событий xQueueSend или обработчиков
                }
                else
                {
                    ESP_LOGI(TAG, "Кнопка [%d] отпущена", last_stable_button);
                }
                last_stable_button = current_button;
            }
        }
        else
        {
            ESP_LOGE(TAG, "Ошибка чтения АЦП");
        }

        // Блокируем таску ровно на 20 мс, отдавая процессорное время другим задачам (LoRa, LED)
        vTaskDelay(pdMS_TO_TICKS(BUTTON_POLL_INTERVAL_MS));
    }

    // Чистим за собой ресурсы, если таска когда-либо завершится (опционально)
    adc_oneshot_del_unit(adc1_handle);
    vTaskDelete(NULL);
}

void link_buttons_init(gpio_num_t gpio_num)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << gpio_num),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&io_conf);

    xTaskCreatePinnedToCore(link_buttons_polling_task, "link_buttons_polling_task", 3072, NULL, 5, NULL, 0);
}