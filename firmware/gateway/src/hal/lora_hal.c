#include "lora_hal.h"
#include "lora.h"
#include "esp_log.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "LORA_HAL";

bool lora_hal_init(void)
{
    ESP_LOGI(TAG, "Initializing SX1278 (Ra-02) radio...");

    if (lora_init() == 0)
    {
        ESP_LOGE(TAG, "Critical: SX1278 chip not responding on SPI!");
        return false;
    }

    lora_set_frequency(LORA_FREQ);
    lora_set_spreading_factor(LORA_SPREADING_FACTOR);
    lora_set_bandwidth(LORA_BANDWITH);
    lora_enable_crc();

    lora_receive();

    ESP_LOGI(TAG, "SX1278 initialized and listening.");
    return true;
}

bool lora_hal_receive(lora_rx_packet_t *out_packet)
{
    uint8_t rx_buffer[sizeof(lora_payload_t)];

    if (!lora_received())
        return false;

    int packet_len = lora_receive_packet(rx_buffer, sizeof(rx_buffer));

    if (packet_len != sizeof(lora_payload_t))
    {
        ESP_LOGW(TAG, "Garbage in air: packet length %d != expected %d",
                 packet_len, (int)sizeof(lora_payload_t));
        lora_receive();
        return false;
    }

    memcpy(&out_packet->payload, rx_buffer, sizeof(lora_payload_t));
    out_packet->rssi = lora_packet_rssi();
    out_packet->snr = lora_packet_snr();

    lora_receive();
    return true;
}