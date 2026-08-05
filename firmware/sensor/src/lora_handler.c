#include "lora_handler.h"
#include "protocol.h"
#include "esp_log.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lora.h"

static const char *TAG = "LORA_MGR";
static uint16_t tx_packet_counter = 0;

static QueueHandle_t lora_tx_queue = NULL;

void lora_tx_task(void *pvParameters);

bool lora_hw_init(void)
{
    ESP_LOGI(TAG, "Запуск инициализации LoRa радиомодуля...");

    // 1. Инициализация SPI и проверка присутствия чипа (пины берутся из menuconfig)
    if (lora_init() == 0)
    {
        ESP_LOGE(TAG, "Критическая ошибка: Чип SX1278 (Ra-02) не откликается по SPI!");
        return false;
    }

    // 2. Накатываем настройки на максимальную пробивную способность
    lora_set_coding_rate(LORA_CODING_RATE);
    lora_set_frequency(LORA_FREQ);                    // Частота 433 МГц (отлично идет сквозь стены)
    lora_set_tx_power(LORA_TX_POWER);                 // Выкручиваем мощность на максимум (+20 dBm / 100 мВт)
    lora_set_spreading_factor(LORA_SPREADING_FACTOR); // Экстремальный фактор расширения спектра SF12
    lora_set_bandwidth(LORA_BANDWITH);                // Узкая полоса 125 кГц для максимальной чувствительности
    lora_enable_crc();                                // Включаем аппаратный контроль целостности

    ESP_LOGI(TAG, "Радиомодуль Ra-02 успешно настроен в режим Extreme Range!");

    lora_tx_queue = xQueueCreate(50, sizeof(lora_payload_t));
    if (lora_tx_queue == NULL)
    {
        ESP_LOGE(TAG, "Не удалось создать очередь LoRa!");
        return false;
    }

    // Запускаем единый таск-отправщик
    xTaskCreate(lora_tx_task, "lora_tx_task", 4096, NULL, 5, NULL);

    return true;
}

void lora_send_binding_packet(uint32_t unique_id)
{
    lora_payload_t packet;
    memset(&packet, 0, sizeof(lora_payload_t));

    packet.mac_addr = unique_id;
    packet.status = STATUS_BIT_PAIRING_MODE;
    packet.battery_mv = 3300;
    packet.packet_id = tx_packet_counter++;

    ESP_LOGW(TAG, ">>> В очередь отправки: пакет ПРИВЯЗКИ: ID=0x%08X, PktSeq=%d",
             packet.mac_addr, packet.packet_id);

    if (xQueueSend(lora_tx_queue, &packet, 0) != pdTRUE)
    {
        ESP_LOGW(TAG, "⚠️ Переполнение очереди LoRa! Пакет пропущен.");
    }
}

void lora_send_test_alarm_packet(uint32_t unique_id)
{
    lora_payload_t packet;
    memset(&packet, 0, sizeof(lora_payload_t));

    packet.mac_addr = unique_id;
    packet.status = STATUS_BIT_INTERRUPT | STATUS_BIT_ALARM_WATER;
    packet.battery_mv = 3300;
    packet.packet_id = tx_packet_counter++;

    ESP_LOGE(TAG, "🚨 >>> В очередь отправки: ТЕСТОВАЯ ТРЕВОГА: ID=0x%08X, PktSeq=%d",
             packet.mac_addr, packet.packet_id);
    if (xQueueSend(lora_tx_queue, &packet, 0) != pdTRUE)
    {
        ESP_LOGW(TAG, "⚠️ Переполнение очереди LoRa! Пакет пропущен.");
    }
}

void lora_send_heartbit_packet(uint32_t unique_id)
{
    lora_payload_t packet;
    memset(&packet, 0, sizeof(lora_payload_t));

    packet.mac_addr = unique_id;
    packet.status = STATUS_BIT_PING;
    packet.battery_mv = 3300; // Тут позже будет реальный замер батареи
    packet.packet_id = tx_packet_counter++;

    ESP_LOGI(TAG, "❤ >>> В очередь отправки: регулярный Heartbeat (ID=0x%08X)", packet.mac_addr);
    if (xQueueSend(lora_tx_queue, &packet, 0) != pdTRUE)
    {
        ESP_LOGW(TAG, "⚠️ Переполнение очереди LoRa! Пакет пропущен.");
    }
}

void lora_send_real_alarm_packet(uint32_t unique_id)
{
    lora_payload_t packet;
    memset(&packet, 0, sizeof(lora_payload_t));

    packet.mac_addr = unique_id;
    // Флаг внеочередного события + реальный флаг аварии:
    packet.status = STATUS_BIT_INTERRUPT | STATUS_BIT_ALARM_WATER;
    packet.battery_mv = 3300;
    packet.packet_id = tx_packet_counter++;

    ESP_LOGE(TAG, "В очередь отправки: 🚨🚨🚨 КРИТИЧЕСКАЯ ТРЕВОГА! (ID=0x%08X)", packet.mac_addr);
    if (xQueueSend(lora_tx_queue, &packet, 0) != pdTRUE)
    {
        ESP_LOGW(TAG, "⚠️ Переполнение очереди LoRa! Пакет пропущен.");
    }
}

void lora_tx_task(void *pvParameters)
{
    lora_payload_t packet_to_send;

    while (1)
    {
        // Ждем, пока в очереди появится пакет на отправку (блокирующий режим, 0% CPU в простое)
        if (xQueueReceive(lora_tx_queue, &packet_to_send, portMAX_DELAY) == pdTRUE)
        {

            ESP_LOGI(TAG, "📤 [TX Task] Отправка пакета #%d (Status: 0x%02X)...",
                     packet_to_send.packet_id, packet_to_send.status);

            // 1. Отправляем в радиоэфир
            lora_send_packet((uint8_t *)&packet_to_send, sizeof(lora_payload_t));

            // 2. ЖЕСТКАЯ ПАУЗА под Time-On-Air для SF12 (4 секунды),
            // чтобы чип гарантированно успел доизлучать пакет до следующей посылки!
            vTaskDelay(pdMS_TO_TICKS(4000));
        }
    }
}