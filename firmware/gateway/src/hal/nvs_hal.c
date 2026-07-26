#include "nvs_hal.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "NVS_HAL";

void nvs_hal_init(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_LOGI(TAG, "NVS erased.");
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    ESP_LOGI(TAG, "NVS initialized.");
}

esp_err_t nvs_hal_save_blob(const char *ns, const char *key, const void *data, size_t size)
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open(ns, NVS_READWRITE, &handle);
    if (err != ESP_OK)
        return err;

    err = nvs_set_blob(handle, key, data, size);
    if (err == ESP_OK)
    {
        err = nvs_commit(handle);
        ESP_LOGI(TAG, "Blob '%s/%s' (%d bytes) saved.", ns, key, (int)size);
    }
    else
    {
        ESP_LOGE(TAG, "Write error: %s", esp_err_to_name(err));
    }

    nvs_close(handle);
    return err;
}

esp_err_t nvs_hal_load_blob(const char *ns, const char *key, void *out_data, size_t *size)
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open(ns, NVS_READONLY, &handle);

    if (err != ESP_OK)
    {
        if (err == ESP_ERR_NVS_NOT_FOUND)
        {
            ESP_LOGW(TAG, "Namespace '%s' not found.", ns);
        }
        return err;
    }

    err = nvs_get_blob(handle, key, out_data, size);

    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "Blob '%s/%s' (%d bytes) loaded.", ns, key, (int)*size);
    }
    else if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        ESP_LOGW(TAG, "Key '%s' not found in namespace '%s'.", key, ns);
    }
    else
    {
        ESP_LOGE(TAG, "Read error: %s", esp_err_to_name(err));
    }

    nvs_close(handle);
    return err;
}