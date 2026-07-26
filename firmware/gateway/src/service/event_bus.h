#ifndef EVENT_BUS_H
#define EVENT_BUS_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief System-wide event bus (service layer).
 *        Decouples producers (buttons, LoRa) from consumers (state machine).
 */

typedef enum {
    EVENT_BTN_PAIR_PRESSED,
    EVENT_BTN_LONG_PRESS,
    EVENT_LORA_PACKET_RX,
    EVENT_PERIODIC_TICK,
    EVENT_PAIRING_TIMEOUT,
} event_type_t;

typedef struct {
    event_type_t type;
    uint8_t button_num;
    uint8_t mac_addr;
    uint8_t packet_type;
} event_t;

void event_bus_init(void);
bool event_bus_post(const event_t *ev);
bool event_bus_receive(event_t *ev, uint32_t timeout_ms);

#endif // EVENT_BUS_H