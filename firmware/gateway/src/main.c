#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gpio_mapping.h"
#include "app_config.h"

#include "hal/buzzer_hal.h"
#include "hal/led_strip_hal.h"
#include "hal/nvs_hal.h"
#include "service/event_bus.h"
#include "service/sensor_fsm.h"
#include "service/ui_controller.h"
#include "service/system_manager.h"
#include "service/lora_service.h"
#include "service/buttons_service.h"

static const char *TAG = "MAIN_APP";

void app_main(void)
{
    vTaskDelay(pdMS_TO_TICKS(3000));
    ESP_LOGI(TAG, "Water Sentry Gateway starting...");

    /* ======================================================================
     * LAYER 1: Hardware Abstraction (HAL) — init all peripherals
     * ====================================================================== */

    // NVS flash storage
    nvs_hal_init();

    // Buzzer
    static buzzer_hal_t buzzer;
    buzzer_hal_init(&buzzer, BUZZER_PIN);

    // LED strip
    static led_strip_hal_t led_strip;
    led_strip_hal_init(&led_strip, LED_LINE_PIN, MAX_SENSORS);

    /* ======================================================================
     * LAYER 2: Event bus — the communication backbone
     * ====================================================================== */
    event_bus_init();

    /* ======================================================================
     * LAYER 3: Sensor state machine — load persisted MACs
     * ====================================================================== */
    static sensor_slot_t slots[MAX_SENSORS];

    uint8_t mac_list[MAX_SENSORS];
    size_t mac_size = sizeof(mac_list);
    memset(mac_list, 0, mac_size);

    esp_err_t nvs_err = nvs_hal_load_blob("sensors", "dev_list", mac_list, &mac_size);
    if (nvs_err != ESP_OK)
    {
        ESP_LOGW(TAG, "No saved sensors found, starting with empty slots.");
    }

    for (size_t i = 0; i < MAX_SENSORS; i++)
    {
        ESP_LOGI(TAG, "Loaded MAC[%d]: 0x%02X", (int)i, mac_list[i]);
    }

    sensor_fsm_init(slots, mac_list, MAX_SENSORS);

    /* ======================================================================
     * LAYER 4: System manager — orchestrates FSM
     * ====================================================================== */
    system_manager_init(slots, MAX_SENSORS);

    /* ======================================================================
     * LAYER 4.5: UI controller — independent rendering task
     * ====================================================================== */
    ui_controller_init(&led_strip, &buzzer, slots, MAX_SENSORS);

    /* ======================================================================
     * LAYER 5: Peripheral services — LoRa RX + Buttons polling
     * ====================================================================== */
    lora_service_init();
    buttons_service_init();

    ESP_LOGI(TAG, "Water Sentry Gateway fully operational.");
    ESP_LOGI(TAG, "Monitoring %d sensor slots.", MAX_SENSORS);
}