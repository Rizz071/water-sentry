#include "system_manager.h"
#include "event_bus.h"
#include "sensor_fsm.h"
#include "hal/nvs_hal.h"
#include "protocol.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"
#include <string.h>

static const char *TAG = "SYS_MGR";

#define NVS_NS "sensors"
#define NVS_KEY "dev_list"

static sensor_slot_t *g_slots = NULL;
static size_t g_slot_count = 0;

static void save_sensors_to_nvs(void)
{
    uint8_t mac_list[MAX_SENSORS] = {0};
    for (size_t i = 0; i < g_slot_count; i++)
    {
        if (g_slots[i].state != SLOT_EMPTY)
        {
            mac_list[i] = g_slots[i].mac_addr;
        }
    }
    nvs_hal_save_blob(NVS_NS, NVS_KEY, mac_list, sizeof(mac_list));
}

static void process_event(const event_t *ev)
{
    switch (ev->type)
    {
    case EVENT_BTN_PAIR_PRESSED:
        if (ev->button_num >= 1 && ev->button_num <= MAX_SENSORS)
        {
            size_t idx = (size_t)(ev->button_num - 1);

            switch (g_slots[idx].state)
            {
            case SLOT_EMPTY:
                // Start pairing on empty slot
                sensor_fsm_start_pairing(g_slots, idx);
                ESP_LOGI(TAG, "Pairing mode activated for slot %d.", ev->button_num);
                break;

            case SLOT_PAIRING:
                // Toggle: cancel pairing, clear slot
                sensor_fsm_start_pairing(g_slots, idx);
                ESP_LOGI(TAG, "Slot %d pairing cancelled. Saving to NVS...", ev->button_num);
                save_sensors_to_nvs();
                break;

            case SLOT_OFFLINE:
                // Acknowledge buzzer
                sensor_fsm_acknowledge_offline(g_slots, idx);
                break;

            case SLOT_OK:
            case SLOT_ALARM:
                // Short press on bound slot — do nothing (use long press to rebind)
                break;

            default:
                break;
            }
        }
        break;

    case EVENT_BTN_LONG_PRESS:
        if (ev->button_num >= 1 && ev->button_num <= MAX_SENSORS)
        {
            size_t idx = (size_t)(ev->button_num - 1);

            // Long press on a bound slot (OK, ALARM, OFFLINE) clears binding and enters pairing
            if (g_slots[idx].state == SLOT_OK ||
                g_slots[idx].state == SLOT_ALARM ||
                g_slots[idx].state == SLOT_OFFLINE)
            {
                ESP_LOGI(TAG, "Long press on slot %d — clearing binding and entering pairing.", ev->button_num);
                g_slots[idx].mac_addr = 0;
                g_slots[idx].offline_acked = false;
                g_slots[idx].pairing_start_ms = xTaskGetTickCount();
                g_slots[idx].state = SLOT_PAIRING;
                save_sensors_to_nvs();
            }
        }
        break;

    case EVENT_LORA_PACKET_RX:
        if (ev->packet_type & STATUS_BIT_PAIRING_MODE)
        {
            if (sensor_fsm_pair(g_slots, g_slot_count, ev->mac_addr))
            {
                ESP_LOGI(TAG, "Sensor 0x%02X paired. Saving to NVS...", ev->mac_addr);
                save_sensors_to_nvs();
            }
        }
        else
        {
            bool is_alarm = (ev->packet_type & STATUS_BIT_ALARM_WATER) != 0;
            sensor_fsm_update(g_slots, g_slot_count, ev->mac_addr, is_alarm);
        }
        break;

    case EVENT_PERIODIC_TICK:
        if (sensor_fsm_check_timeouts(g_slots, g_slot_count, xTaskGetTickCount()))
        {
            save_sensors_to_nvs();
        }
        break;

    case EVENT_PAIRING_TIMEOUT:
        if (sensor_fsm_check_timeouts(g_slots, g_slot_count, xTaskGetTickCount()))
        {
            save_sensors_to_nvs();
        }
        break;

    default:
        break;
    }
}

static void system_manager_task(void *pvParameters)
{
    event_t ev;

    esp_task_wdt_add(NULL);

    while (1)
    {
        esp_task_wdt_reset();

        if (event_bus_receive(&ev, pdMS_TO_TICKS(1000)))
        {
            process_event(&ev);
        }
        else
        {
            // Timeout — periodic tick for offline detection
            event_t tick_ev = {.type = EVENT_PERIODIC_TICK};
            process_event(&tick_ev);
        }
    }
}

void system_manager_init(sensor_slot_t *slots, size_t count)
{
    g_slots = slots;
    g_slot_count = count;

    xTaskCreate(system_manager_task, "sys_manager_task", 4096, NULL, 5, NULL);
    ESP_LOGI(TAG, "System manager started.");
}
