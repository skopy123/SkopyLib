/*
 * DHT11.c
 *
 * Created: 19.8.2014 0:55:02
 *  Author: Daniel
 */ 
#include <util/delay.h>
#include <avr/io.h>
#include "DHT11.h"
void SetPinInput(){
	DDRC = DDRC & 0xf7;
}

void SetPinOutput(){
	DDRC =DDRC | 0x08;
}

void SetPinLow(){
	PORTC = PORTC & 0xf7;
}

void SetPinHign(){
	PORTC = PORTC |0x08;
}

unsigned short ReadLowTime(){
	unsigned short t = 0;
	while ((PINC&0x08)==0x00){
		t++;
		_delay_us(1);
		if (t>10000) return 0xffff;
	}
	return t;
}

unsigned short ReadHighTime(){
	unsigned short t = 0;
	while ((PINC&0x08)==0x08){
		t++;
		_delay_us(1);
		if (t>10000) return 0xffff;
	}
	return t;
}

unsigned char ReadBit(){
	unsigned char tl, th;
	for (tl=0;tl<255;tl++){
		_delay_us(2);
		if ((PINC&0x08)==0x08) break; // stop counting when pin go High
	}
	for (th=0;th<255;th++){
		_delay_us(2);
		if ((PINC&0x08)==0x00) break; // stop counting when pin go low
	}
	if (th>=tl) return 0x01;
	return 0x00;
}

unsigned char ReadByte(){
	unsigned char data = 0x00;
	for(int c=0;c<8;c++){
		data = data | (ReadBit()<<(7-c));
	}
	return data;
}

unsigned char ReadSensorData(){

	SetPinLow();
	_delay_us(25);
	SetPinHign();
	_delay_us(25);
	SetPinInput();
	_delay_us(1);
	ReadHighTime();
	ReadLowTime();
	ReadHighTime();
	for(char d=0;d<5;d++){
		DHT11ResponseData[d] = ReadByte();
	}
	//check
	DHT11_Humidity = DHT11ResponseData[0];
	DHT11_Temperature = DHT11ResponseData[2];
	unsigned char sum = DHT11ResponseData[0]+DHT11ResponseData[1]+DHT11ResponseData[2]+DHT11ResponseData[3];
	if (sum==DHT11ResponseData[4]){
		return 0;
	}
	return 1;
}