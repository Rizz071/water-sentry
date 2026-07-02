#pragma once

#include "driver/gpio.h"

void buzzer_init(gpio_num_t sensor_gpio);
void buzz(uint16_t seconds);

