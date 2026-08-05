#ifndef SENSOR_FSM_H
#define SENSOR_FSM_H

#include <stdint.h>
#include <stdbool.h>
#include "app_config.h"

/**
 * @brief Sensor slot state machine (pure logic, no hardware dependencies).
 */

typedef enum {
    SLOT_EMPTY = 0,
    SLOT_PAIRING,
    SLOT_OK,
    SLOT_OFFLINE,
    SLOT_ALARM,
} slot_state_t;

typedef struct {
    uint8_t mac_addr;
    slot_state_t state;
    uint32_t last_seen_ms;
    uint32_t last_ping_ms;       // Timestamp of last ping (for LED blink)
    uint32_t pairing_start_ms;
    bool offline_acked;          // User acknowledged offline buzzer via button
    bool alarm_acked;
} sensor_slot_t;

void sensor_fsm_init(sensor_slot_t *slots, const uint8_t *mac_list, size_t count);
bool sensor_fsm_start_pairing(sensor_slot_t *slots, size_t slot_index);
bool sensor_fsm_pair(sensor_slot_t *slots, size_t count, uint8_t mac_addr);
void sensor_fsm_update(sensor_slot_t *slots, size_t count, uint8_t mac_addr, bool is_alarm);
bool sensor_fsm_check_timeouts(sensor_slot_t *slots, size_t count, uint32_t now_ms);
bool sensor_fsm_acknowledge_offline(sensor_slot_t *slots, size_t slot_index);
bool sensor_fsm_acknowledge_alarm(sensor_slot_t *slots, size_t slot_index);

#endif // SENSOR_FSM_H
