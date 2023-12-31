#ifndef __SLEEP_H__ //{__IT8951_I80_DISPLAY_H__
#define __SLEEP_H__

#include "esp_sleep.h"
#include "driver/touch_pad.h"
#include "esp_log.h"

#define uS_TO_S_FACTOR 1000000ULL
#define S_TO_HOURS 3600
#define HOURS2SLEEP 24
#define SLEEPTIMER (uS_TO_S_FACTOR * S_TO_HOURS * HOURS2SLEEP)
#define ONEMINSLEEP (1000000 * 60 * 1)
#define FIVEMINSLEEP (1000000 * 60 * 5)


#define TOUCH_THRESH_NO_USE 0

static const char *TAG_SLEEP = "sleep";

static void calibrate_touch_pad(touch_pad_t pad){
    int avg = 0;
    const size_t calibration_count = 128;
    for (int i = 0; i < calibration_count; ++i) {
        uint16_t val;
        touch_pad_read(pad, &val);
        avg += val;
    }
    avg /= calibration_count;
    const int min_reading = 300;
    if (avg < min_reading) {
        ESP_LOGI(TAG_SLEEP,"Touch pad #%d average reading is too low: %d (expecting at least %d). ""Not using for deep sleep wakeup.\n", pad, avg, min_reading);
        touch_pad_config(pad, 0);
    } else {
        int threshold = avg - 100;
        ESP_LOGI(TAG_SLEEP, "Touch pad #%d average: %d, wakeup threshold set to %d.\n", pad, avg, threshold);
        touch_pad_config(pad, threshold);
    }
}


void example_deep_sleep_register_touch_wakeup(void){
    // Initialize touch pad peripheral.
    // The default fsm mode is software trigger mode.
    ESP_ERROR_CHECK(touch_pad_init());
    // If use touch pad wake up, should set touch sensor FSM mode at 'TOUCH_FSM_MODE_TIMER'.
    touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
    // Set reference voltage for charging/discharging
    // In this case, the high reference valtage will be 2.4V - 1V = 1.4V
    // The low reference voltage will be 0.5
    // The larger the range, the larger the pulse count value.
    touch_pad_set_voltage(TOUCH_HVOLT_2V4, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);
    //init RTC IO and mode for touch pad.
    touch_pad_config(TOUCH_PAD_NUM8, TOUCH_THRESH_NO_USE);
    touch_pad_config(TOUCH_PAD_NUM9, TOUCH_THRESH_NO_USE);
    calibrate_touch_pad(TOUCH_PAD_NUM8);
    calibrate_touch_pad(TOUCH_PAD_NUM9);
    ESP_LOGI(TAG_SLEEP, "Enabling touch pad wakeup\n");
    ESP_ERROR_CHECK(esp_sleep_enable_touchpad_wakeup());
    ESP_ERROR_CHECK(esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON));
}


void example_deep_sleep_register_rtc_timer_wakeup(void){
    ESP_LOGI(TAG_SLEEP, "Enabling timer wakeup\n");
    ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(SLEEPTIMER));
}



#endif
