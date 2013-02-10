// VirtualWire.cpp
//
// Virtual Wire implementation for Arduino
// See the README file in this directory fdor documentation
// See also
// ASH Transceiver Software Designer's Guide of 2002.08.07
//   http://www.rfm.com/products/apnotes/tr_swg05.pdf
//
// Changes:
// 1.5 2008-05-25: fixed a bug that could prevent messages with certain
//  bytes sequences being received (false message start detected)
// 1.6 2011-09-10: Patch from David Bath to prevent unconditional reenabling of the receiver
//  at end of transmission.
//
// Author: Mike McCauley (mikem@open.com.au)
// Copyright (C) 2008 Mike McCauley
// $Id: VirtualWire.cpp,v 1.8 2013/01/14 06:49:29 mikem Exp mikem $


#include "VirtualWire.h"
#include <avr/interrupt.h>
#include <util/crc16.h>
#include <stdbool.h>
#include <string.h>

static uint8_t vw_tx_buf[(VW_MAX_MESSAGE_LEN * 2) + VW_HEADER_LEN] 
     = {0x2a, 0x2a, 0x2a, 0x2a, 0x2a, 0x2a, 0x38, 0x2c};   // 0x28 & (0x2C << 6) = 0xb38 for 12-bit encoding

// Number of symbols in vw_tx_buf to be sent;
static uint8_t vw_tx_len = 0;

// Index of the next symbol to send. Ranges from 0 to vw_tx_len
static uint8_t vw_tx_index = 0;

// Bit number of next bit to send
static uint8_t vw_tx_bit = 0;

// Sample number for the transmitter. Runs 0 to 7 during one bit interval
static uint8_t vw_tx_sample = 0;

// Flag to indicated the transmitter is active
static volatile uint8_t vw_tx_enabled = 0;

// Total number of messages sent
static uint16_t vw_tx_msg_count = 0;

// 4 bit to 6 bit symbol converter table
// Used to convert the high and low nybbles of the transmitted data
// into 6 bit symbols for transmission. Each 6-bit symbol has 3 1s and 3 0s 
// with at most 3 consecutive identical bits
static uint8_t symbols[] =
{
    0xd,  0xe,  0x13, 0x15, 0x16, 0x19, 0x1a, 0x1c, 
    0x23, 0x25, 0x26, 0x29, 0x2a, 0x2c, 0x32, 0x34
};

void vw_setup(uint16_t speed)
{
    // Calculate the OCR1A overflow count based on the required bit speed
    // and CPU clock rate
    uint8_t ocr0a = 125;

#ifndef TEST
    // for send, set up one of the 8-bit timers

    TCCR0A = _BV(WGM01);  // CTC mode
    TCCR0B = _BV(CS01) | _BV(CS00); // clock / 64 (doesn't work for 4000bps!)
    OCR0A = ocr0a;  // overflow when we hit our bit rake
    // Enable interrupt
#ifdef TIMSK0
    // atmega168
    TIMSK0 |= _BV(OCIE0A);
#else
    // others
    TIMSK |= _BV(OCIE1A);
#endif

#endif

    // Set up digital IO pins
    pinMode(vw_tx_pin, OUTPUT);
    // pinMode(vw_ptt_pin, OUTPUT);
    // digitalWrite(vw_ptt_pin, 1);
}
	
// Start the transmitter, call when the tx buffer is ready to go and vw_tx_len is
// set to the total number of symbols to send
void vw_tx_start(void)
{
    vw_tx_index = 0;
    vw_tx_bit = 0;
    vw_tx_sample = 0;

    // Enable the transmitter hardware
    // digitalWrite(vw_ptt_pin, true);

    // Next tick interrupt will send the first bit
    vw_tx_enabled = true;
}

// Stop the transmitter, call when all bits are sent
void vw_tx_stop(void)
{
    // Disable the transmitter hardware
    // digitalWrite(vw_ptt_pin, false);
    digitalWrite(vw_tx_pin, false);

    // No more ticks for the transmitter
    vw_tx_enabled = false;
}

// Return true if the transmitter is active
uint8_t vx_tx_active(void)
{
    return vw_tx_enabled;
}

// Wait for the transmitter to become available
// Busy-wait loop until the ISR says the message has been sent
void vw_wait_tx(void)
{
    while (vw_tx_enabled) asm volatile("nop");
}

// Wait until transmitter is available and encode and queue the message
// into vw_tx_buf
// The message is raw bytes, with no packet structure imposed
// It is transmitted preceded a byte count and followed by 2 FCS bytes
uint8_t vw_send(uint8_t* buf, uint8_t len)
{
    uint8_t i;
    uint8_t index = 0;
    uint16_t crc = 0xffff;
    uint8_t *p = vw_tx_buf + VW_HEADER_LEN; // start of the message area
    uint8_t count = len + 3; // Added byte count and FCS to get total number of bytes

    if (len > VW_MAX_PAYLOAD) return false;

    // Wait for transmitter to become available
    vw_wait_tx();

    // Encode the message length
    crc = _crc_ccitt_update(crc, count);
    p[index++] = symbols[count >> 4];
    p[index++] = symbols[count & 0xf];

    // Encode the message into 6 bit symbols. Each byte is converted into 
    // 2 6-bit symbols, high nybble first, low nybble second
    for (i = 0; i < len; i++)
    {
		crc = _crc_ccitt_update(crc, buf[i]);
		p[index++] = symbols[buf[i] >> 4];
		p[index++] = symbols[buf[i] & 0xf];
    }

    // Append the fcs, 16 bits before encoding (4 6-bit symbols after encoding)
    // Caution: VW expects the _ones_complement_ of the CCITT CRC-16 as the FCS
    // VW sends FCS as low byte then hi byte
    crc = ~crc;
    p[index++] = symbols[(crc >> 4)  & 0xf];
    p[index++] = symbols[crc & 0xf];
    p[index++] = symbols[(crc >> 12) & 0xf];
    p[index++] = symbols[(crc >> 8)  & 0xf];

    // Total number of 6-bit symbols to send
    vw_tx_len = index + VW_HEADER_LEN;

    // Start the low level interrupt handler sending symbols
    vw_tx_start();

    return true;
}

// This is the interrupt service routine called when timer1 overflows
// Its job is to output the next bit from the transmitter (every 8 calls)
// and to call the PLL code if the receiver is enabled
//ISR(SIG_OUTPUT_COMPARE1A)
ISR(TIMER0_COMPA_vect)
{
    if (!vw_tx_enabled) return;

	// Send next bit
	// Symbols are sent LSB first
	// Finished sending the whole message? (after waiting one bit period 
	// since the last bit)
	if (vw_tx_index >= vw_tx_len)
	{
	    vw_tx_stop();
	    vw_tx_msg_count++;
	}
	else
	{
        digitalWrite(vw_tx_pin, vw_tx_buf[vw_tx_index] & (1 << vw_tx_bit));

	    vw_tx_bit++;
	    if (vw_tx_bit >= 6)
	    {
			vw_tx_bit = 0;
			vw_tx_index++;
	    }
	}
}

