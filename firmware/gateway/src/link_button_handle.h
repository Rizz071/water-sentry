#ifndef LINK_BUTTON_HANDLE_H
#define LINK_BUTTON_HANDLE_H

#include "driver/gpio.h"
#include <stdint.h>

/**
 * @brief Инициализация кнопки привязки на датчике
 * @param gpio_num Номер GPIO, к которому подключена физическая кнопка
 */
void link_buttons_init(gpio_num_t gpio_num);

void link_buttons_polling_task(void *pvParameters);

#endif // LINK_BUTTON_HANDLE_H