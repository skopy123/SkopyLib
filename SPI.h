
#ifndef _SPI
#define _SPI

#define SPI_SD             SPCR = 0x52
#define SPI_HIGH_SPEED     SPCR = 0x50//; SPSR |= (1<<SPI2X)


void SPI_init(void);
unsigned char SPI_transmit(unsigned char);
unsigned char SPI_receive(void);

#endif
