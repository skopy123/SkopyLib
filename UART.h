// UART functions, communication speed 57600bd

#ifndef _UART
#define _UART
	#include <avr/pgmspace.h>
	#define UART_BaudRate 57600

	#define CHAR 0
	#define INT  1
	#define LONG 2
	


	void UART_init(void);
	unsigned char UART_receiveByte(void);
	int UART_receiveNumber();
	int UART_receiveNumberRange(int min,int max);
	
	unsigned char UART_receiveBool();
	void UART_transmitByte(char);
	void UART_transmitString(char* string);
	void UART_transmitStringFlash(char* string);
	void UART_transmitHex( unsigned char dataType, unsigned long data );
	void UART_transmitNumber(int i);
	void UART_transmitLong(long i);
	
	# define UART_transmitStringF(s) UART_transmitStringFlash(__extension__({static const char __c[] PROGMEM = (s); &__c[0];})) //integration of pstr

#endif
