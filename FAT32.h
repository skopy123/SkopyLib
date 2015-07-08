#ifndef _FAT32_H_
#define _FAT32_H_

struct MBRinfo_Structure{
unsigned char	nothing[446];		//ignore, placed here to fill the gap in the structure
unsigned char	partitionData[64];	//partition records (16x4)
unsigned int	signature;		//0xaa55
};

struct partitionInfo_Structure{ 				
unsigned char	status;				//0x80 - active partition
unsigned char 	headStart;			//starting head
unsigned int	cylSectStart;		//starting cylinder and sector
unsigned char	type;				//partition type 
unsigned char	headEnd;			//ending head of the partition
unsigned int	cylSectEnd;			//ending cylinder and sector
unsigned long	firstSector;		//total sectors between MBR & the first sector of the partition
unsigned long	sectorsTotal;		//size of this partition in sectors
};

//Structure to access boot sector data
struct BS_Structure{
unsigned char jumpBoot[3]; //default: 0x009000EB
unsigned char OEMName[8];
unsigned int bytesPerSector; //deafault: 512
unsigned char sectorPerCluster;
unsigned int reservedSectorCount;
unsigned char numberofFATs;
unsigned int rootEntryCount;
unsigned int totalSectors_F16; //must be 0 for FAT32
unsigned char mediaType;
unsigned int FATsize_F16; //must be 0 for FAT32
unsigned int sectorsPerTrack;
unsigned int numberofHeads;
unsigned long hiddenSectors;
unsigned long totalSectors_F32;
unsigned long FATsize_F32; //count of sectors occupied by one FAT
unsigned int extFlags;
unsigned int FSversion; //0x0000 (defines version 0.0)
unsigned long rootCluster; //first cluster of root directory (=2)
unsigned int FSinfo; //sector number of FSinfo structure (=1)
unsigned int BackupBootSector;
unsigned char reserved[12];
unsigned char driveNumber;
unsigned char reserved1;
unsigned char bootSignature;
unsigned long volumeID;
unsigned char volumeLabel[11]; //"NO NAME "
unsigned char fileSystemType[8]; //"FAT32"
unsigned char bootData[420];
unsigned int bootEndSignature; //0xaa55
};

//Structure to access FSinfo sector data
struct FSInfo_Structure
{
unsigned long leadSignature; //0x41615252
unsigned char reserved1[480];
unsigned long structureSignature; //0x61417272
unsigned long freeClusterCount; //initial: 0xffffffff
unsigned long nextFreeCluster; //initial: 0xffffffff
unsigned char reserved2[12];
unsigned long trailSignature; //0xaa550000
};

#define DIR_ENTRY_SIZE     0x32
#define EMPTY              0x00
#define DELETED            0xe5
#define GET     0
#define SET     1
#define READ	0
#define VERIFY  1
#define ADD		0
#define REMOVE	1
#define TOTAL_FREE   1
#define NEXT_FREE    2
#define GET_LIST     0
#define GET_FILE     1
#define DELETE		 2
#define EOF		0x0fffffff

//************* external variables *************
unsigned long firstDataSector, rootCluster, totalClusters,unusedSectors;
unsigned int  bytesPerSector, sectorPerCluster, reservedSectorCount;
unsigned char freeClusterCountUpdated;


unsigned long dataSectors;
//************* functions *************
unsigned char FAT_getBootSectorData (void);
unsigned long FAT_getFirstSector(unsigned long clusterNumber);
unsigned long FAT_getNextCluster (unsigned long clusterNumber);
unsigned long FAT_SetNextCluster (unsigned long clusterNumber, unsigned long nextcluster);
unsigned long FAT_searchNextFreeCluster (unsigned long startCluster);
unsigned long FAT_TotalSpace();
unsigned long FAT_FreeSpace();

unsigned long FAT_GetFreeClusterCount();
unsigned long FAT_GetNextFreeCluster();
void FAT_SetFreeClusterCount(unsigned long data);
void FAT_SetNextFreeCluster(unsigned long data);


void freeMemoryUpdate (unsigned char flag, unsigned long size);

#endif