#ifndef LINK_BUTTON_HANDLE_H
#define LINK_BUTTON_HANDLE_H

#include "driver/gpio.h"
#include <stdint.h>

/**
 * @brief Инициализация кнопки привязки на датчике
 * @param gpio_num Номер GPIO, к которому подключена физическая кнопка
 */
void link_button_init(gpio_num_t gpio_num);

/**
 * @brief Генерация уникального 32-битного ID устройства на основе eFuse MAC-адреса
 * @return uint32_t Уникальный ID
 */
uint32_t generate_unique_id(void);

#endif // LINK_BUTTON_HANDLE_H