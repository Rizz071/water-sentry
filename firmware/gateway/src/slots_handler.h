#ifndef SLOTS_HANDLER_H
#define SLOTS_HANDLER_H

#include <stdint.h>
#include "led_strip_handler.h"
#include "buzzer_handler.h"
#include "protocol.h"

enum slot_state_t
{
    SLOT_EMPTY,   // 0. Не привязан (LED: Выключен)
    SLOT_PAIRING, // 1. Ждем пакет для привязки (LED: Синий мигает)
    SLOT_OK,      // 2. Привязан, всё отлично (LED: Зеленый)
    SLOT_OFFLINE, // 3. Потеряна связь (LED: Желтый)
    SLOT_ALARM    // 4. ПРОТЕЧКА! (LED: Красный мигает)
};

struct sensor_slot_t
{
    uint8_t mac_addr;
    enum slot_state_t state;
    uint32_t last_seen_ms; // Время последнего пакета
};

// void remote_sensor_init(struct remote_sensor_t *sensor);
// void remote_sensor_set_state(struct remote_sensor_t *sensor, enum remote_sensor_state_t state);

#endif