#include <stdlib.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "UART.h"
//#include <util/delay.h>

void UART_init(void){
	UCSR0A = 0x02; // 2x mode
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	UCSR0C = (1<<UCSZ00)|(1<<UCSZ01);
	UBRR0 = F_CPU  /	(8*UART_BaudRate) -1;	
}

unsigned char UART_receiveByte( void ){
	while(!(UCSR0A & (1<<RXC0)));
	return UDR0;
}

int UART_receiveNumber(){
	char UART_str[8];
	for (int i =0; i<8;i++){
		char c = UART_receiveByte();
		if ((c=='\n')||(c=='\r')){
			UART_str[i] = 0x00;	
			break;
		}
		UART_str[i]=c;
	}
	UART_receiveByte();
	return atoi(UART_str);
}

int UART_receiveNumberRange(int min,int max){
	int data;
	while (1){
		data = UART_receiveNumber();
		if ((data>=min)&&(data<=max)){
			break;
		}
		UART_transmitStringFlash(PSTR("Value out of range, enter again\n"));
	}
	UART_transmitNumber(data);
	UART_transmitByte('\n');
	return data;
}

unsigned char UART_receiveBool(){
	char c;
	while(1){
		c = UART_receiveByte();
		if ((c=='y')||(c=='n')) break;
		UART_transmitStringFlash(PSTR("invalid choise!, [y/n]\n"));	
	}
	return (c=='y');
}

void UART_transmitByte(char data){
	while(!(UCSR0A & (1<<UDRE0)));
	UDR0 = data;
}

void UART_transmitHex( unsigned char dataType, unsigned long data ){
	unsigned char count, i, temp;
	char dataString[] = "0x        ";

	if (dataType == CHAR) count = 2;
	if (dataType == INT) count = 4;
	if (dataType == LONG) count = 8;

	for(i=count; i>0; i--)
	{
		temp = data % 16;
		if((temp>=0) && (temp<10)) dataString [i+1] = temp + 0x30;
		else dataString [i+1] = (temp - 10) + 0x41;

		data = data/16;
	}

	UART_transmitString (dataString);
}

void UART_transmitString(char* string){
	int i=0;
	while(string[i]){
		UART_transmitByte(string[i]);
		i++;	
	}
}

void UART_transmitStringFlash(char* string){
	int i=0;
	while(pgm_read_byte(&string[i])){
		UART_transmitByte(pgm_read_byte(&string[i]));
		i++;
	}
}

void UART_transmitNumber(int i){	
	char UART_str[8];
	itoa(i,UART_str,10);
	UART_transmitString(UART_str);
}

void UART_transmitLong(long i){
	char UART_str[16];
	ltoa(i,UART_str,10);
	UART_transmitString(UART_str);
}
