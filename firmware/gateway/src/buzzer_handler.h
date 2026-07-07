#ifndef BUZZER_HANDLER_H
#define BUZZER_HANDLER_H

#include "driver/gpio.h"

void buzzer_init(void);
void buzz(uint16_t seconds);

#endif