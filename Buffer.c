/*
 * Buffer.c
 *
 * Created: 14.8.2014 2:51:06
 *  Author: Daniel
 */ 
#include "Buffer.h"

void ClearBuffer(){
	for (int i=0;i<MAX_BUF;i++){
		buffer[i] = 0;
	}
}