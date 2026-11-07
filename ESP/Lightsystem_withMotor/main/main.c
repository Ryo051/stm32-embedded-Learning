#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_strip.h"
#include "led_control.h"
#include "motor_control.h"

void app_main(void) {
    xTaskCreatePinnedToCore(led_control, "led_control", 4096, NULL, 5, NULL, 0);
    xTaskCreatePinnedToCore(motor_control, "motor_control", 4096, NULL, 5, NULL, 1);

    vTaskDelay(pdMS_TO_TICKS(1000)); // Wait for tasks to start
    vTaskDelete(NULL); // Main task is deleted after creating other tasks
}