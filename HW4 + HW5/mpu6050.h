#ifndef MPU6050_H
#define MPU6050_H

#include <stdint.h>

#define MPU6050_ADDR 0x68

// config registers
#define CONFIG          0x1A
#define GYRO_CONFIG     0x1B
#define ACCEL_CONFIG    0x1C
#define PWR_MGMT_1      0x6B
#define PWR_MGMT_2      0x6C

// sensor registers
#define ACCEL_XOUT_H    0x3B
#define TEMP_OUT_H      0x41
#define GYRO_XOUT_H     0x43
#define WHO_AM_I        0x75

typedef struct {
    int16_t ax;
    int16_t ay;
    int16_t az;

    int16_t temp;

    int16_t gx;
    int16_t gy;
    int16_t gz;
} mpu6050_data_t;

void mpu6050_init(void);
void mpu6050_read(mpu6050_data_t *data);

#endif