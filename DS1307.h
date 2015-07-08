/*
 * DS1307.h
 *
 * Created: 17.8.2014 13:50:20
 *  Author: Daniel
 */ 


#ifndef DS1307_H_
#define DS1307_H_


//
// DS1307 register addresses
//
#define DS1307_SECONDS 0x00
#define DS1307_MINUTES 0x01
#define DS1307_HOURS 0x02
#define DS1307_DAY 0x03
#define DS1307_DATE 0x04
#define DS1307_MONTH 0x05
#define DS1307_YEAR 0x06
#define DS1307_CONTROL 0x07

#define DS1307_ADDRESS_WRITE 0xD0
#define DS1307_ADDRESS_READ 0xD1

void DS_init();
void DS_ReadFromRTC();
void DS_WriteToRTC();
unsigned char DS_ReadAndDecodeBCD(unsigned char addr);
unsigned char DS_ReadByte(unsigned char addr);

#endif /* DS1307_H_ */