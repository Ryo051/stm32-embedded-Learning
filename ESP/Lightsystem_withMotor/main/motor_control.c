#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define IN1_PIN GPIO_NUM_26
#define IN2_PIN GPIO_NUM_25
#define IN3_PIN GPIO_NUM_33
#define IN4_PIN GPIO_NUM_32

const uint8_t half_step_patterns[8] = {//Einstellung der Ausgabe der Motorsteuerung
    0b1000,
    0b1100, 
    0b0100,
    0b0110, 
    0b0010,
    0b0011, 
    0b0001,
    0b1001  
};
void set_stepper_pins(uint8_t pattern){//Einstellung der Ausgabe jeder Pin 
    gpio_set_level(IN1_PIN, (pattern >> 0) & 0x01);
    gpio_set_level(IN2_PIN, (pattern >> 1) & 0x01);
    gpio_set_level(IN3_PIN, (pattern >> 2) & 0x01);
    gpio_set_level(IN4_PIN, (pattern >> 3) & 0x01);
}

void init(void){
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << IN1_PIN) | (1ULL << IN2_PIN) | 
                        (1ULL << IN3_PIN) | (1ULL << IN4_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
    printf("start¥n");
}

void motor_control(void *pvParameters)
{
    
    init();
    int step_index = 0;

    while(1){
        set_stepper_pins(half_step_patterns[step_index]);
        step_index = (step_index+1)%8;

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}