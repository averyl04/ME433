#include "mpu6050.h"

#include "hardware/i2c.h"

extern i2c_inst_t *i2c_default;

static void write_register(uint8_t reg, uint8_t value)
{
    uint8_t buf[2];

    buf[0] = reg;
    buf[1] = value;

    i2c_write_blocking(
        i2c_default,
        MPU6050_ADDR,
        buf,
        2,
        false
    );
}

void mpu6050_init(void)
{
    write_register(PWR_MGMT_1,0x00);

    write_register(ACCEL_CONFIG,0x00);

    write_register(GYRO_CONFIG,0x18);
}

void mpu6050_read(mpu6050_data_t *data)
{
    uint8_t reg = ACCEL_XOUT_H;

    uint8_t raw[14];

    i2c_write_blocking(
        i2c_default,
        MPU6050_ADDR,
        &reg,
        1,
        true
    );

    i2c_read_blocking(
        i2c_default,
        MPU6050_ADDR,
        raw,
        14,
        false
    );

    data->ax =
        (int16_t)((raw[0] << 8) | raw[1]);

    data->ay =
        (int16_t)((raw[2] << 8) | raw[3]);

    data->az =
        (int16_t)((raw[4] << 8) | raw[5]);

    data->temp =
        (int16_t)((raw[6] << 8) | raw[7]);

    data->gx =
        (int16_t)((raw[8] << 8) | raw[9]);

    data->gy =
        (int16_t)((raw[10] << 8) | raw[11]);

    data->gz =
        (int16_t)((raw[12] << 8) | raw[13]);
}