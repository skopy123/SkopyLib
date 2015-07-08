/*
 * sockets.c
 *
 * Created: 28.7.2014 23:05:53
 *  Author: Daniel
 */ 
#include "sockets.h"
#include "w5100.h"
#include "Buffer.h"
#include <util/delay.h>
#include <avr/pgmspace.h>

unsigned char  Socket_Open(unsigned char  eth_protocol, unsigned int  tcp_port) {
	if (W51_read(W5100_SKT_REG_BASE+W5100_SR_OFFSET) != W5100_SKT_SR_CLOSED){
		Socket_Close();
	}
	W51_write(W5100_SKT_REG_BASE+W5100_MR_OFFSET ,W5100_SKT_MR_TCP);		// set protocol for this socket
	W51_write(W5100_SKT_REG_BASE+W5100_PORT_OFFSET, ((tcp_port & 0xFF00) >> 8 ));		// set port for this socket (MSB)
	W51_write(W5100_SKT_REG_BASE+W5100_PORT_OFFSET + 1, (tcp_port & 0x00FF));			// set port for this socket (LSB)
	//UART_transmitString("Opening socket\n");
	W51_write(W5100_SKT_REG_BASE+W5100_CR_OFFSET, W5100_SKT_CR_OPEN);	               	// open the socket
	while (W51_read(W5100_SKT_REG_BASE+W5100_CR_OFFSET)) ;			// loop until device reports socket is open (blocks!!)
	if (W51_read(W5100_SKT_REG_BASE+W5100_SR_OFFSET) != W5100_SKT_SR_INIT){
		UART_transmitString(PSTR("error, socket not inicialized\n"));		
		Socket_Close();	
		return 1;							// if failed, close socket immediately
	}
	//UART_transmitString("socket opened\n");
	return  0;
}

void  Socket_Close() {
	W51_write(W5100_SKT_REG_BASE+W5100_CR_OFFSET, W5100_SKT_CR_CLOSE);	// tell chip to close the socket
	while (W51_read(W5100_SKT_REG_BASE+W5100_CR_OFFSET))  ;	// loop until socket is closed (blocks!!)
}

void  Socket_Disconect() {
	W51_write(W5100_SKT_REG_BASE+W5100_CR_OFFSET, W5100_SKT_CR_DISCON);		// disconnect the socket
	while (W51_read(W5100_SKT_REG_BASE+W5100_CR_OFFSET))  ;	// loop until socket is closed (blocks!!)
}

unsigned char Socket_Listen(){
	if (W51_read(W5100_SKT_REG_BASE+W5100_SR_OFFSET) == W5100_SKT_SR_INIT)	// if socket is in initialized state...
	{
		W51_write(W5100_SKT_REG_BASE+W5100_CR_OFFSET, W5100_SKT_CR_LISTEN);		// put socket in listen state
		while (W51_read(W5100_SKT_REG_BASE+W5100_CR_OFFSET))  ;		// block until command is accepted

		if (W51_read(W5100_SKT_REG_BASE+W5100_SR_OFFSET) != W5100_SKT_SR_LISTEN){  	// if socket state changed, show success
			Socket_Close();	
			return 1;
		}				// not in listen mode, close and show an error occurred
		return 0;
	}
	return  1;
}

