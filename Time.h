/*
 * Time.h
 *
 * Created: 17.8.2014 13:35:15
 *  Author: Daniel
 */ 


#ifndef TIME_H_
#define TIME_H_

volatile unsigned char timeDataBuffer[6];

struct Time_Structure{ // length 6B
	unsigned char year;//00 to 99
	unsigned char month;
	unsigned char date; //day in month
	unsigned char hour; //24h format
	unsigned char minute;
	unsigned char second;
};

struct Time_Structure* ActualTime;
unsigned int FatTime();
unsigned int FatDate();
unsigned char* TimeString();
unsigned char* DateString();

#endif /* TIME_H_ */