#include "sockets.h"
#include "WebServer.h"
#include "w5100.h"
#include "Buffer.h"
#include "UART.h"
#include "FAT32.h"
#include "File.h"
#include <avr/io.h>
#include <string.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

char WebServerEnabled = 1;
#define  HTTP_PORT					80	

void WebServerLoop()	{
	unsigned char SocketStatus;
	while (WebServerEnabled){
		SocketStatus = W51_read(W5100_SKT_REG_BASE+W5100_SR_OFFSET);
		switch  (SocketStatus)		// based on current status of socket...
		{
			case  W5100_SKT_SR_CLOSED:						// if socket is closed open it
				if (Socket_Open(W5100_SKT_MR_TCP, HTTP_PORT) == 0 ){
					Socket_Listen();
				}
				break;
				
			case  W5100_SKT_SR_ESTABLISHED:
				HandleEstabilishedConnection();
				break;
				
			case  W5100_SKT_SR_FIN_WAIT:
			case  W5100_SKT_SR_CLOSING:
			case  W5100_SKT_SR_TIME_WAIT:
			case  W5100_SKT_SR_CLOSE_WAIT:
			case  W5100_SKT_SR_LAST_ACK:
				Socket_Close();
				break;	
		}
	}
}
/*
unsigned char SendFile ( unsigned char *fileName)
{
	struct dir_Structure *dir;
	unsigned long cluster, byteCounter = 0, fileSize, firstSector;
	unsigned int datalength;
	unsigned char j;

	if(convertFileName(fileName)){
		ClearBuffer();
		strcpy_P((char *)buffer, PSTR("HTTP/1.1 500 filename_parse_failed\r\n\r\n"));
		Socket_Write( buffer, strlen((char *)buffer));
		return 5; //error 500
	}
	if((dir=findFiles (GET_FILE, fileName))==0) {
		ClearBuffer();
		strcpy_P((char *)buffer, PSTR("HTTP/1.1 404 \r\n\r\n"));
		Socket_Write( buffer, strlen((char *)buffer));
		return 4; //error 404
	}

	//cluster = (((unsigned long) dir->firstClusterHI) << 16) | dir->firstClusterLO;

	fileSize = dir->fileSize;
	ClearBuffer();
	strcpy_P((char *)buffer, PSTR("HTTP/1.0 200 OK\r\n"));
	Socket_Write( buffer, strlen((char *)buffer));
	datalength = 512;
	while(1){
		firstSector = getFirstSector (cluster);
		for(j=0; j<sectorPerCluster; j++){
			SD_readSingleBlock(firstSector + j);
			if((byteCounter+512)>fileSize){
				datalength = fileSize-byteCounter;
			}
			byteCounter += 512;
			Socket_Write( buffer, datalength);
			
		}
		cluster = getSetNextCluster (cluster, GET, 0);
		if(cluster == 0) {UART_transmitString(("Error in getting cluster")); return 0;}
	}
	return 0;
}
*/



void HandleEstabilishedConnection(){
	unsigned int rsize = Socket_Available();					// find out how many bytes
	unsigned int i;
	if (rsize > 0){
		if (Socket_Read(buffer, rsize) != W5100_OK)  return;	// if we had problems, all done
		//copy up to first 64b to cache
		int startIndex =0;
		int endIndex = 0;
		for (i=startIndex; i<rsize;i++){
			Requestheadercache[i] = buffer[i];
			if (buffer[i]=='\n'){
				endIndex=i;
				Requestheadercache[i]=0;
				break;
			}
		}


		for (i=0; i<endIndex;i++){ // find first /
			if (Requestheadercache[i]=='/'){
				startIndex=i+1;
				break;
			}
		} //cut on end of file name
		for (i=startIndex; i<endIndex;i++){
			if (Requestheadercache[i]==' '){
				endIndex=i;
				Requestheadercache[i]=0;
				break;
			}
		}
		
		UART_transmitString(("Request cache content:"));
		UART_transmitString(Requestheadercache);
		TX_NEWLINE
	
		if (startIndex==endIndex){ // file name = /
			UART_transmitString("file name:root\n");
			strcpy_P((char *)buffer, PSTR("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"));
			strcat_P((char *)buffer, PSTR("<html>\r\n<body>\r\n"));
			strcat_P((char *)buffer, PSTR("<h2>web server using Wiznet W5100 chip</h2>\r\n"));
			strcat_P((char *)buffer, PSTR("<br /><hr>\r\n"));
			if (Socket_Write( buffer, strlen((char *)buffer)) == W5100_FAIL) {
				UART_transmitString(PSTR("Socket write failed\n"));
				Socket_Close();
				Socket_Disconect();
			}
		}
		else{
		
			TX_NEWLINE
			unsigned char folderCount = 0;
			unsigned char webfileName[13];
			for (i=startIndex; i<endIndex;i++){
				if(Requestheadercache[i]=='/') folderCount++;
			}
			GoToRoot();
			
			while(folderCount){
				UART_transmitString("start");
				UART_transmitNumber(startIndex);
				UART_transmitString("end");
				UART_transmitNumber(endIndex);
				TX_NEWLINE
				for (i=0; i<13;i++){
					webfileName[i] = 0;
				}
				for (i=startIndex; i<endIndex;i++){
					if(Requestheadercache[i]=='/') break;
					webfileName[i-startIndex] = Requestheadercache[i];
				}
				webfileName[i-startIndex] = '.';
				webfileName[i-startIndex+1] = 0;
				UART_transmitString("folder name:");
				UART_transmitString(webfileName);
				TX_NEWLINE
				startIndex=i+1;
				folderCount--;
				if(SetFileName(webfileName)==0){
					UART_transmitString("folder name set");
				}
				if(OpenFolder()>0){
					UART_transmitString("folder not found");
					break;// to do error 404
				}
			}
			
			for (i=0; i<13;i++){
				webfileName[i] = 0;
			}
			UART_transmitString("start");
			UART_transmitNumber(startIndex);
			UART_transmitString("end");
			UART_transmitNumber(endIndex);
			TX_NEWLINE
			UART_transmitString("file name:");
			for (i=startIndex; i<endIndex;i++){
				webfileName[i-startIndex] = Requestheadercache[i];
				UART_transmitByte(Requestheadercache[i]);
			}
			
			UART_transmitString(":");
			TX_NEWLINE

			SetFileName(webfileName);
			if(FindFile()==0){
				OpenFileForRead();
				unsigned int cnt;
				while ((cnt=ReadFile())>0){
					Socket_Write(buffer,cnt);
				}
			}
			else{
				strcpy_P((char *)buffer, PSTR("HTTP/1.1 404 Not found\r\nContent-Type: text/html\r\n\r\n"));
				strcat_P((char *)buffer, PSTR("<html>\r\n<body>\r\n"));
				strcat_P((char *)buffer, PSTR("<h2>file not found</h2>\r\n"));
				strcat_P((char *)buffer, PSTR("<br /><hr>\r\n"));
				if (Socket_Write( buffer, strlen((char *)buffer)) == W5100_FAIL) {
					Socket_Close();
					Socket_Disconect();
				}
			}
			//SendFile(fileName);
			//readFile(GET_FILE,fileName);
		}
				// just throw out the packet for now
		Socket_Close();
		Socket_Disconect();
	}
	else											// no data yet...
	{
		_delay_us(10);
	}
}

