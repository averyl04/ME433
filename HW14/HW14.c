#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"

#define HX_SCK 2
#define HX_DT 3

#define MAX_SAMPLES 2000

typedef struct {
    uint32_t time_ms;
    int32_t raw;
    float filtered;
} sample_t;

sample_t data[MAX_SAMPLES];

void hx711_init() {
    gpio_init(HX_SCK);
    gpio_set_dir(HX_SCK, GPIO_OUT);
    gpio_put(HX_SCK, 0);

    gpio_init(HX_DT);
    gpio_set_dir(HX_DT, GPIO_IN);
}

int32_t hx711_read() {

    while (gpio_get(HX_DT)) {
        tight_loop_contents();
    }

    uint32_t raw = 0;

    for (int i = 0; i < 24; i++) {

        gpio_put(HX_SCK, 1);
        sleep_us(1);

        raw <<= 1;

        if (gpio_get(HX_DT)) {
            raw |= 1;
        }

        gpio_put(HX_SCK, 0);
        sleep_us(1);
    }

    // 25th pulse selects gain = 128
    gpio_put(HX_SCK, 1);
    sleep_us(1);
    gpio_put(HX_SCK, 0);
    sleep_us(1);

    // Sign extend 24-bit value
    if (raw & 0x800000) {
        raw |= 0xFF000000;
    }

    return (int32_t)raw;
}

float iir_filter(float x) {

    static float y = 0.0f;
    const float alpha = 0.05f;

    y = alpha * x + (1.0f - alpha) * y;

    return y;
}

int main() {

    stdio_init_all();

    sleep_ms(2000);

    hx711_init();

    printf("HX711 Ready\n");

    while (1) {

        int n_samples = 0;

        printf("Enter sample count:\n");

        scanf("%d", &n_samples);

        if (n_samples > MAX_SAMPLES) {
            n_samples = MAX_SAMPLES;
        }

        absolute_time_t start_time = get_absolute_time();

        for (int i = 0; i < n_samples; i++) {

            int32_t raw = hx711_read();

            uint32_t t_ms =
                absolute_time_diff_us(
                    start_time,
                    get_absolute_time()) / 1000;

            float filtered = iir_filter((float)raw);

            data[i].time_ms = t_ms;
            data[i].raw = raw;
            data[i].filtered = filtered;
        }

        printf("START\n");

        for (int i = 0; i < n_samples; i++) {

            printf("%lu,%ld,%f\n",
                   data[i].time_ms,
                   data[i].raw,
                   data[i].filtered);
        }

        printf("END\n");
    }

    return 0;
}