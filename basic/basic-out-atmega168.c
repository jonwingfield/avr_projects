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
#include <util/delay.h>
#include <stdbool.h>
#include "dbg.h"
#include "lcd.h"

#define FOSC 16000000
#define BAUD 9600
#define MYUBRR (FOSC/16/(BAUD-1))

#define sbi(var, mask)   ((var) |= (uint8_t)(1 << mask))
#define cbi(var, mask)   ((var) &= (uint8_t)~(1 << mask))

#define STATUS_LED 0

#define THERM_PORT PORTD
#define THERM_DDR  DDRD
#define THERM_PIN  PIND
#define THERM_DQ   PD2

#define THERM_INPUT_MODE()  THERM_DDR &= ~(1 << THERM_DQ)
#define THERM_OUTPUT_MODE() THERM_DDR |= (1 << THERM_DQ)
#define THERM_LOW()         THERM_PORT &= ~(1 << THERM_DQ)
#define THERM_HIGH()       THERM_PORT |= (1 << THERM_DQ)

//Define functions
//======================
void ioinit(void);      // initializes IO
static int uart_putchar(char c, FILE *stream);
uint8_t uart_getchar(void);

static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

double therm_read_temp(bool print);
//======================

static uint8_t hour = 0;
static uint8_t minute = 29;
static uint8_t second = 0;
static uint8_t month = 1;
static uint8_t day = 14;
static uint16_t year = 2013;
static uint8_t day_of_week = 0;

const char* days[] = { "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday" };

ISR(TIMER1_OVF_vect){
    // Restart Counter
    TCNT1H = 11;
    TCNT1L = 220;
    
    if (++second >= 60) {
        second = 0;
        if (++minute >= 60) {
            minute = 0;
            if (++ hour >= 24) {
                hour = 0;
                if (++day_of_week >= 7) {
                    day_of_week = 0;
                }
            }
        }
    }
}

void print_time()
{
    char time_str[32];
    uint8_t hour_12 = hour;
    if (hour_12 == 0) hour_12 = 12;
    if (hour_12 > 12) hour_12 -= 12;
    snprintf(time_str, 31, "%.3s %2d:%02d:%02d %s ", days[day_of_week], hour_12, minute, second, hour > 12 ? "PM" : "AM");
    lcd_print(time_str, 1);
}

void print_message()
{
    if (hour == 10 || hour == 15) {
        lcd_print("Snack time!     ", 2);
    } else if (hour < 7 || (hour == 7 && minute < 30) || hour >= 19) {
        lcd_print("Bedtime, bucko  ", 2);
    } else if (hour < 11) {
        lcd_print("Morning, Aaron! ", 2);
    } else if (hour < 18) {
        lcd_print("Afternoon time  ", 2);
    } else if (hour < 19) {
        lcd_print("Dinner is soon  ", 2);
    } 
}

typedef enum {
    MODE_TEMP,
    MODE_MESSAGE,
    MODE_DATE
} MODES;

int main (void)
{
    ioinit(); //Setup IO pins and defaults

    init_lcd();
    lcd_print("ATMEGA!", 1);

    OCR2A  = 0x00;
    TCCR2A = 0b10000011;
    TCCR2B = 0b00000001;

    double delta_t = 0.0;

    /* timer stuffs */
    TCCR1B = (1<<CS12);  // clock divider = 256 
    // TCNT1 = 0x8000 - (FOSC / 256);   // i think this is what we're seeing below
    TCNT1H = 11;
    TCNT1L = 220;
    TIMSK1 |= (1<<TOIE1);

    sei();

    MODES mode = MODE_TEMP;

    while(1)
    {

        double temp = therm_read_temp(mode == MODE_TEMP);	
        delta_t = temp - 78.0;
        if (delta_t < 0) delta_t = 0;

        delta_t *= ((double)0x25 / 2);

        OCR2A = (uint8_t)delta_t;

        uint8_t last_minute = 0;
        uint8_t last_hour = 0;

        for (int i = 0; i < 40; ++i)
        {
            if (i > 1) { _delay_us(250000); }

            if (PINB & (1 << PB1)) {
                if (last_minute == 0 || last_minute > 5) {
                    if (++minute >= 60) {
                        minute = 0;
                    }
                    print_time();
                }
                ++last_minute;
            } else if (PINB & (1 << PB2)) {
                if (last_hour == 0 || last_hour > 5) {
                    if (++hour >= 24) {
                        hour = 0;
                    }
                    print_time();
                }
                ++last_hour;
            } else {
                last_hour = last_minute = 0;

                if (PINB & (1 << PB0)) {
                    // change mode
                    if (mode == MODE_TEMP) {
                        mode = MODE_MESSAGE;
                        print_message();
                    } else {
                        mode = MODE_TEMP;
                        therm_read_temp(true);
                    }

                }
            }

            if (i % 4 == 0) {
                print_time();

                if (mode == MODE_MESSAGE) {
                    print_message();
                }
            }
        }
    }
   
    return(0);
}

