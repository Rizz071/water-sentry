#ifndef BUZZER_HAL_H
#define BUZZER_HAL_H

#include "driver/gpio.h"

/**
 * @brief Buzzer hardware abstraction — pure GPIO control, no business logic.
 */

typedef enum {
    BUZZER_STATE_SILENCED = 0,
    BUZZER_STATE_ALARM,
    BUZZER_STATE_WARNING
} buzzer_state_t;

typedef struct {
    gpio_num_t gpio_num;
    volatile buzzer_state_t current_state;
} buzzer_hal_t;

void buzzer_hal_init(buzzer_hal_t *buzzer, gpio_num_t gpio_num);
void buzzer_hal_set_state(buzzer_hal_t *buzzer, buzzer_state_t new_state);

#endif // BUZZER_HAL_H