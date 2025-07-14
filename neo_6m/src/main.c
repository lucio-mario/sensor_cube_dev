#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#define BAUD 9600
#define MYUBRR ((F_CPU / (16UL * BAUD)) - 1)

// --- Funções para a Serial de SOFTWARE (Leitura do GPS no Pino 4) ---
// O GPS usa 9600 baud. O delay é calculado para "esperar" pelo centro de cada bit.
// Bit time = 1 / 9600 Hz = ~104 microssegundos.
#define GPS_BIT_DELAY 104

// Pino 4 do Arduino é o PIND4 no registrador de porta D
#define GPS_RX_PIN PIND4

void uart_init(void)
{
    UBRR0H = (uint8_t)(MYUBRR >> 8);
    UBRR0L = (uint8_t)(MYUBRR);
    UCSR0B = (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void uart_transmit_byte(uint8_t data)
{
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

void gps_init(void)
{
    DDRD &= ~(1 << GPS_RX_PIN);
    PORTD |= (1 << GPS_RX_PIN);
}

uint8_t gps_read_byte(void)
{
    uint8_t byte_read = 0;
    
    while((PIND & (1 << GPS_RX_PIN))); //Waits for start bit

    _delay_us(GPS_BIT_DELAY / 2); //Waits for half the time of one bit to centralize the read

    for (int i = 0; i < 8; i++)
    {
        _delay_us(GPS_BIT_DELAY);
        byte_read >>= 1; //shifts to the right
        if((PIND & (1 << GPS_RX_PIN))) //if pin is high, put 1 bit on the start of byte
            byte_read |= 0x80;
    }

    _delay_us(GPS_BIT_DELAY);

    return byte_read;
}


int main(void)
{
    uint8_t gps_byte;
    
    uart_init();
    gps_init();

    while(1)
    {
        gps_byte = gps_read_byte();
        uart_transmit_byte(gps_byte);
    }

    return 0;
}
