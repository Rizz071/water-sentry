#ifndef LED_HANDLER_H
#define LED_HANDLER_H

#include "driver/gpio.h"

void led_init();
void led_light(uint8_t seconds);

#endif