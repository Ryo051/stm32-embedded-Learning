#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_strip.h"
#include "esp_random.h"
#include "esp_log.h"
#include "esp_timer.h"

#define LED_STRIP_GPIO 14      // LEDのDATAピンがつながるGPIO
#define LED_STRIP_NUM  100     // 接続しているLEDの数
#define DELAY_MS 60   
#define CANDLE_MODE 0
#define INAHO_MODE 1
#define WAVE_MODE 2
#define SOLID_MODE 9
#define MODE_COUNT 4
#define MODE_CHANGE_INTERVAL_US (15 * 60 * 1000000) // 15

#define DEFAULT_BRIGHTNESS 100
#define MINIMUM_BRIGHTNESS 10
#define MAXIMUM_BRIGHTNESS 255

typedef struct {
    uint8_t current_val; // 0~255
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

// sin波 (0~255) を振幅 0~255 にマッピングしたLUT
static const uint8_t SINE_LUT[256] = {
    128, 131, 134, 137, 140, 143, 146, 149, 152, 155, 158, 162, 165, 167, 170, 173,
    176, 179, 182, 185, 188, 190, 193, 196, 198, 201, 203, 206, 208, 211, 213, 215,
    218, 220, 222, 224, 226, 228, 230, 232, 234, 235, 237, 238, 240, 241, 243, 244,
    245, 246, 247, 248, 249, 250, 250, 251, 252, 252, 253, 253, 253, 253, 253, 253,
    253, 253, 253, 253, 253, 253, 252, 252, 251, 250, 250, 249, 248, 247, 246, 245,
    244, 243, 241, 240, 238, 237, 235, 234, 232, 230, 228, 226, 224, 222, 220, 218,
    215, 213, 211, 208, 206, 203, 201, 198, 196, 193, 190, 188, 185, 182, 179, 176,
    173, 170, 167, 165, 162, 158, 155, 152, 149, 146, 143, 140, 137, 134, 131, 128,
    124, 121, 118, 115, 112, 109, 106, 103, 100,  97,  94,  90,  87,  85,  82,  79,
     76,  73,  70,  67,  64,  62,  59,  56,  54,  51,  49,  46,  44,  41,  39,  37,
     34,  32,  30,  28,  26,  24,  22,  20,  18,  17,  15,  14,  12,  11,   9,   8,
      7,   6,   5,   4,   3,   2,   2,   1,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   1,   2,   2,   3,   4,   5,   6,   7,
      8,   9,  11,  12,  14,  15,  17,  18,  20,  22,  24,  26,  28,  30,  32,  34,
     37,  39,  41,  44,  46,  49,  51,  54,  56,  59,  62,  64,  67,  70,  73,  76,
     79,  82,  85,  87,  90,  94,  97, 100, 103, 106, 109, 112, 115, 118, 121, 124
};

// -----------------------------------------------------------------------------
// 3. 固定色 (255, 100, 5) 演算とパルス判定関数
// -----------------------------------------------------------------------------

// 指定の固定カラー比率
#define COLOR_R  255
#define COLOR_G  35
#define COLOR_B    5

static led_strip_handle_t led_strip;

//  -------------------------------------------------------------
// 4. グローバル変数とLED制御タスクの実装
//  -------------------------------------------------------------

// 風の強さをグローバルに保持し、周期的に変化させる
static uint8_t global_wind_force = 100;
static uint8_t wind_buffer[LED_STRIP_NUM];

// タイマー割り込みやタスクで呼び出されるLED制御関数
static volatile uint8_t current_mode = INAHO_MODE; // 初期モードはキャンドル風

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
// 15分ごとに呼ばれるコールバック関数
static void mode_timer_callback(void* arg) {
    current_mode = (current_mode + 1) % MODE_COUNT;
    ESP_LOGI("TIMER", "Auto Mode Changed to: %d", current_mode);
}

// タイマーの初期化・開始関数
void init_mode_timer(void) {
    const esp_timer_create_args_t timer_args = {
        .callback = &mode_timer_callback,
        .name = "mode_switch_timer"
    };
    esp_timer_handle_t mode_timer;
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &mode_timer));
    // 周期タイマーとしてスタート
    ESP_ERROR_CHECK(esp_timer_start_periodic(mode_timer, MODE_CHANGE_INTERVAL_US));
}

