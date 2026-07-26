// #include "slots_handler.h"

// void remote_sensor_init(struct remote_sensor_t *sensor)
// {
//     remote_sensor_set_state(sensor, SENSOR_DISABLED);
// }

// void remote_sensor_set_state(struct remote_sensor_t *sensor, enum remote_sensor_state_t state)
// {
//     sensor->state = state;
// }

// void remote_sensors_task(void *pvParameters)
// {
//     struct remote_sensor_t *remote_sensors = (struct remote_sensor_t *)pvParameters;

//     while (1)
//     {

//         switch (buzzer->current_state)
//         {
//         case UNLINKED:
//             // Молчим: выключаем пин и спим 100 мс (не забиваем CPU)
//             gpio_set_level(buzzer->gpio_num, 0);
//             vTaskDelay(pdMS_TO_TICKS(100));
//             break;

//         case BUZZER_WARNING:
//             // Предупреждение: короткие пики (200 мс звук / 200 мс пауза)
//             gpio_set_level(buzzer->gpio_num, 1);
//             vTaskDelay(pdMS_TO_TICKS(200));
//             gpio_set_level(buzzer->gpio_num, 0);
//             vTaskDelay(pdMS_TO_TICKS(200));
//             break;

//         case BUZZER_ALARM:
//             // Тревога: длинный громкий сигнал (1000 мс звук / 1000 мс пауза)
//             gpio_set_level(buzzer->gpio_num, 1);
//             vTaskDelay(pdMS_TO_TICKS(1000));
//             gpio_set_level(buzzer->gpio_num, 0);
//             vTaskDelay(pdMS_TO_TICKS(1000));
//             break;
//         }
//     };
// }