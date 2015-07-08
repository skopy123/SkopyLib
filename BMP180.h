/*
 * BMP180.h
 *
 * Created: 17.8.2014 17:54:35
 *  Author: Daniel
 */ 


#ifndef BMP180_H_
#define BMP180_H_
#include <avr/io.h>

typedef struct{
	short  ac1;
	short  ac2;
	short  ac3;
	unsigned short ac4;
	unsigned short ac5;
	unsigned short ac6;
	short  b1;
	short  b2;
	short  mb;
	short  mc;
	short  md;
} bmp085_calib_data;

#define BMP085_MODE_ULTRALOWPOWER 0
#define BMP085_MODE_STANDARD 1
#define BMP085_MODE_HIGHRES 2
#define BMP085_MODE_ULTRAHIGHRES 3

bmp085_calib_data calibrationData;

enum
{
	BMP085_REGISTER_CAL_AC1            = 0xAA,  // R   Calibration data (16 bits)
	BMP085_REGISTER_CAL_AC2            = 0xAC,  // R   Calibration data (16 bits)
	BMP085_REGISTER_CAL_AC3            = 0xAE,  // R   Calibration data (16 bits)
	BMP085_REGISTER_CAL_AC4            = 0xB0,  // R   Calibration data (16 bits)
	BMP085_REGISTER_CAL_AC5            = 0xB2,  // R   Calibration data (16 bits)
	BMP085_REGISTER_CAL_AC6            = 0xB4,  // R   Calibration data (16 bits)
	BMP085_REGISTER_CAL_B1             = 0xB6,  // R   Calibration data (16 bits)
	BMP085_REGISTER_CAL_B2             = 0xB8,  // R   Calibration data (16 bits)
	BMP085_REGISTER_CAL_MB             = 0xBA,  // R   Calibration data (16 bits)
	BMP085_REGISTER_CAL_MC             = 0xBC,  // R   Calibration data (16 bits)
	BMP085_REGISTER_CAL_MD             = 0xBE,  // R   Calibration data (16 bits)
	BMP085_REGISTER_CHIPID             = 0xD0,
	BMP085_REGISTER_VERSION            = 0xD1,
	BMP085_REGISTER_SOFTRESET          = 0xE0,
	BMP085_REGISTER_CONTROL            = 0xF4,
	BMP085_REGISTER_TEMPDATA           = 0xF6,
	BMP085_REGISTER_PRESSUREDATA       = 0xF6,
	BMP085_REGISTER_READTEMPCMD        = 0x2E,
	BMP085_REGISTER_READPRESSURECMD    = 0x34
};

int ActualPressure, ActualIndoorTemp;
void BMP_WriteByte(unsigned char addr, unsigned char data);
unsigned char BMP_ReadByte(unsigned char addr);
unsigned short BMP_ReadShort(unsigned char addr);
int BMP_readRawTemperature();
long BMP_readRawPressure();
void BMP_getActualData();
void BMP_Init();
#endif /* BMP180_H_ */