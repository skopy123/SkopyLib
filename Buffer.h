/*
 * Buffer.h
 *
 * Created: 7.8.2014 0:45:36
 *  Author: Daniel
 */ 


#ifndef BUFFER_H_
#define BUFFER_H_

#define MAX_BUF	512
volatile unsigned char buffer[MAX_BUF];

void ClearBuffer();


#endif /* BUFFER_H_ */