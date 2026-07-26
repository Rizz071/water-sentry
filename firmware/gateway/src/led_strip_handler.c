#include "led_strip_handler.h"
#include "gpio_mapping.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_strip.h"

static const char *TAG = "LED_STRIPE";

void rgb_led_strip_clear(struct rgb_led_strip_handle_t *led_strip_handle)
{
    led_strip_clear(led_strip_handle->led_strip_hw_handle);
}

void rgb_led_strip_update(struct rgb_led_strip_handle_t *led_strip_handle)
{
    uint8_t leds_amount = sizeof(led_strip_handle->rgb_led) / sizeof(led_strip_handle->rgb_led[0]);

    led_strip_clear(led_strip_handle->led_strip_hw_handle);

    for (int i = 0; i <= leds_amount - 1; i++)
    {
        led_strip_set_pixel(
            led_strip_handle->led_strip_hw_handle,
            i,
            led_strip_handle->rgb_led[i].red,
            led_strip_handle->rgb_led[i].green,
            led_strip_handle->rgb_led[i].blue);
    }

    led_strip_refresh(led_strip_handle->led_strip_hw_handle);
}

void rgb_led_strip_init(struct rgb_led_strip_handle_t *led_strip_handle, gpio_num_t gpio_num, uint8_t max_leds_in_strip)
{
    ESP_LOGI(TAG, "Конфигурация lED STRIPE...");

    led_strip_handle->gpio_num = gpio_num;

    // 1. Конфигурация светодиодной ленты/цепочки
    led_strip_config_t strip_config = {
        .strip_gpio_num = led_strip_handle->gpio_num,
        .max_leds = max_leds_in_strip,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_RGB, // APA106/WS2812 совместимы
        .led_model = LED_MODEL_WS2812,                               // Тайминги WS2812 отлично подходят для APA106
        .flags.invert_out = false,
    };

    // 2. Конфигурация аппаратного модуля RMT
    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000, // 10 МГц (разрешение 100 нс)
        .flags.with_dma = false,           // Для 5 диодов DMA не требуется
    };

    // 3. Создаем драйвер RMT
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip_handle->led_strip_hw_handle));

    for (int i = 0; i <= 4; i++)
    {
        led_strip_handle->rgb_led[i].red = 0;
        led_strip_handle->rgb_led[i].green = 255;
        led_strip_handle->rgb_led[i].blue = 0;
    }

    rgb_led_strip_update(led_strip_handle);

    vTaskDelay(pdMS_TO_TICKS(500));

    rgb_led_strip_clear(led_strip_handle);

    ESP_LOGI(TAG, "...LED SRIPE инициализирована.");
}