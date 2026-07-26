#include "buttons_hal.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/gpio.h"
#include "app_config.h"

static const char *TAG = "BUTTONS_HAL";

#define BUTTONS_ADC_UNIT     ADC_UNIT_1
#define BUTTON_ADC_CHANNEL   ADC_CHANNEL_1 // GPIO1

static adc_oneshot_unit_handle_t adc_handle = NULL;

static uint8_t adc_to_button(int adc_raw)
{
    if (adc_raw > 3500)
        return 0; // Nothing pressed (pull-up to 3.3V)

    if (adc_raw >= 0   && adc_raw < 300)  return 1; // BTN1 (0 Ω)
    if (adc_raw >= 500 && adc_raw < 1000) return 2; // BTN2 (2.2k)
    if (adc_raw >= 1200 && adc_raw < 1700) return 3; // BTN3 (5.5k)
    if (adc_raw >= 2000 && adc_raw < 3000) return 4; // BTN4 (12.3k)
    if (adc_raw >= 3000 && adc_raw < 3500) return 5; // BTN5 (27.3k)

    return 0; // Noise
}

void buttons_hal_init(void)
{
    ESP_LOGI(TAG, "Initializing ADC buttons hardware...");

    adc_oneshot_unit_init_cfg_t init_cfg = {
        .unit_id = BUTTONS_ADC_UNIT,
        .clk_src = ADC_DIGI_CLK_SRC_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_cfg, &adc_handle));

    adc_oneshot_chan_cfg_t chan_cfg = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, BUTTON_ADC_CHANNEL, &chan_cfg));

    ESP_LOGI(TAG, "ADC buttons hardware initialized.");
}

uint8_t buttons_hal_read(void)
{
    int adc_raw = 0;
    esp_err_t r = adc_oneshot_read(adc_handle, BUTTON_ADC_CHANNEL, &adc_raw);
    if (r != ESP_OK)
    {
        ESP_LOGE(TAG, "ADC read error");
        return 0;
    }

    return adc_to_button(adc_raw);
}