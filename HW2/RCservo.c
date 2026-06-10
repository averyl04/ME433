#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"

#define PWMPIN 16

bool timer_interrupt_function(__unused struct repeating_timer *t) {
    // Read ADC
    uint16_t result1 = adc_read();

    // Print voltage
    printf("%f\r\n", (float)result1 / 4095.0f * 3.3f);

    return true;
}

// Sweep PWM duty cycle forward and backward
void servo_sweep(uint pin,
                 uint16_t min_level,
                 uint16_t max_level,
                 uint16_t step,
                 uint16_t delay_ms) {

    // Forward sweep
    for(uint16_t level = min_level;
        level <= max_level;
        level += step) {

        pwm_set_gpio_level(pin, level);
        sleep_ms(delay_ms);
    }

    // Reverse sweep
    for(uint16_t level = max_level;
        level > min_level;
        level -= step) {

        pwm_set_gpio_level(pin, level);
        sleep_ms(delay_ms);
    }
}

int main() {

    stdio_init_all();

    // Timer interrupt every 100 ms
    struct repeating_timer timer;
    add_repeating_timer_ms(
        -100,
        timer_interrupt_function,
        NULL,
        &timer);

    // Configure PWM
    gpio_set_function(PWMPIN, GPIO_FUNC_PWM);

    uint slice_num = pwm_gpio_to_slice_num(PWMPIN);

    float div = 10.0f;
    pwm_set_clkdiv(slice_num, div);

    uint16_t wrap = 1500;
    pwm_set_wrap(slice_num, wrap);

    pwm_set_enabled(slice_num, true);

    // Start in center position
    pwm_set_gpio_level(PWMPIN, wrap / 2);

    // Configure ADC
    adc_init();
    adc_gpio_init(26);      // GP26 = ADC0
    adc_select_input(0);

    while (true) {

        // Adjust these values for your servo
        servo_sweep(
            PWMPIN,
            300,    // minimum position
            1200,   // maximum position
            10,     // step size
            20      // delay between steps (ms)
        );
    }
}