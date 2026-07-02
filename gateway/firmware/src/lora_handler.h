#ifndef LORA_HANDLER_H
#define LORA_HANDLER_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Наша кастомная, пошаговая инициализация SX1278
 */
bool lora_hardware_init_custom(void);

/**
 * @brief Полная инициализация чипа SX1278 (Ra-02) на максимальную дальность
 * @return true если чип успешно найден и настроен, false при ошибке
 */
bool lora_handler_init(void);

/**
 * @brief Отправка специального пакета для привязки датчика к базовой станции
 */
void lora_send_binding_packet(uint32_t unique_id);

/**
 * @brief Отправка тестового пакета тревоги (имитация протечки)
 */
void lora_send_test_alarm_packet(uint32_t unique_id);

/**
 * @brief Отправка штатного периодического пакета (Heartbeat)
 */
void lora_send_ping_packet(uint32_t unique_id);

/**
 * @brief Отправка реального пакета тревоги при обнаружении воды
 */
void lora_send_real_alarm_packet(uint32_t unique_id);

#endif // LORA_HANDLER_H