/*
 *  w5100.c      library of target-independent AVR support routines
 *               for the Wiznet W5100 Ethernet interface device
 *
 *  This file is derived from the excellent work found here:
 *  www.ermicro.com/blog/?p=1773
 *  by RWB.  I am leaving the header from the original file intact below,
 *  but you need to remember the rest of the source here is fairly
 *  heavily modified.  Go to the above site for the original.
 */

/*****************************************************************************
//  File Name    : wiznetping.c
//  Version      : 1.0
//  Description  : Wiznet W5100
//  Author       : RWB
//  Target       : AVRJazz Mega168 Board
//  Compiler     : AVR-GCC 4.3.2; avr-libc 1.6.6 (WinAVR 20090313)
//  IDE          : Atmel AVR Studio 4.17
//  Programmer   : AVRJazz Mega168 STK500 v2.0 Bootloader
//               : AVR Visual Studio 4.17, STK500 programmer
//  Last Updated : 01 July 2010
*****************************************************************************/


/*
 *  The following code turns the above wiznetping.c source code into a
 *  generic library of W5100 support routines that are target-independent.
 *  That is, you build this library for a generic AVR ATmega device, then
 *  write your application to use the W51_xxx routines below for accessing
 *  the W5100 chip.  Because these routines are target-independent, you
 *  never have to rebuild them just because you are moving your code from,
 *  say, a 'mega128 to an 'xmega128a1 device.
 *
 *  For this to work properly, your application must provide three target-
 *  specific functions and must register the addresses of those functions
 *  with the W5100 library at run-time.  These functions are:

 *  Your application registers these three functions with the W5100 library
 *  by invoking the W51_register() function.  Your application must make this
 *  call one time and must make this call before calling any other W5100
 *  functions.
 */


#include <util/delay.h>
#include "w5100.h"
#include <avr/io.h>

#define W5100SS 0x04

void  W51_write(unsigned int  addr, unsigned char  data)
{
	PORTB&=~(W5100SS);
	SPI_transmit(W5100_WRITE_OPCODE);					// need to write a byte
	SPI_transmit((addr & 0xff00) >> 8);				// send MSB of addr
	SPI_transmit(addr & 0x00ff);							// send LSB
	SPI_transmit(data);		
	PORTB|=(W5100SS);							// send the data
}

unsigned char  W51_read(unsigned int  addr){
	unsigned char data;
	PORTB&=~(W5100SS);
	SPI_transmit(W5100_READ_OPCODE);					// need to read a byte
	SPI_transmit((addr & 0xff00) >> 8);				// send MSB of addr
	SPI_transmit(addr & 0xff);							// send LSB
	data = SPI_receive();	
	PORTB|=(W5100SS);
	return data;					
}

void  W51_init(void)
{
	W51_write(W5100_MR, W5100_MR_SOFTRST); 		// otherwise, force the w5100 to soft-reset
	_delay_ms(1);
	
	W51_write(W5100_GAR + 0, 192);	// set up the gateway address
	W51_write(W5100_GAR + 1, 168);
	W51_write(W5100_GAR + 2, 168);
	W51_write(W5100_GAR + 3, 1);
	_delay_ms(1);

	W51_write(W5100_SHAR + 0, 0x00);	// set up the MAC address
	W51_write(W5100_SHAR + 1, 0x16);
	W51_write(W5100_SHAR + 2, 0x36);
	W51_write(W5100_SHAR + 3, 0xDE);
	W51_write(W5100_SHAR + 4, 0x59);
	W51_write(W5100_SHAR + 5, 0xAB);
	_delay_ms(1);

	W51_write(W5100_SUBR + 0, 255);	// set up the subnet mask
	W51_write(W5100_SUBR + 1, 255);
	W51_write(W5100_SUBR + 2, 0);
	W51_write(W5100_SUBR + 3, 0);
	_delay_ms(1);

	W51_write(W5100_SIPR + 0, 192);	// set up the source IP address
	W51_write(W5100_SIPR + 1, 168);
	W51_write(W5100_SIPR + 2, 168);
	W51_write(W5100_SIPR + 3, 78);
	_delay_ms(1);

	W51_write(W5100_RMSR, 0x55); //8k buffer single socket
	W51_write(W5100_TMSR, 0x55);
}