void calculate_fixed_color_pwm(Led_state_t *led_state)
{
    // 1. 整数ベース値と小数パルス数を取得（どちらも配列参照で1クロック）
    uint16_t base_scale = GAMMA_INT_LUT[led_state->current_val];
    uint8_t  frac_pulse = GAMMA_FRAC_LUT[led_state->current_val];

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
void calculate_simple_color(Led_state_t *led_state)
{
    // 1. 整数ベース値と小数パルス数を取得（どちらも配列参照で1クロック）
    uint16_t base_scale = GAMMA_INT_LUT[led_state->current_val];


    led_state->r = (uint8_t)((base_scale * COLOR_R) >> 8);
    led_state->g = (uint8_t)((base_scale * COLOR_G) >> 8);
    led_state->b = (uint8_t)((base_scale * COLOR_B) >> 8);
}

void update_inaho_logic(Led_state_t leds[], size_t num_leds) {
    static uint16_t wind_timer = 0;
    wind_timer++;

    // 1. マスター風の制御：普段は静かなそよ風(30~60)、たまに突風(120~150)が吹き抜ける
    if (wind_timer % 10 == 0) {
        // 約10%の確率で一時的な突風を発生させる
        if ((esp_random() % 100) < 10) {
            global_wind_force = 120 + (esp_random() % 40); // 突風: 120~160
        } else {
            // 普段の穏やかなそよ風: 輝度 30~70 (ここを低く抑えるのが白化防止のキモ)
            global_wind_force = 30 + (esp_random() % 41);
        }
    }

    // 2. 風の空間伝播（0番から順に流れていく）
    if (wind_timer % 4 == 0) {
        for (int i = num_leds - 1; i > 0; i--) {
            wind_buffer[i] = wind_buffer[i - 1];
        }
        wind_buffer[0] = global_wind_force;
    }

    // 3. 各LEDへの適応とイージング・RGB確定
    for (size_t i = 0; i < num_leds; i++) {
        // 個別ノイズ (-5 ~ +5)
        int8_t local_noise = (esp_random() % 11) - 5;
        int16_t final_target = wind_buffer[i] + local_noise;
        
        // 輝度レンジを 15 ~ 160 に抑え込み、絶対発光量の飽和をガード
        if (final_target < 15)  final_target = 15;
        if (final_target > 160) final_target = 160;

        leds[i].target_val = (uint8_t)final_target;

        // イージング（スムーズな追従）
        if (leds[i].current_val < leds[i].target_val) {
            leds[i].current_val++;
        } else if (leds[i].current_val > leds[i].target_val) {
            leds[i].current_val--;
        }

        leds[i].pwm_step = (leds[i].pwm_step + 1) % 10;

    }
}

void update_candle_logic(Led_state_t *led) {
    // 1. 現在値が目標値に到達したか判定
    if (led->current_val == led->target_val) {
        // たまに風で急に暗くなる演出 (約3%の確率)
        if ((esp_random() % 100) < 3) {
            led->target_val = 10 + (esp_random() % 30); // 10~40 の超低輝度へ
        } else {
            // 通常のゆらぎ目標値 (80~200の範囲)
            led->target_val = 80 + (esp_random() % 121);
        }
    } else {
        // 2. 目標に向かって現在値を少しずつ移動（追従速度の調整）
        if (led->current_val < led->target_val) {
            led->current_val++;
        } else {
            led->current_val--;
        }
    }

    // 3. PWMステップの更新 (0~9)
    led->pwm_step = (led->pwm_step + 1) % 10;
}


void update_wave_logic(Led_state_t leds[], size_t num_leds) {
    static uint8_t wave_position = 0;
    wave_position += 2; // 波の進むスピードを調整

    for (size_t i = 0; i < num_leds; i++) {
        // uint8_t の自然なオーバーフロー(0~255循環)を利用してインデックス計算
        // LED間隔のオフセットを例えば 12 とする (256/20 ≒ 12.8 でちょうど1波長入る)
        uint8_t index = wave_position + (i * 12);

        // LUTから1クロックで値取得 (0~253)
        // 最大光量を少し抑えるため 0.6 倍（>>1 など）にスケール調整も可能
        leds[i].target_val = SINE_LUT[index];

        // イージング（スムーズな追従）
        if (leds[i].current_val < leds[i].target_val) {
            leds[i].current_val++;
        } else if (leds[i].current_val > leds[i].target_val) {
            leds[i].current_val--;
        }

        leds[i].pwm_step = (leds[i].pwm_step + 1) % 10;
    }
}
void init_led_array(Led_state_t *led_state, size_t num_leds) {
    memset(led_state, 0, sizeof(Led_state_t) * num_leds);
}

void led_control(void *pvParameters) {
    Led_state_t led_state[LED_STRIP_NUM];
    memset(led_state, 0, sizeof(led_state));

    // 1. ハードウェア初期化
    init_led_strip();
    init_mode_timer();

    for (int i = 0; i < LED_STRIP_NUM; i++) {
        led_state[i].current_val = DEFAULT_BRIGHTNESS;
        led_state[i].target_val  = DEFAULT_BRIGHTNESS;
    }

    // 2. メインループ
    while (1) {
        // -------------------------------------------------------------
        // A. 【Updateフェーズ】モード別の輝度・目標値・イージングの更新
        // -------------------------------------------------------------
        switch (current_mode) {
            case CANDLE_MODE:
                for (int i = 0; i < LED_STRIP_NUM; i++) {
                    update_candle_logic(&led_state[i]);
                }
                break;

            case INAHO_MODE:
                update_inaho_logic(led_state, LED_STRIP_NUM);
                break;

            case WAVE_MODE:
                update_wave_logic(led_state, LED_STRIP_NUM);
                break;
            case SOLID_MODE:
                for (int i = 0; i < LED_STRIP_NUM; i++) {
                    led_state[i].current_val = 100; // 最大輝度
                    led_state[i].pwm_step = 0; // PWMステップをリセット
                }
                break;
            default:
                break;
        }

        // -------------------------------------------------------------
        // B. 【Renderフェーズ】共通のカラーPWM計算 ＋ ピクセルバッファ描画
        // -------------------------------------------------------------
        for (int i = 0; i < LED_STRIP_NUM; i++) {
            calculate_fixed_color_pwm(&led_state[i]);
            led_strip_set_pixel(led_strip, i, led_state[i].g, led_state[i].r, led_state[i].b);
        }

        // -------------------------------------------------------------
        // C. 【Debugフェーズ】先頭ピクセルのみピンポイントでログ出力
        // -------------------------------------------------------------
        ESP_LOGI("LED_DEBUG", "Val:%d, Step:%d -> R:%d, G:%d, B:%d", 
                 led_state[0].current_val, led_state[0].pwm_step,
                 led_state[0].r, led_state[0].g, led_state[0].b);

        // -------------------------------------------------------------
        // D. 【Outputフェーズ】物理LEDへの一括送信とタスクウェイト
        // -------------------------------------------------------------
        led_strip_refresh(led_strip);
        vTaskDelay(pdMS_TO_TICKS(DELAY_MS));
    }
}

/* --- 抱えている問題 ---

光度の最低光度合が赤しか光らない問題が発生するのでそれを解決するために、LEDの色比率を調整する必要がある。現在は固定色比率であるため、低輝度時に赤が強く出てしまう。
→使う範囲を制限する

キャンドルモードのゆらぎが小さすぎる問題が発生するので、ゆらぎの範囲を広げる必要がある。現在は80~200の範囲であるため、低輝度時に変化が少なく見える。
あともっと大幅に変化させるために、目標値の変化をもっと大きくする必要がある。現在は10~40の範囲であるため、低輝度時に変化が少なく見える。




*/