/*
 * File.c
 *
 * Created: 14.8.2014 23:13:34
 *  Author: Daniel
 */ 
#include "FAT32.h"
#include "Buffer.h"
#include "File.h"
#include "UART.h"
#include <avr/pgmspace.h>

void GoToRoot(){
	dirCluster = rootCluster;
}

void OpenFileList(){
	actualCluster = dirCluster;
	actualSector = FAT_getFirstSector(actualCluster);
	sectorInCluster = 0;
	offsetInSector = 0;
	SD_readSingleBlock (actualSector + sectorInCluster);
	EndTask = 0;
}

unsigned char ReadFileListItem(){
	if (offsetInSector>=bytesPerSector){
		sectorInCluster++;
		if (sectorInCluster>=sectorPerCluster){
			actualCluster = (FAT_getNextCluster (actualCluster));
			if(actualCluster > 0x0ffffff6){ 
				EndTask = 1;
				return 2;
			}
			actualSector = FAT_getFirstSector(actualCluster);
			sectorInCluster=0;
		}
		offsetInSector = 0;

		SD_readSingleBlock (actualSector + sectorInCluster);
	}
	ActualItem = (struct dir_Structure *) &buffer[offsetInSector];
	if(ActualItem->name[0] == EMPTY){
		EndTask = 1;
		return 1; 
	}
	offsetInSector+=32;
	return 0;
}

unsigned char FindFile(){
	OpenFileList(dirCluster);
	char j;
	while(ReadFileListItem()==0){
		for(j=0; j<11; j++){
			if(ActualItem->name[j]!=fileName[j]) break;
		}
		if(j!=11) continue;
		return 0;
	}	
	return 1; // not found
}

unsigned char OpenFileForRead(){
	actualCluster = (((unsigned long) ActualItem->firstClusterHI) << 16) | ActualItem->firstClusterLO;
	if (actualCluster==0) return 1;
	if (actualCluster > 0x0ffffff6) return 1;
	fileSize = ActualItem->fileSize;
	actualFileBytesPassed = 0;
	sectorInCluster = 0;
	actualSector = FAT_getFirstSector(actualCluster);
	
	return 0;
}

unsigned char SetFileName (unsigned char *fileNameptr){
	unsigned char dotPos;
	unsigned char  k;
	for(k=0; k<11; k++){
		if(fileNameptr[k] == '.'){ 
			dotPos = k;	
			break;
		}
	}
	if(dotPos>8) return 1;

	for(k=0; k<dotPos; k++){ //setting file name}
		fileName[k] = fileNameptr[k];
	}
	for(k=dotPos; k<=7; k++){
		fileName[k] = ' ';
	}
	dotPos++;
	for(k=8; k<11; k++){ //setting file extention
		if(fileNameptr[dotPos] != 0){
			fileName[k] = fileNameptr[dotPos];
			dotPos++;
		}
		else{//filling extension trail with blanks
			while(k<11){
				fileName[k] = ' ';
				k++;	
			}
		} 
		
	}
	for(k=0; k<11; k++){ //converting small letters to caps
		if((fileName[k] >= 0x61) && (fileName[k] <= 0x7a)){
			fileName[k] -= 0x20;
		}
	}

	return 0;
}

unsigned int ReadFile(){ //return byte readed
	if(actualFileBytesPassed >= fileSize) return 0;
	if (sectorInCluster>=sectorPerCluster){
		actualCluster = (FAT_getNextCluster (actualCluster));
		if(actualCluster > 0x0ffffff6){ return 0; }
		actualSector = FAT_getFirstSector(actualCluster);
		sectorInCluster=0;
	}
	actualFileBytesPassed = actualFileBytesPassed+512;
	SD_readSingleBlock (actualSector + sectorInCluster);
	sectorInCluster++;
	if (actualFileBytesPassed >= fileSize){
		actualFileBytesPassed = fileSize;
		return (int)(fileSize - (actualFileBytesPassed - MAX_BUF));
	}
	return MAX_BUF;	
}

void SendBufferOverUart(){
	unsigned int k;
	unsigned int byteCounter;
	for(k=0; k<512; k++){
		UART_transmitByte(buffer[k]);
		if ((byteCounter++) >= fileSize ) return;
	}
}

