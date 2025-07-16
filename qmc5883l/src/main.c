#define F_CPU 16000000UL

#include <math.h>
#include <avr/io.h>
#include <util/delay.h>
#include <util/twi.h>
#include <stdio.h>

#define QMC5883L_ADDR             0x0D
#define QMC5883L_DATA_REG         0x00
#define QMC5883L_STATUS_REG       0x06
#define QMC5883L_CONTROL_REG_1    0x09
#define QMC5883L_CONTROL_REG_2    0x0A
#define MAG_SCALE_FACTOR          12000.0
#define MAGNETIC_DECLINATION_DEG -23.88

// Hard-iron offsets
#define CAL_HI_OFFSET_X  (512.0000f)
#define CAL_HI_OFFSET_Y  (-10.0000f)
#define CAL_HI_OFFSET_Z  (1237.5000f)

// Soft-iron correction matrix (diagonal matrix of scales)
static const float cal_si_matrix[3][3] = {
    { 1.0092f, 0.0000f, 0.0000f },
    { 0.0000f, 0.7260f, 0.0000f },
    { 0.0000f, 0.0000f, 1.5829f }
};

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define BAUD 9600
#define MYUBRR ((F_CPU / (16UL * BAUD)) - 1)

void uart_init(void)
{
    UBRR0H = (uint8_t)(MYUBRR >> 8);
    UBRR0L = (uint8_t)(MYUBRR);
    UCSR0B = (1 << TXEN0) | (1 << RXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void uart_transmit_byte(uint8_t data)
{
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

void uart_transmit_string(const char *data)
{
    while (*data)
        uart_transmit_byte(*data++);
}

void i2c_init(void)
{
    TWSR = 0x00;
    TWBR = 72;
    TWCR = (1 << TWEN);
}

void i2c_start(void)
{
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
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
    while (!(TWCR & (1 << TWINT)));
    return TWDR;
}

uint8_t i2c_read_nack(void)
{
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
    return TWDR;
}

void qmc5883l_init(void)
{
    i2c_start();
    i2c_write((QMC5883L_ADDR << 1) | 0);
    i2c_write(QMC5883L_CONTROL_REG_1);
    i2c_write(0x1D);
    i2c_stop();
}

int main(void)
{
    char buffer[100];
    
    int16_t mag_x_raw, mag_y_raw, mag_z_raw;
    float mag_x_cal, mag_y_cal, mag_z_cal;
    float heading_mag, heading_true;

    uart_init();
    i2c_init();
    qmc5883l_init();

    _delay_ms(100);

    while(1)
    {
        i2c_start();
        i2c_write((QMC5883L_ADDR << 1) | 0);

        i2c_start();
        i2c_write((QMC5883L_ADDR << 1) | 1);
        
        uint8_t mag_x_lsb = i2c_read_ack();
        uint8_t mag_x_msb = i2c_read_ack();
        uint8_t mag_y_lsb = i2c_read_ack();
        uint8_t mag_y_msb = i2c_read_ack();
        uint8_t mag_z_lsb = i2c_read_ack();
        uint8_t mag_z_msb = i2c_read_nack();
        i2c_stop();

        mag_x_raw = (mag_x_msb << 8) | mag_x_lsb;
        mag_y_raw = (mag_y_msb << 8) | mag_y_lsb;
        mag_z_raw = (mag_z_msb << 8) | mag_z_lsb;

        float temp_x = (float)mag_x_raw - CAL_HI_OFFSET_X;
        float temp_y = (float)mag_y_raw - CAL_HI_OFFSET_Y;
        float temp_z = (float)mag_z_raw - CAL_HI_OFFSET_Z;

        mag_x_cal = cal_si_matrix[0][0] * temp_x + cal_si_matrix[0][1] * temp_y + cal_si_matrix[0][2] * temp_z;
        mag_y_cal = cal_si_matrix[1][0] * temp_x + cal_si_matrix[1][1] * temp_y + cal_si_matrix[1][2] * temp_z;
        mag_z_cal = cal_si_matrix[2][0] * temp_x + cal_si_matrix[2][1] * temp_y + cal_si_matrix[2][2] * temp_z;

        heading_mag = atan2(mag_y_cal, mag_x_cal) * (180.0 / M_PI);
        heading_true = heading_mag + MAGNETIC_DECLINATION_DEG;
        if (heading_true < 0)
            heading_true += 360;
        else if (heading_true >= 360)
            heading_true -= 360;

        sprintf(buffer, "%ld,%ld,%ld,%ld\n",
                (long)(mag_x_cal),
                (long)(mag_y_cal),
                (long)(mag_z_cal),
                (long)(heading_true * 10));
        uart_transmit_string(buffer);

        _delay_ms(10);
    }

    return 0;
}
