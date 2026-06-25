#pragma once

#include <stdint.h>
#include <stdbool.h>

// Инициализация модуля (частота, SF и CRC настраиваются внутри)
bool board_lora_init(uint8_t node_id);

// Функция отправки нашей 6-байтовой упакованной структуры
void send_lora_data(bool is_alarm_triggered, bool liquid_detected, uint16_t battery_voltage);