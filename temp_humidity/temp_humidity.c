
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>
#include "dbg.h"
#include "lcd.h"

#define BAUD 9600
#define MYUBRR (F_CPU/16/(BAUD-1))

#define sbi(var, mask)   ((var) |= (uint8_t)(1 << mask))
#define cbi(var, mask)   ((var) &= (uint8_t)~(1 << mask))
#define round(fp) (int)((fp) >= 0 ? (fp) + 0.5 : (fp) - 0.5)

//Define functions
//======================
void ioinit(void);      // initializes IO
static int uart_putchar(char c, FILE *stream);
uint8_t uart_getchar(void);

static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

uint16_t last_timer = 0;
bool reading = false;

/*

    Vc = Vs(1 - e ^ (-t/RC))

    C = t / (ln(1 - Vc/Vs) * R)

*/

#define turn_on_cap()     sbi(PORTD, PD2)
#define turn_off_cap()    cbi(PORTD, PD2)

#define CAP_AT_55   3364
#define PF_PER_PERCENT_HUMIDITY  0.65
#define VRATIO  0.82456140350877    // 1 - (v_cap / v_source), defined by the voltage 
                                    // divider circuit (in this case 470 ohms / 100 ohms) 
#define LN_V1_V2 0.693147180559945 // 0.26469255422708   // ln (1 / VRATIO)  0.693147180559945 @ .5 of Vs
#define LN_TIMES_CYCLES LN_V1_V2 * 1.6  // this is actually in .1pF units (for convenience)
#define CYCLES_TO_USEC  ((double)FOSC / 1000000.0)

// const double ln_vdiff = 1 - (log(V1 * V2));

// returns capacitance in pF * 10 (10^-13F) (for precision)
uint16_t calc_capacitance(uint16_t ticks)
{
    // printf("%u ticks\n", ticks);
    return (uint16_t)(((double)ticks / (LN_TIMES_CYCLES)));
    // return (uint16_t)((double)ticks / (1.6 * .98));
}

// returns humidity in %RH * 10 (for precision)
uint16_t calc_humidity(double pf_times_10, 
    uint16_t temp_c_times_10)
{
    // numbers are calculated at 
    double temp_adjust = 4.0 - (double)temp_c_times_10 * .016;

    double diff = (pf_times_10 - CAP_AT_55) / (PF_PER_PERCENT_HUMIDITY * 10.0);  
    return (uint16_t)(((55 + diff) + temp_adjust) * 10);
}

#define NUM_READINGS 8
#define NUM_TO_AVG   8
uint16_t readings[NUM_READINGS] = { 0 };
uint8_t reading_num = 0;
uint8_t display_num = 0;
uint16_t displays[NUM_TO_AVG] = { 0 };

uint16_t last_ticks = 0;

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

    printf("%u ticks\n", ticks);
    last_ticks = ticks;

    reading = false;
}

ISR(BADISR_vect)
{
    lcd_print("badisr", 2);
    printf("badisr\n");
}

#define ADC_STEPS_PER_VOLT 919.9
#define TEMPS_TO_AVG   8
uint8_t temp_reading_num = 0;
uint16_t temps[TEMPS_TO_AVG] = { 0 };

uint16_t calc_temp()
{
    // measure temp
    cbi(PRR, PRADC);
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // set prescaler to 128 (16Mhz / 128 = 125Khz)
    // ADMUX |= (1 << REFS0);
    ADMUX &= 0; // set ADC1
    ADMUX |= 0b001;
    ADMUX |= (1 << REFS1) | (1 << REFS0);   // use internal 1.1v reference
    cbi(ACSR, ACIE); 
    sbi(ADCSRA, ADEN); // turn on ADC
    sbi(ADCSRA, ADSC); // start conversion

    int max = 0;
    uint16_t result = 0;
    while (!(ADCSRA & ADIF) && (++max < 500)) { _delay_ms(1); }
    if (max >= 500) printf("Failed\n");
    else {
        result |= ADCL;
        result |= (ADCH & 3) << 8;
        printf("Temp conversion result %u\n", result);
    }

    ADMUX &= 0; // set ADC4 as analog compare negative input
    ADMUX |= 0b100;
    sbi(ACSR, ACIE);
    cbi(ADCSRA, ADEN); // turn off ADC

    if (result == 0) return result;

    double temp = (double)result / ADC_STEPS_PER_VOLT;
    // factory is .6V, I added X V (XC) to SUBTRACT XC to calibrate based on my sensors
    temp -= 0.605;
    temp /= 0.01;

    uint16_t temp_int = (uint16_t)(temp * 10);

    printf("This temp: %u.%u\n", temp_int / 10, temp_int % 10);

    temps[temp_reading_num] = temp_int;
    if (++temp_reading_num >= TEMPS_TO_AVG) temp_reading_num = 0;

    int i = 0;
    uint16_t temp_avg = 0;
    for (i = 0; i < TEMPS_TO_AVG && temps[i]; ++i)
    {
        temp_avg += temps[i];
    }

    return (temp_avg / i);
}

//======================

int main (void)
{
    ioinit(); //Setup IO pins and defaults

    DDRD |= 1 << PD4;
    // PORTD |= 1 << PD4;

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

    DDRD |= (1 << PD2); // pinout to start measuring
    DDRC &= ~(1 << PC1);
    turn_off_cap();

    _delay_ms(1000);

    init_lcd();
    lcd_print("Temp & Humidity!", 1);

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

            uint16_t temp = calc_temp();
            
            uint16_t avg_reading = 0;
            for (int i=0; i<NUM_READINGS; i++) avg_reading += readings[i];
            double avg_double = (double)avg_reading / (NUM_READINGS * 2);
            // avg_reading /= NUM_READINGS * 2;

            printf("averages: %f %u\n", avg_double, avg_reading);

            uint16_t humidity = calc_humidity(avg_double, temp);

            displays[display_num] = humidity;
            if (++display_num >= NUM_TO_AVG) display_num = 0;

            uint16_t avg_display = 0;
            int i = 0;
            for (i=0; i<NUM_TO_AVG && displays[i]; i++) {
                avg_display += displays[i];
            } 
            avg_display /= i;

            uint16_t temp_f = 9.0/5.0 * (double)temp + 320.0;

            char lcd_buf[17];
            snprintf(lcd_buf, 16, "%u.%uC %u.%uF", 
                temp / 10, temp % 10,
                temp_f / 10, temp_f % 10);

            char cap_info[17] = { '\0' };
            sprintf(cap_info, "%u.%u%%   (%u.%u)", 
                avg_display / 10, avg_display % 10,
                (uint16_t)avg_double / 10, (uint16_t)avg_double % 10);
            lcd_print(cap_info, 1);


            lcd_print(lcd_buf, 2);
            printf("avg: %upF %u.%u%% Humidity (last reading: %u.%u%%) Temp: %u.%u\n", 
                avg_reading, 
                avg_display / 10, avg_display % 10,
                humidity / 10, humidity % 10,
                temp / 10, temp % 10);

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


