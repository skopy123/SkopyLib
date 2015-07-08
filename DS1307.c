/*
 * DS1307.c
 *
 * Created: 17.8.2014 13:47:29
 *  Author: Daniel
 */ 
#include "I2C.h"
#include "Time.h"
#include "DS1307.h"
#include "avr/delay.h"

void DS_WriteByte(unsigned char addr, unsigned char data){
	I2C_Start();
	I2C_Write(DS1307_ADDRESS_WRITE);
	I2C_Write(addr);
	I2C_Write(data);
	I2C_Stop();
}

unsigned char DS_ReadByte(unsigned char addr){
	unsigned char result;
	I2C_Start();
	I2C_Write(DS1307_ADDRESS_WRITE);
	I2C_Write(addr);
	I2C_Stop();
	
	_delay_ms(5);
	I2C_Start();
	I2C_Write(DS1307_ADDRESS_READ);
	result = I2C_ReadNACK();
	I2C_Stop();
	return result;
}

unsigned char DS_ReadAndDecodeBCD(unsigned char addr){
	unsigned char result,temp;
	temp = DS_ReadByte(addr);
	if (addr==DS1307_HOURS){
		temp = temp & 0x3f;//0b00111111;
	}
	result = (temp & 0xf0) >> 4;
	result = result*10;
	result = result + (temp & 0x0f);
	return result;
}

void DS_WriteAndEncodeBCD(unsigned char addr,unsigned char data){
	unsigned char tempData;
	tempData = ((data / 10) % 0x0f)<<4;
	tempData = tempData | ((data % 10)&0x0f);
	DS_WriteByte(addr,tempData);
}

void DS_init(){
	ActualTime = (struct Time_Structure *) &timeDataBuffer[0];
	unsigned char sec= DS_ReadByte(DS1307_SECONDS);
	sec = sec & 0b01111111;	// clear the CH bit
	DS_WriteByte(DS1307_SECONDS,sec);
	
	sec= DS_ReadByte(DS1307_HOURS);
	sec = sec & 0b00111111;	// set 24 h mode
	DS_WriteByte(DS1307_HOURS,sec);
}

void DS_ReadFromRTC(){
	ActualTime->second = DS_ReadAndDecodeBCD(DS1307_SECONDS);
	ActualTime->minute = DS_ReadAndDecodeBCD(DS1307_MINUTES);
	ActualTime->hour = DS_ReadAndDecodeBCD(DS1307_HOURS);
	ActualTime->date = DS_ReadAndDecodeBCD(DS1307_DATE);
	ActualTime->month = DS_ReadAndDecodeBCD(DS1307_MONTH);
	ActualTime->year = DS_ReadAndDecodeBCD(DS1307_YEAR);
}

void DS_WriteToRTC(){
	DS_WriteAndEncodeBCD(DS1307_SECONDS,ActualTime->second);
	DS_WriteAndEncodeBCD(DS1307_MINUTES,ActualTime->minute);
	DS_WriteAndEncodeBCD(DS1307_HOURS,ActualTime->hour);
	DS_WriteAndEncodeBCD(DS1307_DATE,ActualTime->date);
	DS_WriteAndEncodeBCD(DS1307_MONTH,ActualTime->month);
	DS_WriteAndEncodeBCD(DS1307_YEAR,ActualTime->year);
}




