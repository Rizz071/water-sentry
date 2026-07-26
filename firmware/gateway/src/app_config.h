#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/**
 * @brief Unified project configuration.
 *        All magic numbers live here — no duplicates across modules.
 */

#define MAX_SENSORS        5    // Total sensor slots
#define EVENT_QUEUE_LENGTH 10   // System event queue capacity
#define LORA_RX_QUEUE_LENGTH 20 // LoRa RX packet queue capacity

#define BUTTON_POLL_MS      20  // ADC polling interval
#define BUTTON_DEBOUNCE_CNT 3   // Consecutive reads to confirm press/release

#define SENSOR_TIMEOUT_MS   30000 // 30 s — sensor considered offline
#define PAIRING_TIMEOUT_MS  60000 // 60 s — pairing mode auto-cancel

#endif // APP_CONFIG_H