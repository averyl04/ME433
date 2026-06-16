#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/i2c.h"

#include "ssd1306.h"
#include "font.h"
#include "mpu6050.h"

#define SDA_PIN 4
#define SCL_PIN 5

#define LED_PIN 15

//--------------------------------------------------
// Bresenham line drawing
//--------------------------------------------------
void drawLine(int x0, int y0,
              int x1, int y1)
{
    int dx = abs(x1 - x0);
    int sx = (x0 < x1) ? 1 : -1;

    int dy = -abs(y1 - y0);
    int sy = (y0 < y1) ? 1 : -1;

    int err = dx + dy;

    while(1)
    {
        ssd1306_drawPixel(x0, y0, 1);

        if(x0 == x1 && y0 == y1)
        {
            break;
        }

        int e2 = 2 * err;

        if(e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }

        if(e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

//--------------------------------------------------
// Main
//--------------------------------------------------
int main()
{
    stdio_init_all();

    // Heartbeat LED
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // I2C
    i2c_init(i2c_default, 400000);

    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);

    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    // OLED
    ssd1306_setup();

    // IMU
    mpu6050_init();

    bool ledState = false;

    mpu6050_data_t imu;

    while(1)
    {
        //--------------------------------------------------
        // Read IMU
        //--------------------------------------------------
        mpu6050_read(&imu);

        //--------------------------------------------------
        // Convert to g units
        //--------------------------------------------------
        float ax_g =
            imu.ax * 0.000061f;

        float ay_g =
            imu.ay * 0.000061f;

        float az_g =
            imu.az * 0.000061f;

        //--------------------------------------------------
        // Convert temperature
        //--------------------------------------------------
        float tempC =
            imu.temp / 340.0f + 36.53f;

        //--------------------------------------------------
        // Print to serial
        //--------------------------------------------------
        printf(
            "AX=%.2f AY=%.2f AZ=%.2f TEMP=%.2f\n",
            ax_g,
            ay_g,
            az_g,
            tempC
        );

        //--------------------------------------------------
        // Compute line endpoint
        //--------------------------------------------------
        int centerX = 64;
        int centerY = 16;

        int endX =
            centerX + (int)(ax_g * 30.0f);

        int endY =
            centerY - (int)(ay_g * 15.0f);

        //--------------------------------------------------
        // Draw OLED
        //--------------------------------------------------
        ssd1306_clear();

        drawLine(
            centerX,
            centerY,
            endX,
            endY
        );

        ssd1306_update();

        //--------------------------------------------------
        // Heartbeat LED
        //--------------------------------------------------
        ledState = !ledState;

        gpio_put(
            LED_PIN,
            ledState
        );

        //--------------------------------------------------
        // 100 Hz update
        //--------------------------------------------------
        sleep_ms(10);
    }

    return 0;
}