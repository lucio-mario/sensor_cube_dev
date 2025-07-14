#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <util/twi.h>
#include <stdio.h>

#define MPU6050_ADDR 0x68
#define PWR_MGMT_1_REG 0x6B
#define ACCEL_XOUT_H_REG 0x3B
#define GYRO_XOUT_H_REG 0x43
#define ACCEL_SCALE_FACTOR_2G 16384.0
#define GYRO_SCALE_FACTOR_250DPS 131.0
#define TEMP_SCALE_FACTOR 340.0
#define TEMP_OFFSET 36.53

#define BAUD 9600
#define MYUBRR ((F_CPU / (16UL * BAUD)) - 1)

void uart_init(void)
{
    UBRR0H = (uint8_t)(MYUBRR >> 8);
    UBRR0L = (uint8_t)(MYUBRR);
    UCSR0B = (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void uart_transmit_byte(uint8_t data)
{
    while(!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

void uart_transmit_string(const char *data)
{
    while(*data)
        uart_transmit_byte(*data++);
}

void i2c_init(void)
{
    // SCL frequency = F_CPU / (16 + 2 * TWBR * Prescaler)
    // Para 100kHz, com Prescaler = 1: TWBR = 72
    TWSR = 0x00; // Define prescaler como 1
    TWBR = 72;   // Define o bit rate
    TWCR = (1 << TWEN); // Habilita a interface TWI (I2C)
}

void i2c_start(void)
{
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    while(!(TWCR & (1 << TWINT)));
}

void i2c_stop(void)
{
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}

void i2c_write(uint8_t data)
{
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
}

uint8_t i2c_read_ack(void)
{
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
    while(!(TWCR & (1 << TWINT)));
    return TWDR;
}

uint8_t i2c_read_nack(void)
{
    TWCR = (1 << TWINT) | (1 << TWEN);
    while(!(TWCR & (1 << TWINT)));
    return TWDR;
}

void mpu6050_init(void)
{
    i2c_start();
    i2c_write((MPU6050_ADDR << 1) | 0);
    i2c_write(PWR_MGMT_1_REG);
    i2c_write(0x00);
    i2c_stop();
}

int main(void)
{
    char buffer[150];
    
    int16_t accel_x, accel_y, accel_z;
    int16_t temp_raw_mpu;
    int16_t gyro_x, gyro_y, gyro_z;
   
    long gyro_x_cal = 0, gyro_y_cal = 0, gyro_z_cal = 0; 
    
    float accel_x_g, accel_y_g, accel_z_g;
    float temp_conv_mpu;
    float gyro_x_dps, gyro_y_dps, gyro_z_dps;

    uart_init();
    i2c_init();
    mpu6050_init();

    uart_transmit_string("Starting gyroscope calibration... Maintain sensor put.\r\n");
    _delay_ms(1000);

    for (int i = 0; i < 2000; i++)
    {
        i2c_start();
        i2c_write((MPU6050_ADDR << 1) | 0);
        i2c_write(0x43);
        i2c_stop();

        i2c_start();
        i2c_write((MPU6050_ADDR << 1) | 1);
        gyro_x_cal += (i2c_read_ack() << 8) | i2c_read_ack();
        gyro_y_cal += (i2c_read_ack() << 8) | i2c_read_ack();
        gyro_z_cal += (i2c_read_ack() << 8) | i2c_read_nack();
        i2c_stop();
        _delay_ms(1);
    }
    gyro_x_cal /= 2000;
    gyro_y_cal /= 2000;
    gyro_z_cal /= 2000;

    uart_transmit_string("Calibration done!\r\n");

    uart_transmit_string("MPU6050 ready.\r\n");
    _delay_ms(1000);

    while(1)
    {
        i2c_start();
        i2c_write((MPU6050_ADDR << 1) | 0);
        i2c_write(ACCEL_XOUT_H_REG);
        i2c_stop();

        i2c_start();
        i2c_write((MPU6050_ADDR << 1) | 1);
        
        accel_x = (i2c_read_ack() << 8) | i2c_read_ack();
        accel_y = (i2c_read_ack() << 8) | i2c_read_ack();
        accel_z = (i2c_read_ack() << 8) | i2c_read_ack();
        
        temp_raw_mpu = (i2c_read_ack() << 8) | i2c_read_ack();

        gyro_x = (i2c_read_ack() << 8) | i2c_read_ack();
        gyro_y = (i2c_read_ack() << 8) | i2c_read_ack();
        gyro_z = (i2c_read_ack() << 8) | i2c_read_nack();
        i2c_stop();
        
        gyro_x -= gyro_x_cal;
        gyro_y -= gyro_y_cal;
        gyro_z -= gyro_z_cal;

        accel_x_g = accel_x / ACCEL_SCALE_FACTOR_2G;
        accel_y_g = accel_y / ACCEL_SCALE_FACTOR_2G;
        accel_z_g = accel_z / ACCEL_SCALE_FACTOR_2G;

        temp_conv_mpu = (temp_raw_mpu / TEMP_SCALE_FACTOR) + TEMP_OFFSET;
        
        gyro_x_dps = gyro_x / GYRO_SCALE_FACTOR_250DPS;
        gyro_y_dps = gyro_y / GYRO_SCALE_FACTOR_250DPS;
        gyro_z_dps = gyro_z / GYRO_SCALE_FACTOR_250DPS;

        sprintf(buffer, "%ld,%ld,%ld,%ld,%ld,%ld,%ld\n",
                (long)(accel_x_g * 100),
                (long)(accel_y_g * 100),
                (long)(accel_z_g * 100),
                (long)(temp_conv_mpu * 10),
                (long)(gyro_x_dps * 10),
                (long)(gyro_y_dps * 10),
                (long)(gyro_z_dps * 10));
        uart_transmit_string(buffer);

        _delay_ms(10);
    }

    return 0;
}
