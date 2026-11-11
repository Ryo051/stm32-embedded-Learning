#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_strip.h"

#define LED_STRIP_GPIO 14      // LEDのDATAピンがつながるGPIO
#define LED_STRIP_NUM  10     // 接続しているLEDの数

typedef struct {
    uint8_t target_val;  // 0~255
    uint8_t pwm_step;   // 0~9
    uint8_t r;
    uint8_t g;
    uint8_t b;
} Led_state_t;

const uint8_t GAMMA_INT_LUT[256] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   1,   1,   1,   1,   1,   1,   1,   2,   2,   2,
    2,   2,   2,   3,   3,   3,   3,   4,   4,   4,   4,   5,   5,   5,   5,   6,
    6,   6,   7,   7,   7,   8,   8,   8,   9,   9,   9,  10,  10,  11,  11,  11,
   12,  12,  13,  13,  14,  14,  15,  15,  16,  16,  17,  17,  18,  18,  19,  19,
   20,  20,  21,  21,  22,  23,  23,  24,  24,  25,  26,  26,  27,  28,  28,  29,
   30,  30,  31,  32,  32,  33,  34,  35,  35,  36,  37,  38,  39,  39,  40,  41,
   42,  43,  44,  44,  45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,
   57,  58,  59,  60,  61,  62,  63,  64,  65,  66,  67,  68,  70,  71,  72,  73,
   74,  75,  77,  78,  79,  80,  82,  83,  84,  85,  87,  88,  89,  91,  92,  93,
   95,  96,  98,  99, 100, 102, 103, 105, 106, 108, 109, 111, 112, 114, 115, 117,
  119, 120, 122, 124, 125, 127, 129, 130, 132, 134, 136, 137, 139, 141, 143, 145,
  146, 148, 150, 152, 154, 156, 158, 160, 162, 164, 166, 168, 170, 172, 174, 176,
  178, 180, 182, 184, 186, 188, 191, 193, 195, 197, 199, 202, 204, 206, 209, 211,
  213, 215, 218, 220, 223, 225, 227, 230, 232, 235, 237, 240, 242, 245, 247, 250,
  252, 255
};

// -----------------------------------------------------------------------------
// 2. 小数パルス発生テーブル (0 ~ 255 全要素)
//    10段階 (0 ~ 9): 0.1刻みの端数を表現。10周期中「何回 +1 を打つか」を示す
// -----------------------------------------------------------------------------
static const uint8_t GAMMA_FRAC_LUT[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0 - 9   
    1, 1, 2, 2, 3, 3, 4, 5, 6, 7,  // 10 - 19
    8, 9, 0, 2, 3, 5, 7, 9, 1, 3,  // 20 - 29
    5, 7, 9, 2, 4, 7, 0, 2, 5, 8,  // 30 - 39
    1, 4, 7, 0, 3, 6, 0, 3, 7, 0,  // 40 - 49
    4, 7, 1, 5, 9, 3, 7, 1, 6, 0,  // 50 - 59
    5, 0, 4, 9, 4, 9, 4, 0, 5, 1,  // 60 - 69
    6, 2, 8, 4, 0, 6, 2, 8, 4, 1,  // 70 - 79
    7, 4, 0, 7, 3, 0, 7, 4, 1, 8,  // 80 - 89
    5, 3, 0, 8, 5, 3, 0, 8, 6, 4,  // 90 - 99
    1, 9, 7, 5, 3, 1, 9, 8, 6, 4,  // 100 - 109
    3, 1, 0, 8, 7, 6, 4, 3, 2, 1,  // 110 - 119
    0, 9, 8, 7, 6, 5, 4, 4, 3, 2,  // 120 - 129
    2, 1, 1, 0, 0, 9, 9, 8, 8, 8,  // 130 - 139
    7, 7, 7, 6, 6, 6, 6, 5, 5, 5,  // 140 - 149
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5,  // 150 - 159
    5, 5, 6, 6, 6, 6, 7, 7, 7, 8,  // 160 - 169
    8, 8, 9, 9, 0, 0, 1, 1, 2, 2,  // 170 - 179
    3, 4, 4, 5, 6, 7, 7, 8, 9, 0,  // 180 - 189
    1, 2, 3, 4, 5, 6, 7, 8, 9, 0,  // 190 - 199
    1, 3, 4, 5, 7, 8, 9, 1, 2, 4,  // 200 - 209
    5, 7, 8, 0, 1, 3, 5, 6, 8, 0,  // 210 - 219
    1, 3, 5, 7, 9, 1, 3, 5, 7, 9,  // 220 - 229
    2, 4, 6, 8, 1, 3, 6, 8, 1, 3,  // 230 - 239
    6, 8, 1, 4, 6, 9, 2, 4, 7, 0,  // 240 - 249
    3, 5, 8, 1, 4, 0              // 250 - 255
};

