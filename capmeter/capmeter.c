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
// #include <math.h>
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

uint16_t last_timer = 0;
uint8_t timing = 1;

uint16_t readings[3] = { 0, 0, 0 };
uint8_t reading_num = 0;

/*
    C = t / ( R * ln(Vh / Vl) )

    t = ticks / 16  (in units of us)

    R = 1 MΩ

    C = t / ln_vdiff
*/

#define turn_on_cap()     sbi(PORTC, PC0)
#define turn_off_cap()    cbi(PORTC, PC0)


#define V1      4.62
#define V2      0.43
#define LN_V1_V2 2.51022445777229
#define LN_TIMES_CYCLES ln_vdiff * 0.8 // 4.01635913243566 / 2.0  // LN_V1_V2 * 1.6   (divide by 10 to convert to )
#define CYCLES_TO_USEC  ((double)FOSC / 1000000.0)

const double ln_vdiff = 1 - (log(V1 * V2));

// returns capacitance in pF * 10 (10^-13)
uint16_t calc_capacitance(uint16_t ticks)
{
    printf("%u ticks\n", ticks);
    return (uint16_t)(((double)ticks / (LN_TIMES_CYCLES)));
    // return (uint16_t)((double)ticks / (1.6 * .98));
}

ISR(ANALOG_COMP_vect)
{
    uint16_t ticks = TCNT1;

    if (!(ACSR & (1 << ACO))) return;

    // if (!timing) {
    //     last_timer = TCNT1;
    //     sbi(ADCSRB, ACME); // turn on Analog compare multiplexer
    //     timing = 1;
    // } else {

        uint16_t pf = calc_capacitance(ticks);

        // if ((reading_num % 3) == 2) {
            debug("Cap read...");

            char tests[32];
            snprintf(tests, 31, "%d.%d pF          ", pf / 10, pf % 10);
            printf("%s", tests);
            printf("%s\n", tests);
        // } 

        turn_off_cap();
        cbi(ADCSRB, ACME); // turn off analog compare mux
        timing = 0;

        // if ((reading_num  % 3) != 2) {
        //     _delay_ms(100);

        //     PORTD |= (1 << PD3);
        // }

        reading_num++;
    // }
}

ISR(BADISR_vect)
{
    lcd_print("badisr", 2);
    printf("badisr\n");
}

ISR(TIMER1_CAPT_vect)
{
    // shouldn't trigger
}

//======================

int main (void)
{
    ioinit(); //Setup IO pins and defaults

    DDRD |= 1 << PD4;

    // init_lcd();
    // lcd_print("CapMeter!", 1);
    printf("Capmeter initialized\n");

    OCR2A  = 0x00;
    TCCR2A = 0b10000011;
    TCCR2B = 0b00000001;

    double delta_t = 0.0;

    cbi(ADCSRA, ADEN); // turn off ADC

    /* timer stuffs */
    TCCR1B |= (1 << CS10);  // no clock divider
    // TCNT1 = 0x8000 - (FOSC / 256);   // i think this is what we're seeing below
    // TCNT1H = 11;
    // TCNT1L = 220;
    // TIMSK1 |= (1<<TOIE1);

    // TIMSK1 |= (1 << ICIE1);
    ACSR |= 1 << ACIE;
    // ADMUX |= 0b100; // set ADC4 as analog compare negative input

    DDRC |= (1 << PC0); // pinout to start measuring
    turn_off_cap();

    _delay_ms(1000);

    sei();

    while (1) {
        _delay_ms(1000);
        if (ACSR & (1 << ACO)) {
            debug("ACO is set!");
        } else {
            debug("Reading cap...");
            _delay_ms(1000);
            sbi(ADCSRB, ACME); // turn on Analog compare multiplexer
            // turn_on_cap();
            TCNT1 = 0;
            
            // while (!(ACSR & (1 << ACO)));

            // uint16_t count1 = last_timer = TCNT1;
            // sbi(ADCSRB, ACME); // turn on Analog compare multiplexer

            // _delay_us(10); // wait a few cycles

            // while (!(ACSR & (1 << ACO)));

            // uint16_t ticks = TCNT1 - count1;
            // lcd_print("Cap read...", 1);

            // char tests[32];
            // snprintf(tests, 31, "ticks: %d        ", (int)(ticks/16));
            // lcd_print(tests, 2);

            // PORTD &= ~(1 << PD3);
            // cbi(ADCSRB, ACME); // read from the analog compare input

            // _delay_us(100000);
            _delay_ms(3000);
        }
    }

    return(0);
}

// P15 in the manual
void ioinit (void)
{
    //1 = output, 0 = input
    DDRB = 0b00001000; //PB4 = MISO, PB3 = OC2
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


