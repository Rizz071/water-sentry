#ifndef NVS_HAL_H
#define NVS_HAL_H

#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"

/**
 * @brief NVS (Non-Volatile Storage) hardware abstraction.
 *        Pure flash read/write, no business logic.
 */

void nvs_hal_init(void);
esp_err_t nvs_hal_save_blob(const char *ns, const char *key, const void *data, size_t size);
esp_err_t nvs_hal_load_blob(const char *ns, const char *key, void *out_data, size_t *size);

#endif // NVS_HAL_H