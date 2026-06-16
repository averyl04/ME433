#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"


#define I2C_PORT i2c0
#define SDA_PIN 4
#define SCL_PIN 5

// MCP23008 Address
// A2=A1=A0=0 -> 0x20
#define MCP23008_ADDR 0x20

#define IODIR  0x00
#define GPIO   0x09
#define OLAT   0x0A


#define HEARTBEAT_LED PICO_DEFAULT_LED_PIN


// Write one byte to a register
void mcp_write_register(uint8_t reg, uint8_t value)
{
    uint8_t buffer[2];

    buffer[0] = reg;
    buffer[1] = value;

    i2c_write_blocking(I2C_PORT,
                       MCP23008_ADDR,
                       buffer,
                       2,
                       false);
}

// Read one byte from a register
uint8_t mcp_read_register(uint8_t reg)
{
    uint8_t value;

    // Send register address
    i2c_write_blocking(I2C_PORT,
                       MCP23008_ADDR,
                       &reg,
                       1,
                       true);

    // Read register contents
    i2c_read_blocking(I2C_PORT,
                      MCP23008_ADDR,
                      &value,
                      1,
                      false);

    return value;
}

// Set or clear an individual pin
void mcp_set_pin(uint8_t pin, bool state)
{
    uint8_t olat;

    olat = mcp_read_register(OLAT);

    if(state)
        olat |= (1 << pin);
    else
        olat &= ~(1 << pin);

    mcp_write_register(OLAT, olat);
}

// Read an individual pin
bool mcp_read_pin(uint8_t pin)
{
    uint8_t gpio;

    gpio = mcp_read_register(GPIO);

    return (gpio & (1 << pin)) != 0;
}

// Initialize MCP23008
void mcp_init(void)
{
    // GP7 output
    // GP0-GP6 inputs
    //
    // Bit = 1 -> Input
    // Bit = 0 -> Output
    //
    // GP7 = output
    // GP0 = input
    //
    // 0b01111111 = 0x7F

    mcp_write_register(IODIR, 0x7F);

    // Start with outputs low
    mcp_write_register(OLAT, 0x00);
}


int main()
{
    stdio_init_all();

    // Heartbeat LED
    gpio_init(HEARTBEAT_LED);
    gpio_set_dir(HEARTBEAT_LED, GPIO_OUT);

    // Initialize I2C0 at 100 kHz
    i2c_init(I2C_PORT, 100 * 1000);

    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);

    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    sleep_ms(100);

    // Initialize MCP23008
    mcp_init();

    absolute_time_t last_blink = get_absolute_time();
    bool heartbeat_state = false;

    while (true)
    {
       
        if (absolute_time_diff_us(last_blink,
                                  get_absolute_time()) > 500000)
        {
            heartbeat_state = !heartbeat_state;

            gpio_put(HEARTBEAT_LED,
                     heartbeat_state);

            last_blink = get_absolute_time();
        }


        bool button_pressed = mcp_read_pin(0);


        if(button_pressed)
        {
            mcp_set_pin(7, true);
        }
        else
        {
            mcp_set_pin(7, false);
        }

        sleep_ms(10);
    }

    return 0;
}