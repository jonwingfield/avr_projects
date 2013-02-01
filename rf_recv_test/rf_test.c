/*
    5-10-07
    Copyright Spark Fun Electronics© 2007
    Nathan Seidle
    nathan at sparkfun.com
    
    Example basic printf input/output
*/

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "VirtualWire/VirtualWire.h"
#include <util/delay.h>
#include <stdbool.h>
#include <string.h>
#include "dbg.h"
#include "lcd.h"

#define BAUD 9600
#define MYUBRR (F_CPU/16/(BAUD-1))

#define sbi(var, mask)   ((var) |= (uint8_t)(1 << mask))
#define cbi(var, mask)   ((var) &= (uint8_t)~(1 << mask))

//Define functions
//======================
void ioinit(void);      // initializes IO
static int uart_putchar(char c, FILE *stream);
uint8_t uart_getchar(void);

static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

ISR(BADISR_vect)
{
    printf("badisr\n");
}


int main (void)
{
    ioinit(); //Setup IO pins and defaults

    vw_set_rx_pin(PC4);
    // vw_set_ptt_pin(5);
    vw_setup(2000);
    sei();

    DDRC |= 0 << PC4;

    vw_rx_start();

    char msg[VW_MAX_MESSAGE_LEN+1];
    memset(msg, 0, VW_MAX_MESSAGE_LEN+1);
    uint8_t msglen = VW_MAX_MESSAGE_LEN;

    printf("Starting up!...\n");
    uint8_t i = 0;

    while (1) {

        if (vw_get_message((uint8_t*)msg, &msglen)) {
            printf("%s\n", msg);
        } 

        _delay_ms(50);
    }

    return(0);
}

// P15 in the manual
void ioinit (void)
{
    //1 = output, 0 = input
    DDRB = 0b11101000; //PB4 = MISO, PB3 = OC2
    DDRC = 0b11100001; //
    DDRD = 0;

    //USART Baud rate: 9600
    UBRR0H = MYUBRR >> 8;
    UBRR0L = MYUBRR;
    UCSR0B = (1<<RXEN0)|(1<<TXEN0);
    
    stdout = &mystdout; //Required for printf init
}

static int uart_putchar(char c, FILE *stream)
{
    if (c == '\n') uart_putchar('\r', stream);
  
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;
    
    return 0;
}

uint8_t uart_getchar(void)
{
    while( !(UCSR0A & (1<<RXC0)) && UDR0 <= 10) asm volatile("nop");
    return(UDR0);
}


