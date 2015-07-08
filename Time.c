/*
 * Time.c
 *
 * Created: 17.8.2014 13:35:27
 *  Author: Daniel
 */ 

#include "Time.h"
#include <string.h>

unsigned char* TimeString(){
	char temp[5];
	char result[12];
	itoa(ActualTime->hour,temp,10);
	strcpy(result,temp);
	strcat(result,":");
	itoa(ActualTime->minute,temp,10);
	strcat(result,temp);
	strcat(result,":");
	itoa(ActualTime->second,temp,10);
	strcat(result,temp);
	return result;
}

unsigned char* DateString(){
	char temp[5];
	char result[12];
	itoa(ActualTime->year+2000,temp,10);
	strcpy(result,temp);
	strcat(result,".");
	for (int i=0;i<5;i++){
		temp[i]=0;
	}
	itoa(ActualTime->month,temp,10);
	strcat(result,temp);
	strcat(result,".");
	itoa(ActualTime->date,temp,10);
	strcat(result,temp);
	return result;
}

unsigned int FatTime(){
	unsigned int resultTime =  (unsigned int)ActualTime->hour;
	resultTime = (resultTime<<6)|ActualTime->minute;
	resultTime = (resultTime<<5)|((ActualTime->second) / 2);
	return resultTime;
}

unsigned int FatDate(){
	unsigned int resultDate =  (unsigned int)(ActualTime->year)+20;
	resultDate = (resultDate<<4)|ActualTime->month;
	resultDate = (resultDate<<5)|ActualTime->date;
	return resultDate; 
}
