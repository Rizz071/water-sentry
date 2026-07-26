#ifndef NVS_STORAGE_HANDLER_H
#define NVS_STORAGE_HANDLER_H

#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"

void nvs_storage_init(void);
esp_err_t nvs_storage_save_sensors(const uint8_t *sensors, size_t count);
esp_err_t nvs_storage_load_sensors(uint8_t *sensors, size_t count);

#endif