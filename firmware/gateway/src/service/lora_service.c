#include "lora_service.h"
#include "hal/lora_hal.h"
#include "event_bus.h"
#include "protocol.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"

static const char *TAG = "LORA_SVC";

static void post_lora_event(uint8_t mac_addr, uint8_t status)
{
    event_t ev = {
        .type = EVENT_LORA_PACKET_RX,
        .mac_addr = mac_addr,
        .packet_type = status};
    event_bus_post(&ev);
}

static void lora_rx_task(void *pvParameters)
{
    esp_task_wdt_add(NULL);

    while (1)
    {
        esp_task_wdt_reset();

        lora_rx_packet_t packet;
        if (lora_hal_receive(&packet))
        {
            uint8_t status = packet.payload.status;

            ESP_LOGI(TAG, "==================================================");
            ESP_LOGI(TAG, "RX PACKET | Sensor: 0x%02X | Seq: %d",
                     packet.payload.mac_addr, packet.payload.packet_id);
            ESP_LOGI(TAG, "Status: 0x%02X | RSSI: %d dBm | SNR: %.1f dB | Batt: %d mV",
                     status, packet.rssi, packet.snr, packet.payload.battery_mv);

            // Post event for each flag (non-exclusive — all flags are processed)
            if (status & STATUS_BIT_ALARM_WATER)
            {
                ESP_LOGE(TAG, "WATER ALARM from sensor 0x%02X!", packet.payload.mac_addr);
                post_lora_event(packet.payload.mac_addr, status);
            }

            if (status & STATUS_BIT_PAIRING_MODE)
            {
                ESP_LOGW(TAG, "Pairing request from sensor 0x%02X.", packet.payload.mac_addr);
                post_lora_event(packet.payload.mac_addr, status);
            }

            if (status & STATUS_BIT_PING)
            {
                ESP_LOGI(TAG, "Heartbeat from sensor 0x%02X.", packet.payload.mac_addr);
                post_lora_event(packet.payload.mac_addr, status);
            }

            if (!(status & (STATUS_BIT_ALARM_WATER | STATUS_BIT_PAIRING_MODE | STATUS_BIT_PING)))
            {
                ESP_LOGW(TAG, "Unknown status: 0x%02X", status);
            }

            ESP_LOGI(TAG, "==================================================");
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void lora_service_init(void)
{
    if (!lora_hal_init())
    {
        ESP_LOGE(TAG, "LoRa HAL init failed! Service not started.");
        return;
    }

    xTaskCreate(lora_rx_task, "lora_rx_task", 3072, NULL, 5, NULL);
    ESP_LOGI(TAG, "LoRa service started.");
}