/*
 * BMP180.c
 *
 * Created: 17.8.2014 17:54:03
 *  Author: Daniel
 */ 

#include "BMP180.h"
#include "I2C.h"
#include "avr/delay.h"
#include "UART.h"
#include <math.h>

void BMP_Init()
{
	if (BMP_ReadByte(0xD0)!=0x55) {
		UART_transmitString("BMP not connected\n");
		return;
	}

	calibrationData.ac1 = (short) BMP_ReadShort(BMP085_REGISTER_CAL_AC1);
	calibrationData.ac2 = (short) BMP_ReadShort(BMP085_REGISTER_CAL_AC2);
	calibrationData.ac3 = (short) BMP_ReadShort(BMP085_REGISTER_CAL_AC3);
	
	calibrationData.ac4 = BMP_ReadShort(BMP085_REGISTER_CAL_AC4);
	calibrationData.ac5 = BMP_ReadShort(BMP085_REGISTER_CAL_AC5);
	calibrationData.ac6 = BMP_ReadShort(BMP085_REGISTER_CAL_AC6);

	calibrationData.b1 = (short) BMP_ReadShort(BMP085_REGISTER_CAL_B1);
	calibrationData.b2 = (short) BMP_ReadShort(BMP085_REGISTER_CAL_B2);
	
	calibrationData.mb = (short) BMP_ReadShort(BMP085_REGISTER_CAL_MB);
	calibrationData.mc = (short) BMP_ReadShort(BMP085_REGISTER_CAL_MC);
	calibrationData.md = (short) BMP_ReadShort(BMP085_REGISTER_CAL_MD);
	
/*	UART_transmitString("\nA1"); UART_transmitLong((long)calibrationData.ac1);
	UART_transmitString("\nA2"); UART_transmitLong((long)calibrationData.ac2);
	UART_transmitString("\nA3"); UART_transmitLong((long)calibrationData.ac3);
	UART_transmitString("\nA4"); UART_transmitLong((long)calibrationData.ac4);
	UART_transmitString("\nA5"); UART_transmitLong((long)calibrationData.ac5);
	UART_transmitString("\nA6"); UART_transmitLong((long)calibrationData.ac6);

	UART_transmitString("\nb1"); UART_transmitLong((long)calibrationData.b1);
	UART_transmitString("\nb2"); UART_transmitLong((long)calibrationData.b2);

	UART_transmitString("\nmb"); UART_transmitLong((long)calibrationData.mb);
	UART_transmitString("\nmc"); UART_transmitLong((long)calibrationData.mc);
	UART_transmitString("\nmd"); UART_transmitLong((long)calibrationData.md);*/

}

void BMP_WriteByte(unsigned char addr, unsigned char data)
{
	I2C_Start();
	I2C_Write(0xEE);
	I2C_Write(addr);
	I2C_Write(data);
	I2C_Stop();
}

unsigned char BMP_ReadByte(unsigned char addr)
{
	unsigned char bmp_result;
	I2C_Start();
	I2C_Write(0xEE);
	I2C_Write(addr);
	I2C_Stop();
	_delay_ms(5);
	I2C_Start();
	I2C_Write(0xEF);
	bmp_result = I2C_ReadNACK();
	I2C_Stop();
	return bmp_result;
}

unsigned short BMP_ReadShort(unsigned char addr){
	unsigned char b0,b1;
	I2C_Start();
	I2C_Write(0xEE);
	I2C_Write(addr);
	I2C_Stop();
	_delay_ms(5);
	I2C_Start();
	I2C_Write(0xEF);
	b0 = I2C_ReadACK();
	b1 = I2C_ReadNACK();
	I2C_Stop();
	return (b0<<8) | b1;
	
}

int BMP_readRawTemperature(){
	int t;
	BMP_WriteByte(BMP085_REGISTER_CONTROL , BMP085_REGISTER_READTEMPCMD);
	_delay_ms(5);
	t = (int)BMP_ReadShort(BMP085_REGISTER_TEMPDATA);
	return t;
}

long BMP_readRawPressure(){
	unsigned char p8;
	int  p32;

	BMP_WriteByte(BMP085_REGISTER_CONTROL, BMP085_REGISTER_READPRESSURECMD + (BMP085_MODE_STANDARD << 6));
	_delay_ms(14);

	p32 = (int)BMP_ReadShort(BMP085_REGISTER_PRESSUREDATA);
	p32 = p32 << 8;
	p8 = BMP_ReadByte(BMP085_REGISTER_PRESSUREDATA+2);
	p32 += p32 + p8;
	p32 = p32>>(8 - BMP085_MODE_STANDARD);
	
	return p32;
}

