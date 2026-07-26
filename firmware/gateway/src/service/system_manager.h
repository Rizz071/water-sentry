#ifndef SYSTEM_MANAGER_H
#define SYSTEM_MANAGER_H

#include "sensor_fsm.h"

/**
 * @brief System manager — orchestrates the FSM and event bus.
 *        Owns the sensor slots. UI runs independently via ui_controller.
 */

void system_manager_init(sensor_slot_t *slots, size_t count);

#endif // SYSTEM_MANAGER_H