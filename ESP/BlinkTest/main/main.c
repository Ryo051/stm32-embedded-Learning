#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

// DEVKITV1のオンボードLEDピン（GPIO 2）
#define BLINK_GPIO GPIO_NUM_2

static const char *TAG = "LED_TEST";

void app_main(void)
{
    // GPIO2を出力モードとして設定
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    ESP_LOGI(TAG, "ESP32 オンボードLED（GPIO 2）テスト開始");

    while (1) {
        // LED点灯 (HIGH)
        ESP_LOGI(TAG, "LED: ON");
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(1000)); // 1秒待機

        // LED消灯 (LOW)
        ESP_LOGI(TAG, "LED: OFF");
        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(1000)); // 1秒待機
    }
}