// P15 in the manual
uint8_t therm_reset(void) 
{
    // send reset pulse 
    THERM_LOW();
    THERM_OUTPUT_MODE();
    _delay_us(480);

    // wait for presence pulse
    THERM_INPUT_MODE();
    _delay_us(60);

    // it's low if we're OK, high otherwise (pullup should keep it high)
    uint8_t i = (THERM_PIN & (1 << THERM_DQ));
    _delay_us(420);

    return i;
}

void therm_write_bit(uint8_t bit)
{
    THERM_LOW();
    THERM_OUTPUT_MODE();
    _delay_us(5);

    if (bit) THERM_INPUT_MODE();
    _delay_us(56);

    THERM_INPUT_MODE();
}

uint8_t therm_read_bit(void)
{
    THERM_OUTPUT_MODE();
    THERM_LOW();
    _delay_us(1);

    THERM_INPUT_MODE();
    _delay_us(14);

    uint8_t i = (THERM_PIN & (1 << THERM_DQ)) ? 1 : 0;

    _delay_us(45);

    return i;
}

void therm_write(uint8_t data)
{
    for (int i=0; i<8; i++)
    {
        therm_write_bit(data & (1 << i));
    }
}

uint8_t therm_read(void)
{
    uint8_t data = 0;

    for (int i=0; i<8; i++) {
        data |= therm_read_bit() << i;
    }

    return data;
}

#define SEARCH_ROM      0xf0
#define READ_ROM        0x33
#define MATCH_ROM       0x55
#define SKIP_ROM        0xcc
#define ALARM_SEARCH    0xec

#define CONVERT_T       0x44
#define WRITE_SCRATCH   0x4e
#define READ_SCRATCH    0xbe
#define COPY_SCRATCH    0x48
#define RECALL_E2       0xb8
#define READ_POWER_SUPPLY 0xb4

void therm_command(uint8_t command)
{
    therm_write(command);
}

double therm_read_temp(bool print)
{
    debug("Reading temperature...");

    if (therm_reset()) {
        sentinel("Error resetting temp!");
    }

    therm_command(SKIP_ROM);
    therm_command(CONVERT_T);

    while (!therm_read_bit());

    debug("Temp conversion finished, reading...");   

    if (therm_reset()) {
        sentinel("Error resetting temp after conversion!");
    }

    therm_command(SKIP_ROM);
    therm_command(READ_SCRATCH);

    uint16_t temp = 0;

    temp = therm_read();
    temp |= (therm_read() & 0x0F) << 8;  // disregard the top 4 bits, and shift into MSD
    debug("%x", temp);

    if (therm_reset()) { 
        sentinel("Error resetting after reading!");
    }

    uint16_t major = (temp & 0xFF0) >> 4;
    uint16_t minor = (temp & 0xF) * 625;

    double farenheit = (double)temp * 0.0625 * 1.8 + 32;

    printf("Temp: %d.%dC %0.1fF", major, minor, farenheit);

    char tempstr[64];
    snprintf(tempstr, 63, "Temp %d.%dC %0.1fF", major, minor / 1000, farenheit);

    if (print) {
        lcd_print(tempstr, 2);
    }

    return farenheit;

error:
    return 0;
}

void ioinit (void)
{
    //1 = output, 0 = input
    DDRB = 0b11101000; //PB4 = MISO, PB3 = OC2
    DDRC = 0b11100001; //
    DDRD = 0b11111110; //PORTD (RX on PD0)

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

