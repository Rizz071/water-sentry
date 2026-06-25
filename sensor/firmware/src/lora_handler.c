#include "lora_handler.h"
#include "lora.h"     // Заголовочник из компонента esp32-lora-library
#include "protocol.h" // Наша структура пакета на 6 байт
#include "esp_log.h"

static const char *TAG = "LORA_HANDLER";

static uint8_t local_node_id = 0;
static uint16_t packet_counter = 0;

bool board_lora_init(uint8_t node_id)
{
    local_node_id = node_id;

    ESP_LOGI(TAG, "Инициализация шины SPI и чипа SX1278...");

    // Функция lora_init() сама берёт пины из sdkconfig, которые ты вбил в menuconfig!
    if (lora_init() != 0)
    {
        ESP_LOGE(TAG, "Критическая ошибка: Чип SX1278 (Ra-02) не обнаружен!");
        return false;
    }

    // Настраиваем параметры под платы Ra-02 (433 МГц)
    lora_set_frequency(433e6);    // Частота 433.0 МГц
    lora_set_spreading_factor(9); // Радио-покрытие SF9
    lora_enable_crc();            // Включаем аппаратный контроль целостности пакетов

    ESP_LOGI(TAG, "Модуль LoRa Ra-02 успешно запущен на частоте 433 МГц!");
    return true;
}

void send_lora_data(bool is_alarm_triggered, bool liquid_detected, uint16_t battery_voltage)
{
    // Компилятор Си позволяет обнулить всю структуру одной строчкой
    LoraPayload tx_packet = {0};

    tx_packet.node_id = local_node_id;
    tx_packet.packet_id = packet_counter++;
    tx_packet.battery_mv = battery_voltage;

    // Набиваем битовую маску статуса
    if (is_alarm_triggered)
    {
        tx_packet.status |= STATUS_BIT_ALARM_EVENT;
    }
    if (liquid_detected)
    {
        tx_packet.status |= STATUS_BIT_LIQUID_LEVEL;
    }

    ESP_LOGI(TAG, "Отправка Си-пакета #%d (%d байт) в эфир...", tx_packet.packet_id, sizeof(tx_packet));

    // Прямая отправка буфера в FIFO чипа по SPI
    lora_send_packet((uint8_t *)&tx_packet, sizeof(tx_packet));

    ESP_LOGI(TAG, "Пакет успешно улетел!");
}