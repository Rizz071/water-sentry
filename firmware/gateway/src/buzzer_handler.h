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
    int buzzer_pin;
    volatile enum buzzer_state_t current_state;
};


void buzzer_init(struct buzzer_t* new_buzzer, int new_buzzer_pin);
void buzzer_set_state(struct buzzer_t* buzzer, enum buzzer_state_t new_state);

#endif