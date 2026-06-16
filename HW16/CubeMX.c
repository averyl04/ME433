
#include <stdio.h>
#include <string.h>

#define INA219_ADDR               0x40

#define INA219_REG_CONFIG         0x00
#define INA219_REG_CURRENT        0x04
#define INA219_REG_CALIBRATION    0x05

volatile int state = 0;

volatile uint32_t adc_position = 0;
volatile short current_raw = 0;

volatile int data_index[400];
volatile int data_desired[400];
volatile int data_actual[400];

float kp = 4.0f;
float ki = 0.05f;

uint32_t read_adc(void);

void init_ina219(void);
void writeINA219(int reg, int value);
signed short readINA219(unsigned char reg);
float read_ina219(void);

void motor_off(void);
void set_motor_pwm(float control);


int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart3,
                      (uint8_t*)&ch,
                      1,
                      HAL_MAX_DELAY);

    return ch;
}

uint32_t read_adc(void)
{
    uint32_t raw;

    HAL_ADC_Start(&hadc1);

    if(HAL_ADC_PollForConversion(&hadc1,10) == HAL_OK)
    {
        raw = HAL_ADC_GetValue(&hadc1);
    }
    else
    {
        raw = 0;
    }

    HAL_ADC_Stop(&hadc1);

    return raw;
}

void writeINA219(int reg, int value)
{
    uint8_t buf[3];

    buf[0] = reg;
    buf[1] = value >> 8;
    buf[2] = value & 0xFF;

    HAL_I2C_Master_Transmit(
        &hi2c2,
        INA219_ADDR << 1,
        buf,
        3,
        10);
}


signed short readINA219(unsigned char reg)
{
    HAL_I2C_Master_Transmit(
        &hi2c2,
        INA219_ADDR << 1,
        &reg,
        1,
        10);

    uint8_t buffer[2];

    HAL_I2C_Master_Receive(
        &hi2c2,
        INA219_ADDR << 1,
        buffer,
        2,
        10);

    signed short value =
        (buffer[0] << 8) | buffer[1];

    return value;
}


void init_ina219(void)
{
    unsigned short calValue = 1024;

    unsigned short config =
        0b0011000010001111;

    writeINA219(
        INA219_REG_CALIBRATION,
        calValue);

    writeINA219(
        INA219_REG_CONFIG,
        config);
}


float read_ina219(void)
{
    signed short value =
        readINA219(
            INA219_REG_CURRENT);

    return value / 3.0f;
}

void motor_off(void)
{
    __HAL_TIM_SET_COMPARE(
        &htim1,
        TIM_CHANNEL_1,
        2400);

    __HAL_TIM_SET_COMPARE(
        &htim1,
        TIM_CHANNEL_2,
        2400);
}


void set_motor_pwm(float control)
{
    int pwm;

    if(control > 0)
    {
        pwm = 2400 - (int)control;

        if(pwm < 0)
            pwm = 0;

        if(pwm > 2400)
            pwm = 2400;

        __HAL_TIM_SET_COMPARE(
            &htim1,
            TIM_CHANNEL_1,
            2400);

        __HAL_TIM_SET_COMPARE(
            &htim1,
            TIM_CHANNEL_2,
            pwm);
    }
    else
    {
        pwm = 2400 - (int)(-control);

        if(pwm < 0)
            pwm = 0;

        if(pwm > 2400)
            pwm = 2400;

        __HAL_TIM_SET_COMPARE(
            &htim1,
            TIM_CHANNEL_1,
            pwm);

        __HAL_TIM_SET_COMPARE(
            &htim1,
            TIM_CHANNEL_2,
            2400);
    }
}

void HAL_TIM_PeriodElapsedCallback(
    TIM_HandleTypeDef *htim)
{
    if(htim == &htim2)
    {
        static int counter = 0;

        static float eint = 0;

        if(state == 1)
        {
            adc_position = read_adc();

            if(adc_position < 250 ||
               adc_position > (4095-250))
            {
                motor_off();

                state = 0;

                counter = 0;
                eint = 0;

                return;
            }

            current_raw =
                readINA219(
                    INA219_REG_CURRENT);

            int desired;

            if(counter < 100)
                desired = 200;
            else if(counter < 200)
                desired = -200;
            else if(counter < 300)
                desired = 200;
            else
                desired = -200;

            float error =
                desired - current_raw;

            eint += error;

            float u =
                kp*error +
                ki*eint;

            set_motor_pwm(u);

            data_index[counter] =
                counter;

            data_desired[counter] =
                desired;

            data_actual[counter] =
                current_raw;

            counter++;

            if(counter >= 400)
            {
                motor_off();

                state = 0;

                counter = 0;
                eint = 0;
            }
        }
    }
}


int main(void)
{
    HAL_Init();

    SystemClock_Config();

    MX_GPIO_Init();
    MX_ADC1_Init();
    MX_I2C2_Init();
    MX_TIM1_Init();
    MX_TIM2_Init();
    MX_USART3_UART_Init();

    init_ina219();

    HAL_TIM_PWM_Start(
        &htim1,
        TIM_CHANNEL_1);

    HAL_TIM_PWM_Start(
        &htim1,
        TIM_CHANNEL_2);

    motor_off();

    HAL_TIM_Base_Start_IT(
        &htim2);

    printf("\r\n");
    printf("HW16 Current Controller\r\n");

    while(1)
    {
        uint8_t c;

        if(HAL_UART_Receive(
                &huart3,
                &c,
                1,
                HAL_MAX_DELAY)
                == HAL_OK)
        {
            if(c == 'a')
            {
                state = 1;

                while(state);

                for(int i=0;i<400;i++)
                {
                    printf("%d,%d,%d\r\n",
                           data_index[i],
                           data_desired[i],
                           data_actual[i]);
                }
            }

            if(c == 'p')
            {
                adc_position =
                    read_adc();

                current_raw =
                    readINA219(
                        INA219_REG_CURRENT);

                printf("ADC=%lu CURRENT=%d\r\n",
                       adc_position,
                       current_raw);
            }
        }
    }
}