/*
 * DHT11.h
 *
 * Created: 19.8.2014 1:08:59
 *  Author: Daniel
 */ 

#ifndef DHT11_H_
#define DHT11_H_

unsigned char DHT11_Temperature;
unsigned char DHT11_Humidity;

	unsigned char DHT11ResponseData[5];

unsigned char ReadSensorData();

#endif /* DHT11_H_ */