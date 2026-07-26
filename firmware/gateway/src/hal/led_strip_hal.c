#include "led_strip_hal.h"
#include "gpio_mapping.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "LED_STRIP_HAL";

void led_strip_hal_clear(led_strip_hal_t *led_strip)
{
    led_strip_clear(led_strip->hw_handle);
}

void led_strip_hal_update(led_strip_hal_t *led_strip)
{
    uint8_t leds_amount = sizeof(led_strip->leds) / sizeof(led_strip->leds[0]);

    led_strip_clear(led_strip->hw_handle);

    for (int i = 0; i < leds_amount; i++)
    {
        led_strip_set_pixel(
            led_strip->hw_handle,
            i,
            led_strip->leds[i].red,
            led_strip->leds[i].green,
            led_strip->leds[i].blue);
    }

    led_strip_refresh(led_strip->hw_handle);
}

void led_strip_hal_init(led_strip_hal_t *led_strip, gpio_num_t gpio_num, uint8_t max_leds)
{
    ESP_LOGI(TAG, "Configuring LED strip hardware...");

    led_strip->gpio_num = gpio_num;

    led_strip_config_t strip_config = {
        .strip_gpio_num = led_strip->gpio_num,
        .max_leds = max_leds,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_RGB,
        .led_model = LED_MODEL_WS2812,
        .flags.invert_out = false,
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000,
        .flags.with_dma = false,
    };

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip->hw_handle));

    for (int i = 0; i < MAX_SENSORS; i++)
    {
        led_strip->leds[i].red = 0;
        led_strip->leds[i].green = 255;
        led_strip->leds[i].blue = 0;
    }

    led_strip_hal_update(led_strip);

    vTaskDelay(pdMS_TO_TICKS(500));

    led_strip_hal_clear(led_strip);

    ESP_LOGI(TAG, "LED strip hardware initialized.");
}