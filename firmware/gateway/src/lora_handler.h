#ifndef LORA_HANDLER_H
#define LORA_HANDLER_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Полная инициализация чипа SX1278 (Ra-02) на постоянный прием (RX)
 * @return true если чип успешно найден и настроен, false при ошибке
 */
bool lora_hw_init(void);

#endif // LORA_HANDLER_H