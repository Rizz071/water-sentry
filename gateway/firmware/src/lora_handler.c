#include "lora_handler.h"
#include "protocol.h"
#include "esp_log.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lora.h"

static const char *TAG = "LORA_MGR";
static uint16_t tx_packet_counter = 0;

bool lora_handler_init(void)
{
    ESP_LOGI(TAG, "Запуск инициализации LoRa радиомодуля...");
    

    // 1. Инициализация SPI и проверка присутствия чипа (пины берутся из menuconfig)

    if (lora_init() == 0)
    {
        ESP_LOGE(TAG, "Критическая ошибка: Чип SX1278 (Ra-02) не откликается по SPI!");
        return false;
    }

    // 2. Накатываем настройки на максимальную пробивную способность
    lora_set_frequency(433e6);     // Частота 433 МГц (отлично идет сквозь стены)
    lora_set_tx_power(10);         // Выкручиваем мощность на максимум (+20 dBm / 100 мВт)
    lora_set_spreading_factor(12); // Экстремальный фактор расширения спектра SF12
    lora_set_bandwidth(6);     // Узкая полоса 125 кГц для максимальной чувствительности
    lora_enable_crc();             // Включаем аппаратный контроль целостности

    ESP_LOGI(TAG, "Радиомодуль Ra-02 успешно настроен в режим Extreme Range!");
    return true;
}

void lora_send_binding_packet(uint32_t unique_id)
{
    lora_payload_t packet;
    memset(&packet, 0, sizeof(lora_payload_t));

    packet.node_id = unique_id;
    packet.status = STATUS_BIT_PAIRING_MODE;
    packet.battery_mv = 3300;
    packet.packet_id = tx_packet_counter++;

    ESP_LOGW(TAG, ">>> Отправка пакета ПРИВЯЗКИ: ID=0x%08X, PktSeq=%d",
             packet.node_id, packet.packet_id);

    lora_send_packet((uint8_t *)&packet, sizeof(packet));
}

void lora_send_test_alarm_packet(uint32_t unique_id)
{
    lora_payload_t packet;
    memset(&packet, 0, sizeof(lora_payload_t));

    packet.node_id = unique_id;
    packet.status = STATUS_BIT_INTERRUPT | STATUS_BIT_ALARM_WATER;
    packet.battery_mv = 3300;
    packet.packet_id = tx_packet_counter++;

    ESP_LOGE(TAG, "🚨 >>> Отправка ТЕСТОВОЙ ТРЕВОГИ: ID=0x%08X, PktSeq=%d",
             packet.node_id, packet.packet_id);

    lora_send_packet((uint8_t *)&packet, sizeof(packet));
}

void lora_send_ping_packet(uint32_t unique_id)
{
    lora_payload_t packet;
    memset(&packet, 0, sizeof(lora_payload_t));

    packet.node_id = unique_id;
    packet.status = STATUS_BIT_PING; // Чистый ноль — признак того, что всё спокойно
    packet.battery_mv = 3300;        // Тут позже будет реальный замер батареи
    packet.packet_id = tx_packet_counter++;

    ESP_LOGI(TAG, "❤ >>> Отправка регулярного Heartbeat (ID=0x%08X)", packet.node_id);
    lora_send_packet((uint8_t *)&packet, sizeof(packet));
}

void lora_send_real_alarm_packet(uint32_t unique_id)
{
    lora_payload_t packet;
    memset(&packet, 0, sizeof(lora_payload_t));

    packet.node_id = unique_id;
    // Флаг внеочередного события + реальный флаг аварии:
    packet.status = STATUS_BIT_INTERRUPT | STATUS_BIT_ALARM_WATER;
    packet.battery_mv = 3300;
    packet.packet_id = tx_packet_counter++;

    ESP_LOGE(TAG, "🚨🚨🚨 КРИТИЧЕСКАЯ ТРЕВОГА! ОТПРАВКА СИГНАЛА ПРОТЕЧКИ! (ID=0x%08X)", packet.node_id);
    lora_send_packet((uint8_t *)&packet, sizeof(packet));
}
