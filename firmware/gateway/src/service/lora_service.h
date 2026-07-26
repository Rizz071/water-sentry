#ifndef LORA_SERVICE_H
#define LORA_SERVICE_H

/**
 * @brief LoRa service — runs a FreeRTOS task that reads raw packets from HAL
 *        and posts them to the event bus with proper classification.
 */

void lora_service_init(void);

#endif // LORA_SERVICE_H