// -----------------------------------------------------------------------------
// 3. 固定色 (255, 100, 5) 演算とパルス判定関数
// -----------------------------------------------------------------------------

// 指定の固定カラー比率
#define COLOR_R  255
#define COLOR_G  100
#define COLOR_B    5

static led_strip_handle_t led_strip;

/*
　This programm is for core0.
*/

void init_led_strip(void) {
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_STRIP_GPIO,
        .max_leds = LED_STRIP_NUM,
        .led_model = LED_MODEL_WS2811,
    };

    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .mem_block_symbols = 64,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
}

void calculate_fixed_color_pwm(Led_state_t *led_state)
{
    // 1. 整数ベース値と小数パルス数を取得（どちらも配列参照で1クロック）
    uint16_t base_scale = GAMMA_INT_LUT[led_state->target_val];
    uint8_t  frac_pulse = GAMMA_FRAC_LUT[led_state->target_val];

    // 2. パルスステップが端数閾値未満なら、全体のスケールを +1 補完
    if (led_state->pwm_step < frac_pulse) {
        base_scale += 1;
    }

    // 3. スケール(0~256)と固定色(255, 100, 5)を乗算して8bitシフト(256除算)
    //    浮動小数点を使わず、純粋な整数演算のみで超高速化
    led_state->r = (uint8_t)((base_scale * COLOR_R) >> 8);
    led_state->g = (uint8_t)((base_scale * COLOR_G) >> 8);
    led_state->b = (uint8_t)((base_scale * COLOR_B) >> 8);
}
/*
void calculate_fixed_color_pwm(uint8_t input_val, uint8_t pwm_step, 
                                uint8_t *out_r, uint8_t *out_g, uint8_t *out_b) 
{
    // 1. 整数ベース値と小数パルス数を取得（どちらも配列参照で1クロック）
    uint16_t base_scale = GAMMA_INT_LUT[input_val];
    uint8_t  frac_pulse = GAMMA_FRAC_LUT[input_val];

    // 2. パルスステップが端数閾値未満なら、全体のスケールを +1 補完
    if (pwm_step < frac_pulse) {
        base_scale += 1;
    }

    // 3. スケール(0~256)と固定色(255, 100, 5)を乗算して8bitシフト(256除算)
    //    浮動小数点を使わず、純粋な整数演算のみで超高速化
    *out_r = (uint8_t)((base_scale * COLOR_R) >> 8);
    *out_g = (uint8_t)((base_scale * COLOR_G) >> 8);
    *out_b = (uint8_t)((base_scale * COLOR_B) >> 8);
}*/

void init_led_array(Led_state_t *led_state, size_t num_leds) {
    memset(led_state, 0, sizeof(Led_state_t) * num_leds);
}

void led_control(void *pvParameters) {
    
    Led_state_t led_state[LED_STRIP_NUM]; // Array zur Speicherung des LED-Zustands
    memset(led_state, 0, sizeof(led_state)); // Initialisierung des LED-Zustands


    // Initialisierung


    init_led_strip(); // Initialisierung des LED-Streifens, RMT-Konfiguration

    // Hauptschleife zur Steuerung der LEDs
    while (1) {
        
        for (int i = 0; i < LED_STRIP_NUM; i++) {
            calculate_fixed_color_pwm(&led_state[i]); // Beispielaufruf der Funktion zur Berechnung der PWM-Werte
            led_strip_set_pixel(led_strip, i, led_state[i].g, led_state[i].r, led_state[i].b);              
            led_state[i].target_val = (led_state[i].target_val + 1) % 256; // Increment target value for next iteration
            led_state[i].pwm_step = (led_state[i].pwm_step + 1) % 10; // Increment PWM step for next iteration
        }
        
        

        led_strip_refresh(led_strip);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
