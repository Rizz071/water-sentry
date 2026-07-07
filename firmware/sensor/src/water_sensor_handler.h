#ifndef WATER_SENSOR_HANDLER_H
#define WATER_SENSOR_HANDLER_H

#include "driver/gpio.h"

/**
 * @brief Инициализация датчика протечки и запуск задачи мониторинга во FreeRTOS
 */
void water_sensor_init();

#endif // WATER_SENSOR_HANDLER_H