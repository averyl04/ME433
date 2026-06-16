#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"

#include "ssd1306.h"
#include "font.h"

#define SDA_PIN 4
#define SCL_PIN 5

#define LED_PIN 15


void drawChar(int x, int y, char c);
void drawMessage(int x, int y, char *message);


int main() {

    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    i2c_init(i2c_default, 400000);

    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);

    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    ssd1306_setup();

    adc_init();
    adc_gpio_init(26);
    adc_select_input(0);

    uint32_t previousTime =
        to_us_since_boot(get_absolute_time());

    bool ledState = false;

    while(1) {

        uint16_t adcValue = adc_read();

        float voltage =
            adcValue * 3.3f / 4095.0f;

        uint32_t currentTime =
            to_us_since_boot(get_absolute_time());

        float fps =
            1000000.0f /
            (currentTime - previousTime);

        previousTime = currentTime;

        char line1[50];
        char line2[50];

        sprintf(line1,
                "ADC0 = %.3f V",
                voltage);

        sprintf(line2,
                "FPS = %.1f",
                fps);

        ssd1306_clear();

        drawMessage(0,0,"HW4 OLED TEST");

        drawMessage(0,12,line1);

        drawMessage(0,24,line2);

        ssd1306_update();

        ledState = !ledState;

        gpio_put(LED_PIN, 1);
        gpio_put(LED_PIN, 0);


        sleep_ms(500);
    }
}