void BMP_getActualData(){
	int  ut = 0;
	long up=0;
	long  x1,x2,b5, b6, x3, b3, p;
	unsigned long b4, b7;

	ut = BMP_readRawTemperature();
	up = BMP_readRawPressure();


	x1 = ut - (int)calibrationData.ac6;
	x1 = x1 *((int)calibrationData.ac5);
	x1 = x1 >> 15;
	
	x2 = (int)calibrationData.mc;
	x2= x2<<11;
	x2 = x2/(x1+(long)calibrationData.md);
	
	b5 = x1 + x2;
	ActualIndoorTemp = (int)((b5+8)>>4);
	

	b6 = b5 - 4000;
	x1 = b6 * b6;
	x1 = x1 >>12;
	x1 = x1 * (long)calibrationData.b2;
	x1 = x1 >>11;
	x2 = (long)calibrationData.ac2 * b6;
	x2 = x2 >> 11;
	x3 = x1 + x2;
	b3 = (long)calibrationData.ac3;
	b3 = b3<<2;
	b3 = b3+x3;
	b3 = b3 <<BMP085_MODE_HIGHRES;
	b3 = b3 +2;
	b3 = b3>>2;
	
	x1 = (long)calibrationData.ac3 * b6;
	x1 = x1 >> 13;
	
	x2 = b6 * b6;
	x2 = x2 >>12;
	x2 = x2 * (long)calibrationData.b1;
	x2 = x2 >>16;
	

	x3 = x1 + x2 + 2;
	x3 = x3 >> 2;
	b4= (unsigned long)(x3);
	b4 = b4 + 32768;
	b4 = b4 * calibrationData.ac4;
	b4 = b4 >> 15;
	b7 = (unsigned long)up;
	b7 = b7-(unsigned long)b3;
		
	b7 = b7 * (50000 >> BMP085_MODE_HIGHRES);

	if (b7 < 0x80000000){
		p = (long)(b7);
		p = p << 1;
		p =  p/b4;
	}
	else{
		p = (long)(b7 / b4);
		p = p << 1;
	}

	x1 = (p >> 8);
	x1 = x1*x1;
	x1 = x1 * 3038;
	x1 = x1 >> 16;
	x2 = -7357 * p;
	x2 = x2 >> 16;
	p = p + ((x1 + x2 + 3791) >> 4);
	ActualPressure = (int)p;
}

/*
void BMP_getActualData(float *pressure,float *temperature){
		int  ut = 0, up = 0;
		int  x1, x2, b5, b6, x3, b3, p;
		unsigned int b4, b7;

		
		ut = BMP_readRawTemperature();
		up = BMP_readRawPressure();


		x1 = (ut - (int)(calibrationData.ac6))*((int)(calibrationData.ac5))/pow(2,15);
		x2 = ((int)(calibrationData.mc*pow(2,11)))/(x1+(int)(calibrationData.md));
		b5 = x1 + x2;
		*temperature = (b5+8)>>4;

		b6 = b5 - 4000;
		x1 = (calibrationData.b2 * ((b6 * b6) >> 12)) >> 11;
		x2 = (calibrationData.ac2 * b6) >> 11;
		x3 = x1 + x2;
		b3 = (((((int) calibrationData.ac1) * 4 + x3) << BMP085_MODE_STANDARD) + 2) >> 2;
		x1 = (calibrationData.ac3 * b6) >> 13;
		x2 = (calibrationData.b1 * ((b6 * b6) >> 12)) >> 16;
		x3 = ((x1 + x2) + 2) >> 2;
		b4 = (calibrationData.ac4 * (unsigned int) (x3 + 32768)) >> 15;
		b7 = ((unsigned int) (up - b3) * (50000 >> BMP085_MODE_STANDARD));

		if (b7 < 0x80000000)
		{
			p = (b7 << 1) / b4;
		}
		else
		{
			p = (b7 / b4) << 1;
		}

		x1 = (p >> 8) * (p >> 8);
		x1 = (x1 * 3038) >> 16;
		x2 = (-7357 * p) >> 16;
		*pressure = p + ((x1 + x2 + 3791) >> 4);

}

*/
