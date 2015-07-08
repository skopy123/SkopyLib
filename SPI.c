#include <avr/io.h>
#include "SPI.h"

//SPI initialize for SD card
void SPI_init(void){
	SPCR = 0x52; //setup SPI: Master mode, MSB first, SCK phase low, SCK idle low
	SPSR = 0x00;
	// TO DO DDRB
}

unsigned char SPI_transmit(unsigned char data){
	SPDR = data;
	while(!(SPSR & (1<<SPIF)));
	data = SPDR;
	return(data);
}

unsigned char SPI_receive(void){
	unsigned char data;
	SPDR = 0xff;
	while(!(SPSR & (1<<SPIF)));
	data = SPDR;
	return data;
}

