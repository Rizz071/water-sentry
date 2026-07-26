#ifndef SYSTEM_EVENTS_H
#define SYSTEM_EVENTS_H

#include "slots_handler.h"

// 1. Типы событий
enum event_type_t
{
    EVENT_BTN_PAIR_PRESSED, // Нажали одну из 5 кнопок привязки
    EVENT_LORA_PACKET_RX,   // Пришел пакет по радио
};

// 2. Структура события
struct system_event_t
{
    enum event_type_t type;
    uint8_t button_num;
    uint8_t mac_addr;
    uint8_t packet_type;
};

// 3. API модуля событий
void system_events_init(void);
bool system_event_post(const struct system_event_t *ev);
bool system_event_receive(struct system_event_t *ev, uint32_t wait_ms);

#endif