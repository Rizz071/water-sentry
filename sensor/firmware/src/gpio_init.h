#pragma once

#include "driver/gpio.h"


// Определяем пины платы
#define LED_PIN              GPIO_NUM_8  
#define BUZZER_PIN           GPIO_NUM_0  
#define LINK_BUTTON_PIN      GPIO_NUM_9   

void board_gpio_init(void);

