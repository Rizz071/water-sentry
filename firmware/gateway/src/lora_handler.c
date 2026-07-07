#include "lora_handler.h"
#include "protocol.h"
#include "esp_log.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lora.h"

static const char *TAG = "LORA_GATEWAY";

bool lora_hw_init(void)
{
    ESP_LOGI(TAG, "Запуск инициализации LoRa базовой станции...");

    // 1. Инициализация SPI и проверка присутствия чипа
    if (lora_init() == 0)
    {
        ESP_LOGE(TAG, "Критическая ошибка: Чип SX1278 (Ra-02) базовой станции не откликается по SPI!");
        return false;
    }

    // 2. Настройки ОБЯЗАНЫ строго совпадать с датчиком, иначе связь не установится!
    lora_set_frequency(433e6);     // Частота 433 МГц
    lora_set_spreading_factor(12); // Экстремальный фактор расширения спектра SF12
    lora_set_bandwidth(6);         // Узкая полоса 125 кГц
    lora_enable_crc();             // Включаем аппаратный контроль целостности

    // 3. Переводим чип SX1278 в режим постоянного прослушивания эфира (Implicit/Explicit RX)
    lora_receive(); 

    ESP_LOGI(TAG, "Базовая станция успешно настроена и слушает эфир (Extreme Range)...");
    return true;
}

/**
 * @brief Бесконечный FreeRTOS-таск для приема данных от датчиков
 */
void lora_listen_task(void *pvParameters)
{
    lora_payload_t incoming_packet;
    uint8_t rx_buffer[sizeof(lora_payload_t) + 4]; // Буфер с небольшим запасом

    while (1)
    {
        // На всякий случай пинаем чип оставаться в режиме приема (зависит от реализации либы)
        lora_receive(); 

        // Проверяем, прилетело ли что-то в буфер FIFO чипа SX1278
        if (lora_received())
        {
            // Вычитываем пакет и узнаем его реальную длину в байтах
            int packet_len = lora_receive_packet(rx_buffer, sizeof(rx_buffer));
            
            // Важнейшая проверка: размер пришедших данных должен строго соответствовать нашей структуре
            if (packet_len == sizeof(lora_payload_t))
            {
                // Распаковываем сырые байты обратно в читаемую структуру
                memcpy(&incoming_packet, rx_buffer, sizeof(lora_payload_t));

                // Вытаскиваем метрики качества сигнала (критично для отладки дальнобойности в подвалах!)
                int rssi = lora_packet_rssi();
                float snr = lora_packet_snr();

                ESP_LOGI(TAG, "==================================================");
                ESP_LOGI(TAG, "📥 ПРИНЯТ ПАКЕТ | От Датчика: 0x%08X | Номер пакета: %d", 
                         incoming_packet.node_id, incoming_packet.packet_id);
                ESP_LOGI(TAG, "📊 Качество связи: RSSI = %d dBm, SNR = %.1f dB", rssi, snr);
                ESP_LOGI(TAG, "🔋 Напряжение батареи датчика: %d мВ", incoming_packet.battery_mv);

                // Анализируем битовые флаги статуса, которые нам отправил датчик
                if (incoming_packet.status & STATUS_BIT_ALARM_WATER)
                {
                    ESP_LOGE(TAG, "🚨🚨🚨 КРИТИЧЕСКАЯ ТРЕВОГА! ДАТЧИК 0x%08X ПОЙМАЛ ПРОТЕЧКУ ВОДЫ! 🚨🚨🚨", incoming_packet.node_id);
                    // Здесь в будущем будет команда на включение нашего мощного THT-бузера
                }
                else if (incoming_packet.status & STATUS_BIT_PAIRING_MODE)
                {
                    ESP_LOGW(TAG, "⏳ Режим привязки: Датчик 0x%08X просится в сеть.", incoming_packet.node_id);
                }
                else if (incoming_packet.status & STATUS_BIT_PING)
                {
                    ESP_LOGI(TAG, "💚 Heartbeat: Датчик 0x%08X на связи, всё сухо и спокойно.", incoming_packet.node_id);
                }
                else
                {
                    ESP_LOGW(TAG, "❓ Получен неопознанный статус: 0x%02X", incoming_packet.status);
                }
                ESP_LOGI(TAG, "==================================================");
            }
            else
            {
                // Поймали чужой пакет на этой же частоте или ложную наводку
                ESP_LOGW(TAG, "⚠️ В эфире пойман мусор! Длина пакета (%d байт) не совпадает с протоколом (%d байт)", 
                         packet_len, sizeof(lora_payload_t));
            }
        }

        // Обязательно даем FreeRTOS подышать, чтобы не триггерить Watchdog таймер (WDT)
        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
}