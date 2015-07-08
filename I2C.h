/*
 * I2C.h
 *
 * Created: 17.8.2014 13:07:00
 *  Author: Daniel
 */ 

//low level I2C functions
#ifndef I2C_H_
#define I2C_H_

void I2C_Init();
void I2C_Start(void);
void I2C_Stop(void);
void I2C_Write(unsigned char data);
unsigned char I2C_ReadACK(void);
unsigned char I2C_ReadNACK(void);

#endif /* I2C_H_ */