#define F_CPU 16000000UL //External clock frequency used in the Arduino Uno crystal oscillator

#include <avr/io.h>
#include <util/delay.h>

#define BAUD 9600 //Baud rate in bits per second
#define MYUBRR ((F_CPU / (16UL * BAUD)) - 1) //Calculates the UBRR value used to determine the 'baud clock' frequency

void uart_init(void)
{
    UBRR0H = (uint8_t)(MYUBRR >> 8); //Puts the 8 most significant bits of calculated UBRR
    UBRR0L = (uint8_t)(MYUBRR); //Puts the 8 least significant bits of calculated UBRR
    UCSR0A &= ~(1 << U2X0); //Disable asynchrounous double speed mode
    UCSR0B |= (1 << TXEN0); //Enables USART transmitter
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); //Configures the data package bits (8-bit data, 1 stop bit, no parity)
}

void uart_transmit_byte(const char data)
{
    while(!(UCSR0A & (1 << UDRE0))) ; //loops until UDR0 (data buffer) is empty
    UDR0 = data; //Puts data in data buffer
}

void uart_transmit_string(const char *data)
{
    while(*data)
        uart_transmit_byte(*data++);
}

int main(void)
{
    uart_init();
   
    while(1)
    {
        uart_transmit_string("hello\r\n");
        _delay_ms(1000);
    }
    
    return 0;
}