void ReadFileUART(){
	OpenFileForRead();
	while(ReadFile()>0)
	{
		SendBufferOverUart();
	}
}

void SendFileList(){
		UART_transmitString(PSTR("files:\n"));
		while(ReadFileListItem()==0){
			UART_transmitNumber(offsetInSector-32);UART_transmitByte(':');
			if((ActualItem->name[0] == EMPTY)||(ActualItem->name[0] == DELETED) || (ActualItem->attrib == ATTR_LONG_NAME)){
				TX_NEWLINE; 
				continue;
			}
			UART_transmitByte('"');
			for(char j=0; j<11; j++){
				UART_transmitByte (ActualItem->name[j]);
			}
			UART_transmitByte('"');
			UART_transmitHex(LONG,ActualItem->fileSize);
			TX_NEWLINE;
		}
}

unsigned char OpenFolder(){
	if (FindFile()>0) return 2;
	dirCluster = ((((unsigned long) ActualItem->firstClusterHI) << 16) | ActualItem->firstClusterLO);
	if (dirCluster > 0x0ffffff6) return 1;
	if (dirCluster==0) return 1;
	return 0;
}

void ReadRoot2(){
	dirCluster = rootCluster;
	OpenFileList();
	SendFileList();
	
	//UART_transmitString("fldr:\n");
	SetFileName(PSTR("fldr."));
	if(OpenFolder()==0){
		OpenFileList();
		SendFileList();
		
	/*	SetFileName("file1.txt");

		if (FindFile()==0){
			UART_transmitString("file found:\n\n");
			ReadFileUART();
		}	*/
		SetFileName(PSTR("file7.txt"));
		if (FindFile()==0){
			UART_transmitString(PSTR("file7 found:\n append something\n"));
			for(unsigned char i =0; i<2;i++){
				FindFile();
				AppendToFile(PSTR("hello world\r\nhello world\r\nhello world\r\nhello world\r\nhello world\r\n"),65);
			}
			//FindFile();
			//UART_transmitString("content:\n");
			//ReadFileUART();
		}
		else{
			CreateFile();
			//SendFileList();
		}
	}
}

unsigned char CreateFile(){
	unsigned long newFileCluster;
	unsigned char j;
	  //get next free cluster
	newFileCluster = FAT_GetNextFreeCluster();
	if (newFileCluster>totalClusters){
		newFileCluster = rootCluster;
	}
	newFileCluster = FAT_searchNextFreeCluster(newFileCluster);
	if (newFileCluster==0){
		return 1; // fatal error no usable cluster found
	}
	/*UART_transmitString("free cluster:");
	UART_transmitHex(LONG,newFileCluster);
	UART_transmitString("\n");*/
	FAT_SetNextCluster(newFileCluster,EOF); // cluster marked as alocated
	//UART_transmitString("marked as eof:\n");
 	
	 
	OpenFileList();
	while(ReadFileListItem()!=1);
	//offsetInSector = offsetInSector-32; // because ReadFileListItem() increase sector offset
/*	UART_transmitString("last file found at offset:");
	UART_transmitNumber(offsetInSector-32);98
	ActualItem = (struct dir_Structure *) &buffer[offsetInSector];
/*	UART_transmitString(" at:");
	UART_transmitNumber(offsetInSector);
	UART_transmitString(" is:");
	UART_transmitNumber(ActualItem->attrib);
		
	TX_NEWLINE*/
	//update folder
	for(j=0; j<11; j++){
		ActualItem->name[j] = fileName[j];
	}
	ActualItem->attrib = ATTR_ARCHIVE;	//settting file attribute as 'archive'
	ActualItem->NTreserved = 0;			//always set to 0
	ActualItem->timeTenth = 0;			//always set to 0
	ActualItem->createTime = 0x9684;		//fixed time of creation
	ActualItem->createDate = 0x3a37;		//fixed date of creation
	ActualItem->lastAccessDate = 0x3a37;	//fixed date of last access
	ActualItem->writeTime = 0x9684;		//fixed time of last write
	ActualItem->writeDate = 0x3a37;		//fixed date of last write
	ActualItem->firstClusterHI = (unsigned int) ((newFileCluster & 0xffff0000) >> 16 );
	ActualItem->firstClusterLO = (unsigned int) ( newFileCluster & 0x0000ffff);
	ActualItem->fileSize = 0;
	
	offsetInSector=offsetInSector+32;
	
	if (offsetInSector<bytesPerSector){ // set empty record in actual sector
		ActualItem = (struct dir_Structure *) &buffer[offsetInSector];
		ActualItem->name[0] = EMPTY;
	}
	
	SD_writeSingleBlock (actualSector + sectorInCluster);
	if (offsetInSector>=bytesPerSector){//set empty in next sector
		sectorInCluster++;
		if (sectorInCluster>=sectorPerCluster){
			newFileCluster = FAT_searchNextFreeCluster(newFileCluster);
			if (newFileCluster==0){
				return 1; // fatal error no usable cluster found
			}
			FAT_SetNextCluster(actualCluster,newFileCluster); 
			FAT_SetNextCluster(newFileCluster,EOF); // cluster marked as alocated
			actualSector = FAT_getFirstSector(newFileCluster);
			sectorInCluster=0;
		}
		offsetInSector = 0;
	}
		SD_readSingleBlock (actualSector + sectorInCluster);
		ActualItem = (struct dir_Structure *) &buffer[offsetInSector];
		ActualItem->name[0] = EMPTY;
		SD_writeSingleBlock (actualSector + sectorInCluster);
	
}

