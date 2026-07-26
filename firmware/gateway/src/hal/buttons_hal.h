#ifndef BUTTONS_HAL_H
#define BUTTONS_HAL_H

#include <stdint.h>

/**
 * @brief Buttons hardware abstraction — pure ADC reading, no business logic.
 *        Returns 1..5 for physical buttons, 0 for none.
 */

void buttons_hal_init(void);
uint8_t buttons_hal_read(void); // 0 = none, 1-5 = button pressed

#endif // BUTTONS_HAL_H