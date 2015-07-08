/*
 * File.h
 *
 * Created: 14.8.2014 23:52:49
 *  Author: Daniel
 */ 


#ifndef FILE_H_
#define FILE_H_

//Structure to access Directory Entry in the FAT
struct dir_Structure{ // length 32B
	unsigned char name[11];
	unsigned char attrib; //file attributes
	unsigned char NTreserved; //always 0
	unsigned char timeTenth; //tenths of seconds, set to 0 here
	unsigned int createTime; //time file was created
	unsigned int createDate; //date file was created
	unsigned int lastAccessDate;
	unsigned int firstClusterHI; //higher word of the first cluster number
	unsigned int writeTime; //time of last write
	unsigned int writeDate; //date of last write
	unsigned int firstClusterLO; //lower word of the first cluster number
	unsigned long fileSize; //size of file in bytes
};

//Attribute definitions for file/directory
#define ATTR_READ_ONLY     0x01
#define ATTR_HIDDEN        0x02
#define ATTR_SYSTEM        0x04
#define ATTR_VOLUME_ID     0x08
#define ATTR_DIRECTORY     0x10
#define ATTR_ARCHIVE       0x20
#define ATTR_LONG_NAME     0x0f


unsigned long appendFileSector, appendFileLocation,  appendStartCluster;

unsigned long actualSector, actualCluster,dirCluster;
unsigned int offsetInSector;
unsigned char sectorInCluster,EndTask;
unsigned long fileSize,actualFileBytesPassed;
unsigned char fileName[11];

struct dir_Structure* ActualItem; 

void GoToRoot();
unsigned char FindFile();
unsigned char OpenFileForRead();
unsigned char SetFileName (unsigned char *fileNameptr);
unsigned int ReadFile();

unsigned char CreateFile();
unsigned char AppendToFile(unsigned char *data, unsigned int dataLength);

#endif /* FILE_H_ */