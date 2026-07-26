#include "system_manager.h"
#include "system_events.h"
#include "nvs_storage_handler.h"
#include "buzzer_handler.h"
#include "led_strip_handler.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "protocol.h"

static const char *TAG = "SYS_MANAGER";

#define SENSORS_COUNT 5

// Состояния датчиков в оперативной памяти (живут только внутри этого файла)
extern struct sensor_slot_t slots[SENSORS_COUNT];

// Внешние хэндлы железа (из ваших модулей)
extern struct buzzer_t buzzer;
extern struct rgb_led_strip_handle_t led_strip;

static void process_event(const struct system_event_t *event)
{
    switch (event->type)
    {
    // 1. НАЖАТА КНОПКА ПРИВЯЗКИ N (0..4)
    case EVENT_BTN_PAIR_PRESSED:
        if (event->button_num > 0 && event->button_num < 6)
        {
            // Переводим конкретный слот в режим привязки
            slots[event->button_num - 1].state = SLOT_PAIRING;
            ESP_LOGI("FSM", "Слот %d переведен в режим привязки", event->button_num);
        }
        break;

    // 2. ПРИШЕЛ ПАКЕТ ПО LORA
    case EVENT_LORA_PACKET_RX:
        // А) Проверяем, может какой-то слот ждет привязки?
        for (int i = 0; i < 5; i++)
        {
            if (slots[i].state == SLOT_PAIRING)
            {
                slots[i].mac_addr = event->mac_addr;
                slots[i].state = SLOT_OK;
                slots[i].last_seen_ms = xTaskGetTickCount();

                uint8_t slots_to_save[5] = {0};
                for (size_t j = 0; j < 5; j++)
                {
                    if (slots[j].mac_addr)
                        slots_to_save[j] = slots[j].mac_addr;
                }

                nvs_storage_save_sensors(slots_to_save, 5); // Сохраняем в Flash
                ESP_LOGI("FSM", "Датчик 0x%02X привязан к слоту %d", event->mac_addr, i);
                return; // Привязали, выходим
            }
        }

        // Б) Если это обычный пакет, ищем к какому слоту он относится
        for (int i = 0; i < 5; i++)
        {
            if (slots[i].state != SLOT_EMPTY && slots[i].mac_addr == event->mac_addr)
            {
                slots[i].last_seen_ms = xTaskGetTickCount();

                if (event->packet_type & STATUS_BIT_ALARM_WATER)
                {
                    slots[i].state = SLOT_ALARM; // Протечка!
                }
                else
                {
                    slots[i].state = SLOT_OK; // Обычный пинг
                }
                break;
            }
        }
        break;

        // 3. ТАЙМЕР ПРОВЕРКИ ТАЙМАУТОВ (Раз в секунду)
        // case EVENT_PERIODIC_TICK:
        //     for (int i = 0; i < 5; i++)
        //     {
        //         // Если датчик был OK, но не выходил на связь > 30 сек
        //         if (slots[i].state == SLOT_OK &&
        //             (xTaskGetTickCount() - slots[i].last_seen_ms > pdMS_TO_TICKS(30000)))
        //         {
        //             slots[i].state = SLOT_OFFLINE; // Потерян!
        //         }
        //     }
        //     break;

    default:
        break;
    }
}

static void render_ui(void)
{
    bool has_alarm = false;
    bool has_offline = false;

    // 1. Обновляем светодиоды для каждого из 5 слотов
    for (int i = 0; i < 5; i++)
    {
        switch (slots[i].state)
        {
        case SLOT_EMPTY:
            led_strip.rgb_led[i].red = 0;
            led_strip.rgb_led[i].green = 0;
            led_strip.rgb_led[i].blue = 0;
            break;

        case SLOT_PAIRING:
            led_strip.rgb_led[i].red = 0;
            led_strip.rgb_led[i].green = 0;
            led_strip.rgb_led[i].blue = 255;
            break;

        case SLOT_OK:
            led_strip.rgb_led[i].red = 0;
            led_strip.rgb_led[i].green = 255;
            led_strip.rgb_led[i].blue = 0;
            break;

        case SLOT_OFFLINE:
            led_strip.rgb_led[i].red = 0;
            led_strip.rgb_led[i].green = 255;
            led_strip.rgb_led[i].blue = 255;
            has_offline = true;
            break;

        case SLOT_ALARM:
            led_strip.rgb_led[i].red = 255;
            led_strip.rgb_led[i].green = 0;
            led_strip.rgb_led[i].blue = 0;
            has_alarm = true;
            break;
        }
    }
    rgb_led_strip_update(&led_strip);

    // 2. Принимаем решение по единственной пищалке
    if (has_alarm)
    {
        buzzer_set_state(&buzzer, BUZZER_ALARM);
    }
    else if (has_offline)
    {
        buzzer_set_state(&buzzer, BUZZER_ALARM);
    }
    else
    {
        buzzer_set_state(&buzzer, BUZZER_SILENCED);
    }
}

static void system_manager_task(void *pvParameters)
{
    struct system_event_t ev;

    while (1)
    {
        if (system_event_receive(&ev, portMAX_DELAY))
        {
            process_event(&ev); // 1. Изменяем состояние массива
            render_ui();        // 2. Обновляем LED и Пищалку
        }
    }
}

void system_manager_init(void)
{
    // 1. Инициализируем очередь событий
    // system_events_init();

    // 2. Загружаем сохраненные MAC-адреса из NVS в наш массив slots
    // nvs_storage_load_sensors((uint8_t *)slots, SENSORS_COUNT);

    // 3. Запускаем задачу управления
    xTaskCreate(
        system_manager_task,
        "sys_manager_task",
        4096,
        NULL,
        5,
        NULL);

    ESP_LOGI(TAG, "Менеджер системы успешно запущен.");
}