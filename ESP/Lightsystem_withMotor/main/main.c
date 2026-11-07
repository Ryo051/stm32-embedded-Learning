#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_strip.h"
#include "led_control.h"

void app_main(void) {
    xTaskCreatePinnedToCore(led_control, "led_control", 2048, NULL, 5, NULL, 0);
    xTaskCreatePinnedToCore(motor_control, "motor_control", 2048, NULL, 5, NULL, 1);

    vTaskDelete(NULL); // Main task is deleted after creating other tasks
}