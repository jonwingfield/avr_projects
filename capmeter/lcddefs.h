#ifndef __lcddefs_h__
#define __lcddefs_h__

#include <avr/io.h>
#include <util/delay.h> 
#define usleep(n) _delay_us(n * 1.3)
#define sleep(n)  usleep(n* 1000000)

#define SET_OUTP(g, i)   \
	if ((g) != PD4) { \
		if (i) { PORTC |= (1 << (g)); } \
		else { PORTC &= ~(1 << (g)); } \
	} else { \
		if (i) { PORTD |= (1 << (g)); } \
		else { PORTD &= ~(1 << (g)); } \		
	}
#define SETUP_OUT_PIN(g)  if ((g) != PD4) { DDRC |= 1 << (g); } else { DDRD |= 1 << (g); }

#define DB4 PC0   
#define DB5	PC1
#define DB6	PC2
#define DB7	PC3
#define RS	PC5
#define E	PD4

#endif