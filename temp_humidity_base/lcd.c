#include "lcd.h"
#include "lcddefs.h"
#include "dbg.h"

void write_output(char i)
{
	i >>= 4;
	debug("writing %x", i);
	SET_OUTP(DB4, (i & 1));
	SET_OUTP(DB5, (i & 2));
	SET_OUTP(DB6, (i & 4));
	SET_OUTP(DB7, (i & 8));
	//sleep(3);
}

void nybble()
{
	SET_OUTP(E, 1);
	usleep(1000);
	SET_OUTP(E, 0);
}

void command(char i)
{
	debug("command 0x%x", i);
	SET_OUTP(RS, 0);
	write_output(i);
	nybble();
	i <<= 4;
	write_output(i);
	nybble();	
}

void lcd_write(char i)
{
	debug("writing character %c", i);
	SET_OUTP(RS, 1);
	write_output(i);
	nybble();
	i <<= 4;
	write_output(i);
	nybble();
}

void lcd_print(char* str, short line)
{
	command(0x80 | (line == 2 ? 0x40 : 0));

	while (*str) {
		lcd_write(*str++);
	}
}

void init_lcd()
{
	SETUP_OUT_PIN(DB4);
	SETUP_OUT_PIN(DB5);
	SETUP_OUT_PIN(DB6);
	SETUP_OUT_PIN(DB7);
	SETUP_OUT_PIN(RS);
	SETUP_OUT_PIN(E);

	SET_OUTP(E, 0);
	SET_OUTP(RS, 0);
	write_output(0);
	//SET_OUTP(P3, 0);
	usleep(100000);
	write_output(0x30); // wake up
	usleep(30000);
	nybble();
	usleep(10000);
	nybble();
	usleep(10000);
	nybble();
	usleep(10000);
	sleep(1);
	nybble();
	write_output(0x20);  // 4-bit interface
	nybble();
	command(0x28);    // 4-bit / 2 lines 
	command(0x10);	  // set cursor
	command(0x0C);	  // display on, blinking cursor
	command(0x01);
	usleep(2000);
	command(0x02);
	usleep(2000);
	command(0x06);    // entry mode set
}