unsigned char Socket_Write(const unsigned char  *buf, unsigned int  buflen)
{
	unsigned int					ptr;
	unsigned int					offaddr;
	unsigned int					realaddr;
	unsigned int					txsize;
	unsigned int					timeout;
	unsigned int					sockaddr;

	if (buflen == 0)  return  W5100_FAIL;		// ignore illegal requests
	// Make sure the TX Free Size Register is available
	txsize = W51_read(W5100_SKT_REG_BASE+W5100_TX_FSR_OFFSET);		// make sure the TX free-size reg is available
	txsize = (((txsize & 0x00FF) << 8 ) + W51_read(W5100_SKT_REG_BASE+W5100_TX_FSR_OFFSET + 1));

	timeout = 0;
	while (txsize < buflen)
	{
		_delay_ms(1);

		txsize = W51_read(W5100_SKT_REG_BASE+W5100_TX_FSR_OFFSET);		// make sure the TX free-size reg is available
		txsize = (((txsize & 0x00FF) << 8 ) + W51_read(W5100_SKT_REG_BASE+W5100_TX_FSR_OFFSET + 1));

		if (timeout++ > 1000) 						// if max delay has passed...
		{
			Socket_Disconect();					// can't connect, close it down
			return  W5100_FAIL;						// show failure
		}
	}

	// Read the Tx Write Pointer
	ptr = W51_read(W5100_SKT_REG_BASE+W5100_TX_WR_OFFSET);
	offaddr = (((ptr & 0x00FF) << 8 ) + W51_read(W5100_SKT_REG_BASE+W5100_TX_WR_OFFSET + 1));

	while (buflen)
	{
		buflen--;
		realaddr = W5100_TXBUFADDR + (offaddr & W5100_TX_BUF_MASK);		// calc W5100 physical buffer addr for this socket

		W51_write(realaddr, *buf);					// send a byte of application data to TX buffer
		offaddr++;									// next TX buffer addr
		buf++;										// next input buffer addr
	}

	W51_write(W5100_SKT_REG_BASE+W5100_TX_WR_OFFSET, (offaddr & 0xFF00) >> 8);	// send MSB of new write-pointer addr
	W51_write(W5100_SKT_REG_BASE+W5100_TX_WR_OFFSET + 1, (offaddr & 0x00FF));		// send LSB

	W51_write(W5100_SKT_REG_BASE+W5100_CR_OFFSET, W5100_SKT_CR_SEND);	// start the send on its way
	while (W51_read(W5100_SKT_REG_BASE+W5100_CR_OFFSET))  ;	// loop until socket starts the send (blocks!!)

	return  W5100_OK;
}

unsigned int Socket_Read(unsigned char  *buf, unsigned int  buflen)
{
	unsigned int					ptr;
	unsigned int					offaddr;
	unsigned int					realaddr;

	if (buflen == 0)  return  W5100_FAIL;		// ignore illegal conditions

	if (buflen > (MAX_BUF-2))  buflen = MAX_BUF - 2;		// requests that exceed the max are truncated

	ptr = W51_read(W5100_SKT_REG_BASE+W5100_RX_RD_OFFSET);			// get the RX read pointer (MSB)
	offaddr = (((ptr & 0x00FF) << 8 ) + W51_read(W5100_SKT_REG_BASE+W5100_RX_RD_OFFSET + 1));		// get LSB and calc offset addr

	while (buflen)
	{
		buflen--;
		realaddr = W5100_RXBUFADDR + (offaddr & W5100_RX_BUF_MASK);
		*buf = W51_read(realaddr);
		offaddr++;
		buf++;
	}
	*buf='\0'; 												// buffer read is complete, terminate the string

	// Increase the S0_RX_RD value, so it point to the next receive
	W51_write(W5100_SKT_REG_BASE+W5100_RX_RD_OFFSET, (offaddr & 0xFF00) >> 8);	// update RX read offset (MSB)
	W51_write(W5100_SKT_REG_BASE+W5100_RX_RD_OFFSET + 1,(offaddr & 0x00FF));		// update LSB

	// Now Send the RECV command
	W51_write(W5100_SKT_REG_BASE+W5100_CR_OFFSET, W5100_SKT_CR_RECV);			// issue the receive command
	_delay_us(5);											// wait for receive to start

	return  W5100_OK;
}

unsigned int Socket_Available(){
	unsigned int val;
	val = W51_read(W5100_SKT_REG_BASE+W5100_RX_RSR_OFFSET) & 0xff;
	val = (val << 8) + W51_read(W5100_SKT_REG_BASE+W5100_RX_RSR_OFFSET + 1);
	return  val;
}
