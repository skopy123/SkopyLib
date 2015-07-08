/*
 * I2C.c
 *
 * Created: 17.8.2014 13:00:07
 *  Author: Daniel
 */ 
#include <avr/io.h>
#include <avr/pgmspace.h>
void I2C_Init(){
	PORTC=0x30;
	//
	TWSR = 0x00;
	TWBR = 0x48;
	TWCR = (1<<TWEN);
	
}

//send start
void I2C_Start(void){
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	while ((TWCR & (1<<TWINT)) == 0);
}

//send stop signal
void I2C_Stop(void){
	TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
}

void I2C_Write(unsigned char data){
	TWDR = data;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while ((TWCR & (1<<TWINT)) == 0);
}

unsigned char I2C_ReadACK(void)
{
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
	while ((TWCR & (1<<TWINT)) == 0);
	return TWDR;
}
//read byte with NACK
unsigned char I2C_ReadNACK(void)
{
	TWCR = (1<<TWINT)|(1<<TWEN);
	while ((TWCR & (1<<TWINT)) == 0);
	return TWDR;
}