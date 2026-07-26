#ifndef UI_CONTROLLER_H
#define UI_CONTROLLER_H

#include "sensor_fsm.h"
#include "hal/led_strip_hal.h"
#include "hal/buzzer_hal.h"

/**
 * @brief UI controller — runs a FreeRTOS task that renders LED colors and buzzer patterns.
 *        LED blinks briefly (100 ms off) when a ping is received from a SLOT_OK sensor.
 */

void ui_controller_init(led_strip_hal_t *led_strip, buzzer_hal_t *buzzer,
                        const sensor_slot_t *slots, size_t count);

#endif // UI_CONTROLLER_H