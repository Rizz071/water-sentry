#ifndef BUZZER_HANDLER_H
#define BUZZER_HANDLER_H

#include "driver/gpio.h"

enum buzzer_state_t
{
    BUZZER_SILENCED = 0,
    BUZZER_ALARM,
    BUZZER_WARNING
};

struct buzzer_t
{
    gpio_num_t gpio_num;
    volatile enum buzzer_state_t current_state;
};


void buzzer_init(struct buzzer_t* buzzer, gpio_num_t gpio_num);
void buzzer_set_state(struct buzzer_t* buzzer, enum buzzer_state_t new_state);

#endif