// low level SD routines
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "SPI.h"
#include "SD.h"
#include "Buffer.h"

unsigned char SD_init(void){
	unsigned char i, response, retry=0 ;

	SD_CS_ASSERT;
	do{
	   for(i=0;i<10;i++){
		  SPI_transmit(0xff);
	   }
	   response = SD_sendCommand(GO_IDLE_STATE, 0); //send 'reset & go idle' command
	   retry++;
	   if(retry>0xfe) {
		  return 1;
	   } //time out
	} while(response != 0x01);

	SD_CS_DEASSERT;
	
	SPI_transmit (0xff);
	SPI_transmit (0xff);
	
	retry = 0;
	do{
		response = SD_sendCommand(SEND_OP_COND, 0); //activate card's initialization process
		response = SD_sendCommand(SEND_OP_COND, 0); //same command sent again for compatibility with some cards
		retry++;
		if(retry>0xfe) return 1; //time out
	}while(response);

	SD_sendCommand(CRC_ON_OFF, OFF); //disable CRC; deafault - CRC disabled in SPI mode
	SD_sendCommand(SET_BLOCK_LEN, 512); //set block size to 512

	return 0; //normal return
}

unsigned char SD_sendCommand(unsigned char cmd, unsigned long arg){
	unsigned char response, retry=0;
	
	SD_CS_ASSERT;
	
	SPI_transmit(cmd | 0x40); //send command, first two bits always '01'
	SPI_transmit(arg>>24);
	SPI_transmit(arg>>16);
	SPI_transmit(arg>>8);
	SPI_transmit(arg);
	SPI_transmit(0x95);

	while((response = SPI_receive()) == 0xff) //wait response
	   if(retry++ > 0xfe) break; //time out error

	SPI_receive(); //extra 8 CLK
	SD_CS_DEASSERT;

	return response; //return state
}

//*****************************************************************
//Function: to erase specified no. of blocks of SD card
//Arguments: none
//return: unsigned char; will be 0 if no error,
// otherwise the response byte will be sent
//*****************************************************************
unsigned char SD_erase (unsigned long startBlock, unsigned long totalBlocks)
{
unsigned char response;

response = SD_sendCommand(ERASE_BLOCK_START_ADDR, startBlock<<9); //send starting block address
if(response != 0x00) //check for SD status: 0x00 - OK (No flags set)
  return response;

response = SD_sendCommand(ERASE_BLOCK_END_ADDR,(startBlock + totalBlocks - 1)<<9); //send end block address
if(response != 0x00)
  return response;

response = SD_sendCommand(ERASE_SELECTED_BLOCKS, 0); //erase all selected blocks
if(response != 0x00)
  return response;

return 0; //normal return
}

unsigned char SD_readSingleBlock(unsigned long startBlock)
{
unsigned char response;
unsigned int i, retry=0;

response = SD_sendCommand(READ_SINGLE_BLOCK, startBlock<<9); //read a Block command
//block address converted to starting address of 512 byte Block
if(response != 0x00) //check for SD status: 0x00 - OK (No flags set)
  return response;

SD_CS_ASSERT;

retry = 0;
while(SPI_receive() != 0xfe) //wait for start block token 0xfe (0x11111110)
  if(retry++ > 0xfffe){SD_CS_DEASSERT; return 1;} //return if time-out

for(i=0; i<512; i++) //read 512 bytes
  buffer[i] = SPI_receive();

SPI_receive(); //receive incoming CRC (16-bit), CRC is ignored here
SPI_receive();

SPI_receive(); //extra 8 clock pulses
SD_CS_DEASSERT;

return 0;
}

unsigned char SD_writeSingleBlock(unsigned long startBlock) {
	unsigned char response;
	unsigned int i, retry=0;

	response = SD_sendCommand(WRITE_SINGLE_BLOCK, startBlock<<9); //write a Block command
	if(response != 0x00) //check for SD status: 0x00 - OK (No flags set)
	return response;

	SD_CS_ASSERT;

	SPI_transmit(0xfe);     //Send start block token 0xfe (0x11111110)

	for(i=0; i<512; i++)    //send 512 bytes data
	  SPI_transmit(buffer[i]);

	SPI_transmit(0xff);     //transmit dummy CRC (16-bit), CRC is ignored here
	SPI_transmit(0xff);

	response = SPI_receive();

	if( (response & 0x1f) != 0x05) //response= 0xXXX0AAA1 ; AAA='010' - data accepted
	{                              //AAA='101'-data rejected due to CRC error
	  SD_CS_DEASSERT;              //AAA='110'-data rejected due to write error
	  return response;
	}

	while(!SPI_receive()) //wait for SD card to complete writing and get idle
	if(retry++ > 0xfffe){SD_CS_DEASSERT; return 1;}

	SD_CS_DEASSERT;
	SPI_transmit(0xff);   //just spend 8 clock cycle delay before reasserting the CS line
	SD_CS_ASSERT;         //re-asserting the CS line to verify if card is still busy

	while(!SPI_receive()) //wait for SD card to complete writing and get idle
	   if(retry++ > 0xfffe){SD_CS_DEASSERT; return 1;}
	SD_CS_DEASSERT;

	return 0;
}
