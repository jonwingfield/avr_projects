/*
    5-10-07
    Copyright Spark Fun Electronics© 2007
    Nathan Seidle
    nathan at sparkfun.com
    
    Example basic printf input/output
*/

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include "VirtualWire/VirtualWire.h"
#include "dbg.h"
#include "weather_info.h"

#define BAUD 9600
#define MYUBRR (F_CPU/16/(BAUD-1))

#define sbi(var, mask)   ((var) |= (uint8_t)(1 << mask))
#define cbi(var, mask)   ((var) &= (uint8_t)~(1 << mask))

#define c_to_f(temp)   (uint16_t)(9.0 / 5.0 * (double)(temp) + 320.0)

#define MAX_SAME_READINGS 10

//Define functions
//======================
void ioinit(void);      // initializes IO
static int uart_putchar(char c, FILE *stream);
uint8_t uart_getchar(void);
void reset(void);

bool is_repeat_reading_bug(weather_info* winfo);
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

    while (1) {

        if (vw_get_message((uint8_t*)msg, &msglen)) {
            weather_info* winfo = (weather_info*)msg;

            printf("Sensor Id: %d, Temperature: %u.%u, Humidity: %u.%u%%\n", 
                winfo->sensor_id,
                winfo->temperature_times_10 / 10,
                winfo->temperature_times_10 % 10,
                winfo->humidity_times_10 / 10,
                winfo->humidity_times_10 % 10);
        } 

        memset(msg, 0, VW_MAX_MESSAGE_LEN+1);
        msglen = VW_MAX_MESSAGE_LEN;
        _delay_ms(100);
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

bool is_repeat_reading_bug(weather_info* winfo)
{
    static uint8_t same_temp = 0;
    static uint8_t same_humidity = 0;
    static weather_info last_winfo = { .temperature_times_10 = 0, .humidity_times_10 = 0 };

    if (winfo->temperature_times_10 == last_winfo.temperature_times_10) same_temp++;
    else same_temp = 0;
    if (winfo->humidity_times_10 == last_winfo.humidity_times_10) same_humidity++;
    else same_humidity = 0;

    last_winfo = *winfo;

    if (same_temp > MAX_SAME_READINGS || same_humidity > MAX_SAME_READINGS) {
        return true;
    }

    return false;
}

void reset(void)
{
  wdt_disable();  
  wdt_enable(WDTO_15MS);
  while (1) {}
}

