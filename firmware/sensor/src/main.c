#include "buzzer_handler.h"
#include "led_handler.h"
#include "link_button_handle.h"
#include "water_sensor_handler.h"
#include "lora_handler.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gpio_mapping.h"

void app_main(void)
{
    // Будим USB-порт
    vTaskDelay(pdMS_TO_TICKS(2000));

     if (!lora_hw_init())
    {
        ESP_LOGE("MAIN", "Ошибка старта LoRa!");
        return;
    }

    buzzer_init();
    led_init();
    link_button_init();
    water_sensor_init();

    ESP_LOGI("MAIN", "Датчик Sentry узел успешно запущен в работу.");
}