#include "sensor_fsm.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "SENSOR_FSM";

/*
 * Valid transition matrix:
 *   SLOT_EMPTY   -> PAIRING
 *   SLOT_PAIRING -> OK, EMPTY (timeout)
 *   SLOT_OK      -> ALARM, OFFLINE, PAIRING
 *   SLOT_ALARM   -> OK, OFFLINE
 *   SLOT_OFFLINE -> OK, EMPTY
 */

static bool is_valid_transition(slot_state_t from, slot_state_t to)
{
    switch (from)
    {
    case SLOT_EMPTY:
        return (to == SLOT_PAIRING);
    case SLOT_PAIRING:
        return (to == SLOT_OK || to == SLOT_EMPTY);
    case SLOT_OK:
        return (to == SLOT_ALARM || to == SLOT_OFFLINE || to == SLOT_PAIRING);
    case SLOT_ALARM:
        return (to == SLOT_OK || to == SLOT_OFFLINE);
    case SLOT_OFFLINE:
        return (to == SLOT_OK || to == SLOT_EMPTY);
    default:
        return false;
    }
}

static const char *state_name(slot_state_t s)
{
    switch (s)
    {
    case SLOT_EMPTY:
        return "EMPTY";
    case SLOT_PAIRING:
        return "PAIRING";
    case SLOT_OK:
        return "OK";
    case SLOT_OFFLINE:
        return "OFFLINE";
    case SLOT_ALARM:
        return "ALARM";
    default:
        return "?";
    }
}

static void set_state(sensor_slot_t *slot, slot_state_t new_state)
{
    if (slot->state == new_state)
        return;

    if (!is_valid_transition(slot->state, new_state))
    {
        ESP_LOGW(TAG, "Invalid transition: %s -> %s for MAC 0x%02X",
                 state_name(slot->state), state_name(new_state), slot->mac_addr);
        return;
    }

    ESP_LOGI(TAG, "Slot MAC 0x%02X: %s -> %s",
             slot->mac_addr, state_name(slot->state), state_name(new_state));

    // Reset offline_acked on any state change involving OFFLINE
    if (slot->state == SLOT_OFFLINE || new_state == SLOT_OFFLINE)
    {
        slot->offline_acked = false;
    }

    slot->state = new_state;
}

void sensor_fsm_init(sensor_slot_t *slots, const uint8_t *mac_list, size_t count)
{
    for (size_t i = 0; i < count; i++)
    {
        slots[i].mac_addr = mac_list[i];
        slots[i].pairing_start_ms = 0;

        if (mac_list[i] != 0)
        {
            slots[i].state = SLOT_OK;
            slots[i].last_seen_ms = 0;
        }
        else
        {
            slots[i].state = SLOT_EMPTY;
            slots[i].last_seen_ms = 0;
        }
    }

    ESP_LOGI(TAG, "FSM initialized with %d slots.", (int)count);
}

bool sensor_fsm_start_pairing(sensor_slot_t *slots, size_t slot_index)
{
    if (slot_index >= MAX_SENSORS)
        return false;

    // If pairing on other slots, cancel them
    for (size_t i = 0; i < MAX_SENSORS; i++)
    {
        if (i == slot_index)
            continue;

        if (slots[i].state == SLOT_PAIRING)
        {
            ESP_LOGI(TAG, "Pairing cancelled on slot %d — clearing previous binding.", (int)i);
            slots[i].mac_addr = 0;
            slots[i].pairing_start_ms = 0;
            slots[i].offline_acked = false;
            set_state(&slots[i], SLOT_EMPTY);
        }
    }

    // Toggle: if already pairing, cancel and clear the slot
    if (slots[slot_index].state == SLOT_PAIRING)
    {
        ESP_LOGI(TAG, "Pairing cancelled on slot %d — clearing previous binding.", (int)slot_index);
        slots[slot_index].mac_addr = 0;
        slots[slot_index].pairing_start_ms = 0;
        slots[slot_index].offline_acked = false;
        set_state(&slots[slot_index], SLOT_EMPTY);
        return false; // false = cancelled (caller should save NVS)
    }

    if (!is_valid_transition(slots[slot_index].state, SLOT_PAIRING))
    {
        ESP_LOGW(TAG, "Cannot start pairing on slot %d (state=%s).",
                 (int)slot_index, state_name(slots[slot_index].state));
        return false;
    }

    set_state(&slots[slot_index], SLOT_PAIRING);
    slots[slot_index].pairing_start_ms = xTaskGetTickCount();
    return true; // true = pairing started
}

bool sensor_fsm_pair(sensor_slot_t *slots, size_t count, uint8_t mac_addr)
{
    for (size_t i = 0; i < count; i++)
    {
        if (slots[i].state == SLOT_PAIRING)
        {
            slots[i].mac_addr = mac_addr;
            set_state(&slots[i], SLOT_OK);
            slots[i].last_seen_ms = xTaskGetTickCount();
            ESP_LOGI(TAG, "Sensor 0x%02X paired to slot %d.", mac_addr, (int)i);
            return true;
        }
    }
    return false;
}

void sensor_fsm_update(sensor_slot_t *slots, size_t count, uint8_t mac_addr, bool is_alarm)
{
    for (size_t i = 0; i < count; i++)
    {
        if (slots[i].state != SLOT_EMPTY && slots[i].mac_addr == mac_addr)
        {
            slots[i].last_seen_ms = xTaskGetTickCount();

            if (is_alarm)
            {
                set_state(&slots[i], SLOT_ALARM);
            }
            else
            {
                slots[i].last_ping_ms = xTaskGetTickCount();
                set_state(&slots[i], SLOT_OK);
            }
            return;
        }
    }
}

bool sensor_fsm_acknowledge_offline(sensor_slot_t *slots, size_t slot_index)
{
    if (slot_index >= MAX_SENSORS)
        return false;

    if (slots[slot_index].state != SLOT_OFFLINE)
        return false;

    if (slots[slot_index].offline_acked)
        return false; // Already acknowledged

    slots[slot_index].offline_acked = true;
    ESP_LOGI(TAG, "Slot %d offline buzzer acknowledged by user.", (int)slot_index);
    return true;
}

bool sensor_fsm_check_timeouts(sensor_slot_t *slots, size_t count, uint32_t now_ms)
{
    bool slot_cleared = false;

    for (size_t i = 0; i < count; i++)
    {
        // Pairing timeout
        if (slots[i].state == SLOT_PAIRING && slots[i].pairing_start_ms != 0)
        {
            if (now_ms - slots[i].pairing_start_ms > pdMS_TO_TICKS(PAIRING_TIMEOUT_MS))
            {
                ESP_LOGI(TAG, "Pairing timeout on slot %d — clearing previous binding.", (int)i);
                slots[i].mac_addr = 0;
                slots[i].pairing_start_ms = 0;
                slots[i].offline_acked = false;
                set_state(&slots[i], SLOT_EMPTY);
                slot_cleared = true;
            }
        }

        // Offline detection
        if (slots[i].state == SLOT_OK || slots[i].state == SLOT_ALARM)
        {
            if (slots[i].last_seen_ms != 0 &&
                now_ms - slots[i].last_seen_ms > pdMS_TO_TICKS(SENSOR_TIMEOUT_MS))
            {
                set_state(&slots[i], SLOT_OFFLINE);
            }
        }
    }

    return slot_cleared;
}
