#ifndef __lcddefs_h__
#define __lcddefs_h__

#include <avr/io.h>
#include <util/delay.h> 
#define usleep(n) _delay_us(n * 1.3)
#define sleep(n)  usleep(n* 1000000)

#define SET_OUTP(g, i)   \
	if ((g) < 3) { \
		if (i) { PORTB |= (1 << (g)); } \
		else { PORTB &= ~(1 << (g)); } \
	} else { \
		if (i) { PORTC |= (1 << ((g)-4)); } \
		else { PORTC &= ~(1 << ((g)-4)); } \
	}
#define SETUP_OUT_PIN(g)  if ((g) < DB6) { DDRB |= 1 << (g); } else { DDRC |= 1 << ((g)-4); }

#define DB4 4   // PC0
#define DB5	2   // PB2
#define DB6	6   // PC2
#define DB7	7   // PC3
#define RS	0   // PB0
#define E	1   // PB1

#endif