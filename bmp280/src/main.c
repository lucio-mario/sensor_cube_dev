#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <util/twi.h>
#include <stdio.h>

#define BMP280_ADDR 0x76
#define BMP280_REG_ID 0xD0
#define BMP280_REG_CTRL_MEAS 0xF4
#define BMP280_REG_CONFIG 0xF5
#define BMP280_REG_PRESS_MSB 0xF7
#define BMP280_REG_CALIB_START 0x88

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
    TWSR = 0x00; // Define prescaler como 1
    TWBR = 72;   // Define o bit rate
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

uint16_t dig_T1;
int16_t  dig_T2, dig_T3;
uint16_t dig_P1;
int16_t  dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
int32_t t_fine;

void bmp280_read_calibration_data()
{
    i2c_start();
    i2c_write((BMP280_ADDR << 1) | 0);
    i2c_write(BMP280_REG_CALIB_START);
    i2c_stop();

    i2c_start();
    i2c_write((BMP280_ADDR << 1) | 1);
    dig_T1 = (i2c_read_ack() | (i2c_read_ack() << 8));
    dig_T2 = (i2c_read_ack() | (i2c_read_ack() << 8));
    dig_T3 = (i2c_read_ack() | (i2c_read_ack() << 8));
    dig_P1 = (i2c_read_ack() | (i2c_read_ack() << 8));
    dig_P2 = (i2c_read_ack() | (i2c_read_ack() << 8));
    dig_P3 = (i2c_read_ack() | (i2c_read_ack() << 8));
    dig_P4 = (i2c_read_ack() | (i2c_read_ack() << 8));
    dig_P5 = (i2c_read_ack() | (i2c_read_ack() << 8));
    dig_P6 = (i2c_read_ack() | (i2c_read_ack() << 8));
    dig_P7 = (i2c_read_ack() | (i2c_read_ack() << 8));
    dig_P8 = (i2c_read_ack() | (i2c_read_ack() << 8));
    dig_P9 = (i2c_read_ack() | (i2c_read_nack() << 8));
    i2c_stop();
}

void bmp280_init()
{
    bmp280_read_calibration_data();

    i2c_start();
    i2c_write((BMP280_ADDR << 1) | 0);
    i2c_write(BMP280_REG_CTRL_MEAS);
    i2c_write(0x57);
    i2c_stop();

    i2c_start();
    i2c_write((BMP280_ADDR << 1) | 0);
    i2c_write(BMP280_REG_CONFIG);
    i2c_write(0x10);
    i2c_stop();
}

int32_t bmp280_compensate_T(int32_t adc_T)
{
    int32_t var1, var2, T;
    var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) * ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;
    return T;
}

uint32_t bmp280_compensate_P(int32_t adc_P)
{
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)dig_P6;
    var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
    var2 = var2 + (((int64_t)dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;
    if(var1 == 0)
        return 0;
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);
    return (uint32_t)p;
}

int main(void)
{
    char buffer[100];

    int32_t adc_T, adc_P;
    int32_t temp_bmp;
    uint32_t press_bmp;

    uart_init();
    i2c_init();
    bmp280_init();

    _delay_ms(1000);

    while(1)
    {
        i2c_start();
        i2c_write((BMP280_ADDR << 1) | 0);
        i2c_write(BMP280_REG_PRESS_MSB);
        i2c_stop();

        i2c_start();
        i2c_write((BMP280_ADDR << 1) | 1);
        uint32_t p_msb = i2c_read_ack();
        uint32_t p_lsb = i2c_read_ack();
        uint32_t p_xlsb = i2c_read_ack();
        uint32_t t_msb = i2c_read_ack();
        uint32_t t_lsb = i2c_read_ack();
        uint32_t t_xlsb = i2c_read_nack();
        i2c_stop();

        adc_P = (p_msb << 12) | (p_lsb << 4) | (p_xlsb >> 4);
        adc_T = (t_msb << 12) | (t_lsb << 4) | (t_xlsb >> 4);
        
        temp_bmp = bmp280_compensate_T(adc_T);
        press_bmp = bmp280_compensate_P(adc_P) / 256;

        sprintf(buffer,"%ld,%lu\n",
                (long)temp_bmp,
                (unsigned long)press_bmp);
        uart_transmit_string(buffer);

        _delay_ms(10); // Controla a taxa de atualização
    }

    return 0;
}
