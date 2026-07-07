#ifndef HW_ID_H
#define HW_ID_H

#include "driver/gpio.h"

/**
 * @brief Генерация уникального 32-битного ID устройства на основе eFuse MAC-адреса
 * @return uint32_t Уникальный ID
 */
uint32_t get_unique_id(void);

#endif