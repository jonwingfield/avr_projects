#ifndef __lcddefs_h__
#define __lcddefs_h__

#include <unistd.h> // for usleep, refine if needed

extern volatile unsigned* gpio;

#define LCD_INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define LCD_OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))

#define SET_OUTP(g, i)   \
	if (i) { *(gpio+7) = (1 << (g)); } \
	else { *(gpio+10) = (1 << (g)); }
#define SETUP_OUT_PIN(g)  LCD_INP_GPIO(g); LCD_OUT_GPIO(g);

#define DB4 7   
#define DB5	24
#define DB6	9
#define DB7	10
#define RS	11
#define E	25

#endif