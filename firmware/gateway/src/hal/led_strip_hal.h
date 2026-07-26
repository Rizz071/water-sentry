#ifndef LED_STRIP_HAL_H
#define LED_STRIP_HAL_H

#include "driver/gpio.h"
#include "led_strip.h"
#include "app_config.h"

/**
 * @brief LED strip hardware abstraction — pure RGB control, no business logic.
 */

typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} led_color_t;

typedef struct {
    gpio_num_t gpio_num;
    led_strip_handle_t hw_handle;
    led_color_t leds[MAX_SENSORS];
} led_strip_hal_t;

void led_strip_hal_init(led_strip_hal_t *led_strip, gpio_num_t gpio_num, uint8_t max_leds);
void led_strip_hal_update(led_strip_hal_t *led_strip);
void led_strip_hal_clear(led_strip_hal_t *led_strip);

#endif // LED_STRIP_HAL_H