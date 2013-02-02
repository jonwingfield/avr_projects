
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
bool reading = false;

/*
    C = t / ( R * ln(Vh / Vl) )

    t = ticks / 16  (in units of us)

    R = 1 MΩ

    C = t / ln_vdiff
*/

#define turn_on_cap()     sbi(PORTC, PC0)
#define turn_off_cap()    cbi(PORTC, PC0)

#define CAP_AT_55   3535
#define VRATIO  0.82456140350877    // 1 - (v_cap / v_source), defined by the voltage 
                                    // divider circuit (in this case 470 ohms / 100 ohms) 
#define LN_V1_V2 0.26469255422708   // ln (1 / VRATIO)
#define LN_TIMES_CYCLES LN_V1_V2 * 1.6 // 4.01635913243566 / 2.0  // LN_V1_V2 * 1.6   (divide by 10 to convert to )
#define CYCLES_TO_USEC  ((double)FOSC / 1000000.0)

// const double ln_vdiff = 1 - (log(V1 * V2));

// returns capacitance in pF * 10 (10^-13)
uint16_t calc_capacitance(uint16_t ticks)
{
    // printf("%u ticks\n", ticks);
    return (uint16_t)(((double)ticks / (LN_TIMES_CYCLES)));
    // return (uint16_t)((double)ticks / (1.6 * .98));
}

uint16_t calc_humidity(uint16_t pf)
{
    int diff = ((int)pf - CAP_AT_55) / 6;
    printf("diff %d\n", diff);
    return 55 + (uint16_t)diff;
}

#define NUM_READINGS 8
uint16_t readings[NUM_READINGS] = { 0 };
uint8_t reading_num = 0;

ISR(ANALOG_COMP_vect)
{
    uint16_t ticks = TCNT1;

    if (!(ACSR & (1 << ACO))) return;

    uint16_t pf = calc_capacitance(ticks);

    turn_off_cap();
    cbi(ADCSRB, ACME); // turn off analog compare mux

    readings[reading_num] = pf;
    if (++reading_num >= NUM_READINGS) {
        reading_num = 0;
    }

    reading = false;
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
    // PORTD |= 1 << PD4;

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

    ACSR |= 1 << ACIE;
    ADMUX |= 0b100; // set ADC4 as analog compare negative input

    DDRC |= (1 << PC0); // pinout to start measuring
    DDRC &= ~(1 << PC1);
    turn_off_cap();

    _delay_ms(1000);

    sei();

    while (1) {
        _delay_ms(1000);
        if (ACSR & (1 << ACO)) {
            debug("ACO is set!");
        } else {
            debug("Reading cap...");

            for (int i=0; i<NUM_READINGS; i++) {
                sbi(ADCSRB, ACME); // turn on Analog compare multiplexer
                reading = true;
                turn_on_cap();
                TCNT1 = 0;

                while (reading) _delay_ms(1);

                // let it discharge
                _delay_ms(10);
            }
            
            uint16_t avg_reading = 0;
            for (int i=0; i<NUM_READINGS; i++) avg_reading += readings[i];
            avg_reading /= NUM_READINGS;

            // measure temp

                cbi(PRR, PRADC);
                sbi(ADCSRA, ADEN); // turn on ADC
                cbi(ACSR, ACIE); 
                ADCSRA |= 0b111; // set prescaler to 128 (16Mhz / 128 = 125Khz)
                ADMUX &= 0b000; // set ADC1
                ADMUX |= 0b001;

                sbi(ADCSRA, ADSC); // start conversion

                while (ADCSRA & ADSC) { printf("waiting...\n"); _delay_ms(1); }

                printf("Temp conversion result %u\n", ADC);

                ADMUX &= 0b000; // set ADC4 as analog compare negative input
                ADMUX |= 0b100;
                sbi(ACSR, ACIE);
                cbi(ADCSRA, ADEN); // turn off ADC


            printf("avg: %upF %u%% Humidity\n", avg_reading, calc_humidity(avg_reading));



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