unsigned char AppendToFile(unsigned char *data, unsigned int dataLength){ //use find file before append maximal appending size is be 512b
	unsigned int fileCluesterCount,lastClusterBytes,bufferPointer, appended,blockSize,i; 
	unsigned long newFileCluster,fileEntrySector;

	appended = 0;
	fileEntrySector = actualSector;
	blockSize = sectorPerCluster*bytesPerSector;
	fileSize = ActualItem->fileSize;
		
	fileCluesterCount = (unsigned int)(fileSize/blockSize);
	lastClusterBytes = (unsigned int)(fileSize % blockSize);
	actualCluster = (((unsigned long) ActualItem->firstClusterHI) << 16) | ActualItem->firstClusterLO;
	for (i=0;i<fileCluesterCount;i++){//iterate over cluster chain
		actualCluster= FAT_getNextCluster(actualCluster);
		if (actualCluster==0) return 1;
		if (actualCluster > 0x0ffffff6) return 1;
	}
	
	actualSector = FAT_getFirstSector(actualCluster);
	sectorInCluster = lastClusterBytes/bytesPerSector;	
	SD_readSingleBlock (actualSector + sectorInCluster); //read first sector
	bufferPointer = lastClusterBytes%bytesPerSector;
	for (i=0; i<dataLength; i++){//append data
		buffer[bufferPointer] = data[appended];
		bufferPointer++;
		appended++;
		if (bufferPointer==MAX_BUF) break;
	}
	SD_writeSingleBlock(actualSector + sectorInCluster); //write first sector
	if (appended < dataLength){  // all data not fit into one sector
		sectorInCluster++;		//lets get next sector
		if (sectorInCluster>=sectorPerCluster){//if necesary allocated next cluster
			newFileCluster = FAT_searchNextFreeCluster(newFileCluster);
			if (newFileCluster==0){
				return 1; // fatal error no usable cluster found
			}
			FAT_SetNextCluster(actualCluster,newFileCluster);
			FAT_SetNextCluster(newFileCluster,EOF);
			actualSector = FAT_getFirstSector(newFileCluster);
			sectorInCluster=0;
		}
		bufferPointer = 0;
		for (i=0; i<dataLength; i++){
			buffer[bufferPointer] = data[appended];
			bufferPointer++;
			appended++;
			if (bufferPointer==MAX_BUF) return 3;
		}
		SD_writeSingleBlock(actualSector + sectorInCluster); //write to 2nd sector;
	}
	//update directory
	SD_readSingleBlock(fileEntrySector);
	ActualItem = (struct dir_Structure *) &buffer[offsetInSector-32];
	fileSize = fileSize + dataLength;
	ActualItem->fileSize = fileSize;
	SD_writeSingleBlock(fileEntrySector);
	return 0;
}
