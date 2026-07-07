#include "driver/gpio.h"

uint32_t get_unique_id(void)
{
    uint8_t mac[6];
    esp_efuse_mac_get_default(mac);
    return ((uint32_t)mac[2] << 24) | ((uint32_t)mac[3] << 16) | ((uint32_t)mac[4] << 8) | (uint32_t)mac[5];
}