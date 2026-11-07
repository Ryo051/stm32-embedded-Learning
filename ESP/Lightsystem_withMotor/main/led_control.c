#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_strip.h"

#define LED_STRIP_GPIO 14      // LEDのDATAピンがつながるGPIO
#define LED_STRIP_NUM  10     // 接続しているLEDの数

static led_strip_handle_t led_strip;

/*
　This programm is for core0.
*/

void init_led_strip(void) {
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_STRIP_GPIO,
        .max_leds = LED_STRIP_NUM,
    };

    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
}
void led_control(void *pvParameters) {
    // Initialisierung des LED-Streifens, RMT-Konfiguration
    init_led_strip();

    //ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));

    while (1) {
        for (int i = 0; i < LED_STRIP_NUM; i++) {
            switch(i % 3) {
                case 0:
                    led_strip_set_pixel(led_strip, i, 20*(i+1)%256, 0, 0);
                    break;
                case 1:
                    led_strip_set_pixel(led_strip, i, 0, 20*(i+1)%256, 0);
                    break;
                case 2:
                    led_strip_set_pixel(led_strip, i, 0, 0, 20*(i+1)%256);
                    break;
            }
        }
        led_strip_refresh(led_strip);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}