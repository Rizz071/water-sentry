#include "nvs_storage_handler.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "SENSORS_NVS";

#define NVS_NAMESPACE "sensors"
#define NVS_KEY_LIST "dev_list"

void nvs_storage_init(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_LOGI(TAG, "NVS очищена.");
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    ESP_LOGI(TAG, "NVS успешно инициализирована");
}

esp_err_t nvs_storage_save_sensors(const uint8_t *sensors, size_t count)
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK)
        return err;

    size_t required_bytes = count * sizeof(uint8_t);

    err = nvs_set_blob(handle, NVS_KEY_LIST, sensors, required_bytes);
    if (err == ESP_OK)
    {
        err = nvs_commit(handle);
        ESP_LOGI(TAG, "Массив из %d байт успешно сохранен!", (int)count);
    }
    else
    {
        ESP_LOGE(TAG, "Ошибка записи в NVS: %s", esp_err_to_name(err));
    }

    nvs_close(handle);
    return err;
}

esp_err_t nvs_storage_load_sensors(uint8_t *sensors, size_t count)
{

    nvs_storage_init();

    // 1. СРАЗУ защищаемся от мусора: забиваем весь буфер нулями
    memset(sensors, 0, count * sizeof(uint8_t));

    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);

    // Если пространства имен нет — это нормально для первого запуска.
    // Буфер уже занулен, просто возвращаем статус "не найдено".
    if (err != ESP_OK)
    {
        if (err == ESP_ERR_NVS_NOT_FOUND)
        {
            ESP_LOGW(TAG, "Пространство имен NVS не найдено. Данные обнулены.");
        }
        return err;
    }

    size_t required_bytes = count * sizeof(uint8_t);

    // 2. Пробуем прочитать данные
    err = nvs_get_blob(handle, NVS_KEY_LIST, sensors, &required_bytes);

    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "Массив из %d байт успешно прочитан из NVS!", (int)count);
    }
    else if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        ESP_LOGW(TAG, "Ключ датчиков не найден в NVS. Используем нулевой список.");
    }
    else
    {
        ESP_LOGE(TAG, "Ошибка чтения NVS: %s", esp_err_to_name(err));
    }

    nvs_close(handle);
    return err;
}
