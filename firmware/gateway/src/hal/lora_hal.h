#ifndef LORA_HAL_H
#define LORA_HAL_H

#include <stdint.h>
#include <stdbool.h>
#include "protocol.h"

/**
 * @brief LoRa radio hardware abstraction — pure SX1278 control, no business logic.
 */

typedef struct {
    lora_payload_t payload;
    int rssi;
    float snr;
} lora_rx_packet_t;

bool lora_hal_init(void);
bool lora_hal_receive(lora_rx_packet_t *out_packet);

#endif // LORA_HAL_H