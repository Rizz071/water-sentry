#include "buzzer_handler.h"
#include "led_handler.h"
#include "link_button_handle.h"
#include "water_sensor_handler.h"
#include "lora_handler.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void)
{
    // Будим USB-порт
    vTaskDelay(pdMS_TO_TICKS(2000));

     if (!lora_handler_init())
    {
        ESP_LOGE("MAIN", "Ошибка старта LoRa!");
        return;
    }

    buzzer_init(GPIO_NUM_0);
    led_init(GPIO_NUM_8);
    link_button_init(GPIO_NUM_9);
    water_sensor_init(GPIO_NUM_1);

    ESP_LOGI("MAIN", "Датчик Sentry узел успешно запущен в работу.");
}