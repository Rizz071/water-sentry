#include "lora_handler.h"
#include "protocol.h"
#include "esp_log.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lora.h"
#include "buzzer_handler.h"
#include "system_events.h"

static const char *TAG = "LORA_GATEWAY";

extern struct buzzer_t buzzer;

struct rx_packet_t
{
    lora_payload_t payload; // 6 байт данных от датчика
    int rssi;               // Уровень сигнала (dBm)
    float snr;              // Соотношение сигнал/шум (dB)
};

static QueueHandle_t lora_rx_queue = NULL;

void lora_rx_task(void *pvParameters);
void lora_logic_task(void *pvParameters);

bool lora_hw_init()
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

    // Создаем очередь приёма на 20 пакетов
    lora_rx_queue = xQueueCreate(20, sizeof(struct rx_packet_t));
    if (lora_rx_queue == NULL)
    {
        ESP_LOGE(TAG, "Не удалось создать очередь приёма LoRa!");
        return false;
    }

    // Запускаем единый таск-обработчик принятых lora-сообщений
    xTaskCreate(lora_logic_task, "lora_logic_task", 3072, NULL, 4, NULL);

    // Запускаем единый таск-прмёник lora-сообщений
    xTaskCreate(lora_rx_task, "lora_rx_task", 3072, NULL, 5, NULL);

    // 3. Переводим чип SX1278 в режим постоянного прослушивания эфира (Implicit/Explicit RX)
    lora_receive();

    ESP_LOGI(TAG, "Базовая станция успешно настроена и слушает эфир (Extreme Range)...");
    return true;
}

/**
 * @brief Бесконечный FreeRTOS-таск для приема данных от датчиков
 */
void lora_rx_task(void *pvParameters)
{
    struct rx_packet_t rx_msg;
    uint8_t rx_buffer[sizeof(lora_payload_t)];

    while (1)
    {
        if (lora_received())
        {
            int packet_len = lora_receive_packet(rx_buffer, sizeof(rx_buffer));

            if (packet_len == sizeof(lora_payload_t))
            {
                // 1. Быстро собираем пакет
                memcpy(&rx_msg.payload, rx_buffer, sizeof(lora_payload_t));
                rx_msg.rssi = lora_packet_rssi();
                rx_msg.snr = lora_packet_snr();

                // 2. Мгновенно скидываем в очередь для обработки
                if (xQueueSend(lora_rx_queue, &rx_msg, 0) != pdTRUE)
                {
                    ESP_LOGE(TAG, "🚨 Очередь приёма переполнена! Пакет пропущен!");
                }
            }
            else
            {
                // Поймали чужой пакет на этой же частоте или ложную наводку
                ESP_LOGW(TAG, "⚠️ В эфире пойман мусор! Длина пакета (%d байт) не совпадает с протоколом (%d байт)",
                         packet_len, sizeof(lora_payload_t));
            }

            lora_receive(); // Держим чип в режиме RX
        }
        vTaskDelay(pdMS_TO_TICKS(10)); // Не блокируем WDT
    }
}

/**
 * @brief Бесконечный FreeRTOS-таск для обработки принятых по lora данных
 */
void lora_logic_task(void *pvParameters)
{
    struct rx_packet_t incoming_packet;

    while (1)
    {

        // Проверяем, прилетело ли что-то в очередь приёма
        if (xQueueReceive(lora_rx_queue, &incoming_packet, portMAX_DELAY) == pdTRUE)
        {

            ESP_LOGI(TAG, "==================================================");
            ESP_LOGI(TAG, "📥 ПРИНЯТ ПАКЕТ | От Датчика: 0x%08X | Номер пакета: %d",
                     incoming_packet.payload.mac_addr, incoming_packet.payload.packet_id);
            ESP_LOGI(TAG, "📧 Код события: %08X", incoming_packet.payload.status);
            ESP_LOGI(TAG, "📊 Качество связи: RSSI = %d dBm, SNR = %.1f dB", incoming_packet.rssi, incoming_packet.snr);
            ESP_LOGI(TAG, "🔋 Напряжение батареи датчика: %d мВ", incoming_packet.payload.battery_mv);

            // Анализируем битовые флаги статуса, которые нам отправил датчик
            if (incoming_packet.payload.status & STATUS_BIT_ALARM_WATER)
            {
                ESP_LOGE(TAG, "🚨🚨🚨 КРИТИЧЕСКАЯ ТРЕВОГА! ДАТЧИК 0x%08X ПОЙМАЛ ПРОТЕЧКУ ВОДЫ! 🚨🚨🚨", incoming_packet.payload.mac_addr);
                struct system_event_t ev = {
                    .type = EVENT_LORA_PACKET_RX,
                    .mac_addr = incoming_packet.payload.mac_addr,
                    .packet_type = incoming_packet.payload.status};
                system_event_post(&ev);
            }
            else if (incoming_packet.payload.status & STATUS_BIT_PAIRING_MODE)
            {
                ESP_LOGW(TAG, "⏳ Режим привязки: Датчик 0x%08X просится в сеть.", incoming_packet.payload.mac_addr);
                struct system_event_t ev = {
                    .type = EVENT_LORA_PACKET_RX,
                    .mac_addr = incoming_packet.payload.mac_addr,
                    .packet_type = incoming_packet.payload.status};
                system_event_post(&ev);
            }
            else if (incoming_packet.payload.status & STATUS_BIT_PING)
            {
                ESP_LOGI(TAG, "💚 Heartbeat: Датчик 0x%08X на связи, всё сухо и спокойно.", incoming_packet.payload.mac_addr);
                struct system_event_t ev = {
                    .type = EVENT_LORA_PACKET_RX,
                    .mac_addr = incoming_packet.payload.mac_addr,
                    .packet_type = incoming_packet.payload.status};
                system_event_post(&ev);
            }
            else
            {
                ESP_LOGW(TAG, "❓ Получен неопознанный статус: 0x%02X", incoming_packet.payload.status);
            }
            ESP_LOGI(TAG, "==================================================");
        }
    }
}