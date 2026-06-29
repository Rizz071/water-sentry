#ifndef WATER_SENSOR_HANDLER_H
#define WATER_SENSOR_HANDLER_H

#include "driver/gpio.h"

/**
 * @brief Инициализация датчика протечки и запуск задачи мониторинга во FreeRTOS
 * @param sensor_gpio GPIO пин, к которому подключены контакты датчика воды
 */
void water_sensor_init(gpio_num_t sensor_gpio);

#endif // WATER_SENSOR_HANDLER_H