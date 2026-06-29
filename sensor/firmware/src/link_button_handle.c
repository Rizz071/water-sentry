#include "link_button_handle.h"
#include "lora_handler.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_mac.h"

static const char *TAG = "BUTTON_CTRL";

#define BUTTON_POLL_PERIOD_MS 50   // Частота опроса кнопки (50 мс)
#define HOLD_TIME_LONG_MS 5000     // Время для длинного нажатия (5 секунд)
#define HOLD_TIME_SHORT_MIN_MS 100 // Минимальное время для фиксации клика (100 мс)

#define TICKS_TO_LONG (HOLD_TIME_LONG_MS / BUTTON_POLL_PERIOD_MS)           // 100 тиков
#define TICKS_TO_SHORT_MIN (HOLD_TIME_SHORT_MIN_MS / BUTTON_POLL_PERIOD_MS) // 2 тика

static void link_button_task(void *pvParameters)
{
    gpio_num_t btn_gpio = (gpio_num_t)pvParameters;
    uint32_t press_ticks = 0;
    bool long_press_triggered = false;

    ESP_LOGI(TAG, "Мониторинг кнопки (Тест/Привязка) запущен на GPIO %d", btn_gpio);

    while (1)
    {
        // Кнопка нажата (уровень 0 благодаря подтяжке и RC-фильтру)
        if (gpio_get_level(btn_gpio) == 0)
        {
            press_ticks++;

            // Проверяем удержание на длинное нажатие (5 секунд)
            if (press_ticks >= TICKS_TO_LONG && !long_press_triggered)
            {
                ESP_LOGW(TAG, "Кнопка удержана 5 секунд! Запуск привязки...");

                uint32_t my_unique_id = generate_unique_id();
                lora_send_binding_packet(my_unique_id);

                long_press_triggered = true; // Блокируем повторный выстрел, пока кнопку держат
            }
        }
        // Кнопку отпустили (уровень 1)
        else
        {
            if (press_ticks > 0)
            {
                // Если кнопку отпустили ДО того, как исполнилось 5 секунд,
                // но удерживали дольше минимального порога (защита от случайных микро-касаний)
                if (!long_press_triggered && press_ticks >= TICKS_TO_SHORT_MIN)
                {
                    ESP_LOGI(TAG, "Краткое нажатие зафиксировано (%d мс). Отправка ТЕСТОВОГО перелива...", press_ticks * BUTTON_POLL_PERIOD_MS);

                    uint32_t my_unique_id = generate_unique_id();
                    lora_send_test_alarm_packet(my_unique_id);
                }

                // Скан завершен, сбрасываем состояние для следующего нажатия
                press_ticks = 0;
                long_press_triggered = false;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(BUTTON_POLL_PERIOD_MS));
    }
}

void link_button_init(gpio_num_t gpio_num)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << gpio_num),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&io_conf);

    xTaskCreate(link_button_task, "link_button_task", 2048, (void *)gpio_num, 10, NULL);
}

uint32_t generate_unique_id(void)
{
    uint8_t mac[6];
    esp_efuse_mac_get_default(mac);
    return ((uint32_t)mac[2] << 24) | ((uint32_t)mac[3] << 16) | ((uint32_t)mac[4] << 8) | (uint32_t)mac[5];
}