#ifndef LED_HANDLER_H
#define LED_HANDLER_H

#include "driver/gpio.h"
#include "led_strip.h"

#define LEDS_IN_STRIP 5

struct rgb_led_t
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

struct rgb_led_strip_handle_t
{
    gpio_num_t gpio_num;
    led_strip_handle_t led_strip_hw_handle;
    struct rgb_led_t rgb_led[LEDS_IN_STRIP];
};

void rgb_led_strip_init(struct rgb_led_strip_handle_t *led_strip_handle, gpio_num_t gpio_num, uint8_t max_leds_in_strip);
void rgb_led_strip_update(struct rgb_led_strip_handle_t *led_strip_handle);
void rgb_led_strip_clear(struct rgb_led_strip_handle_t *led_strip_handle);

#endif
