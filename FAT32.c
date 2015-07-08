//low level FAT32 functions
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "FAT32.h"
#include "SD.h"
#include "Buffer.h"


unsigned char FAT_getBootSectorData (void){
	struct BS_Structure *bpb; //mapping the buffer onto the structure
	struct MBRinfo_Structure *mbr;
	struct partitionInfo_Structure *partition;
	unsigned long dataSectors;

	unusedSectors = 0;

	SD_readSingleBlock(0);
	bpb = (struct BS_Structure *)buffer;

	if(bpb->jumpBoot[0]!=0xE9 && bpb->jumpBoot[0]!=0xEB)   //check if it is boot sector
	{
		mbr = (struct MBRinfo_Structure *) buffer;       //if it is not boot sector, it must be MBR
	
		if(mbr->signature != 0xaa55) return 1;       //if it is not even MBR then it's not FAT32
	
		partition = (struct partitionInfo_Structure *)(mbr->partitionData);//first partition
		unusedSectors = partition->firstSector; //the unused sectors, hidden to the FAT
	
		SD_readSingleBlock(partition->firstSector);//read the bpb sector
		bpb = (struct BS_Structure *)buffer;
		if(bpb->jumpBoot[0]!=0xE9 && bpb->jumpBoot[0]!=0xEB) return 1;
	}

	bytesPerSector = bpb->bytesPerSector;
	sectorPerCluster = bpb->sectorPerCluster;
	reservedSectorCount = bpb->reservedSectorCount;
	rootCluster = bpb->rootCluster;// + (sector / sectorPerCluster) +1;
	firstDataSector = bpb->hiddenSectors + reservedSectorCount + (bpb->numberofFATs * bpb->FATsize_F32);

	dataSectors = bpb->totalSectors_F32
	- bpb->reservedSectorCount
	- ( bpb->numberofFATs * bpb->FATsize_F32);
	totalClusters = dataSectors / sectorPerCluster;
	
	if(FAT_GetFreeClusterCount() > totalClusters){  //check if FSinfo free clusters count is valid
		freeClusterCountUpdated = 0;
	}
	else{
		freeClusterCountUpdated = 1;
	}
	return 0;
}

//get first sector of cluster
unsigned long FAT_getFirstSector(unsigned long clusterNumber){
  return (((clusterNumber - 2) * sectorPerCluster) + firstDataSector);
}

unsigned long FAT_getNextCluster (unsigned long clusterNumber){
	unsigned int FATEntryOffset;
	unsigned long *FATEntryValue;
	unsigned long FATEntrySector;
	unsigned char retry = 0;
	//get sector number of the cluster entry in the FAT
	FATEntrySector = unusedSectors + reservedSectorCount + ((clusterNumber * 4) / bytesPerSector) ;
	//get the offset address in that sector number
	FATEntryOffset = (unsigned int) ((clusterNumber * 4) % bytesPerSector);
	//read the sector into a buffer
	while(retry <10){ 
		if(!SD_readSingleBlock(FATEntrySector)) break; 
		retry++;
	}
	//get the cluster address from the buffer
	FATEntryValue = (unsigned long *) &buffer[FATEntryOffset];
	return ((*FATEntryValue) & 0x0fffffff);
}

unsigned long FAT_SetNextCluster (unsigned long clusterNumber, unsigned long nextcluster){
	unsigned int FATEntryOffset;
	unsigned long *FATEntryValue;
	unsigned long FATEntrySector;
	unsigned char retry = 0;

	//get sector number of the cluster entry in the FAT
	FATEntrySector = unusedSectors + reservedSectorCount + ((clusterNumber * 4) / bytesPerSector) ;

	//get the offset address in that sector number
	FATEntryOffset = (unsigned int) ((clusterNumber * 4) % bytesPerSector);

	//read the sector into a buffer
	while(retry <10){ 
		if(!SD_readSingleBlock(FATEntrySector)) break; 
		retry++;
	}
	//get the cluster address from the buffer
	FATEntryValue = (unsigned long *) &buffer[FATEntryOffset];
	*FATEntryValue = nextcluster;   //for setting new value in cluster entry in FAT
	SD_writeSingleBlock(FATEntrySector);
	return (0);
}

unsigned long FAT_searchNextFreeCluster (unsigned long startCluster){
	unsigned long cluster, *val, sector;
	unsigned char i;
	startCluster -=  (startCluster % 128);   //to start with the first file in a FAT sector
	
    for(cluster =startCluster; cluster <totalClusters; cluster+=128) {
		sector = unusedSectors + reservedSectorCount + ((cluster * 4) / bytesPerSector);
		SD_readSingleBlock(sector);
		for(i=0; i<128; i++){
			val = (unsigned long *) &buffer[i*4];
			if(((*val) & 0x0fffffff) == 0){
			return(cluster+i);
			}
		}
	}
	return 0;
}

unsigned long FAT_TotalSpace(){
	return totalClusters * sectorPerCluster * bytesPerSector;
}

unsigned long FAT_FreeSpace(){
	unsigned long totalMemory, freeClusters, totalClusterCount, cluster;
	unsigned long sector, *value;
	unsigned int i;

	totalMemory = FAT_TotalSpace();
	freeClusters = FAT_GetFreeClusterCount();   
	if(freeClusters > totalClusters){
	   freeClusterCountUpdated = 0;
	   freeClusters = 0;
	   totalClusterCount = 0;
	   cluster = rootCluster;    
		while(1){
		  sector = unusedSectors + reservedSectorCount + ((cluster * 4) / bytesPerSector) ;
		  SD_readSingleBlock(sector);
		  for(i=0; i<128; i++){
			value = (unsigned long *) &buffer[i*4];
			if(((*value)& 0x0fffffff) == 0){
				freeClusters++;;
			}
			totalClusterCount++;
			if(totalClusterCount == (totalClusters+2)) break;
		  }  
		  if(i < 128){
			   break;
		  }
		  cluster+=128;
		} 
	}

	if(!freeClusterCountUpdated){
		FAT_SetFreeClusterCount(freeClusters); //update FSinfo next free cluster entry
		freeClusterCountUpdated = 1;  //set flag
	}
	return freeClusters * sectorPerCluster * bytesPerSector;
}

unsigned char FAT_CheckSignature(){
	struct FSInfo_Structure *FS = (struct FSInfo_Structure *) &buffer;
	SD_readSingleBlock(unusedSectors + 1);
	if((FS->leadSignature != 0x41615252) || (FS->structureSignature != 0x61417272) || (FS->trailSignature !=0xaa550000)){
		return 0;
	}
	return 1;
}

unsigned long FAT_GetFreeClusterCount(){
	if(FAT_CheckSignature()>0){
		return 0xffffffff;
	}
	struct FSInfo_Structure *FS = (struct FSInfo_Structure *) &buffer;
	return(FS->freeClusterCount);
}

unsigned long FAT_GetNextFreeCluster(){
	if(FAT_CheckSignature()>0){
		return 0xffffffff;
	}
	struct FSInfo_Structure *FS = (struct FSInfo_Structure *) &buffer;
	return(FS->nextFreeCluster);
}

void FAT_SetFreeClusterCount(unsigned long data){
	if(FAT_CheckSignature()>0){
		return;
	}
	struct FSInfo_Structure *FS = (struct FSInfo_Structure *) &buffer;
	FS->freeClusterCount = data;
	SD_writeSingleBlock(unusedSectors + 1);	//update FSinfo
}

void FAT_SetNextFreeCluster(unsigned long data){
	if(FAT_CheckSignature()>0){
		return;
	}
	struct FSInfo_Structure *FS = (struct FSInfo_Structure *) &buffer;
	FS->nextFreeCluster = data;
	SD_writeSingleBlock(unusedSectors + 1);	//update FSinfo
}



//********************************************************************
//Function: update the free memory count in the FSinfo sector. 
//			Whenever a file is deleted or created, this function will be called
//			to ADD or REMOVE clusters occupied by the file
//Arguments: #1.flag ADD or REMOVE #2.file size in Bytes
//return: none
//********************************************************************
void freeMemoryUpdate (unsigned char flag, unsigned long size)
{
  unsigned long freeClusters;
  //convert file size into number of clusters occupied
  if((size % 512) == 0) size = size / 512;
  else size = (size / 512) +1;
  if((size % 8) == 0) size = size / 8;
  else size = (size / 8) +1;

  if(freeClusterCountUpdated)
  {
	freeClusters = getSetFreeCluster (TOTAL_FREE, GET, 0);
	if(flag == ADD)
  	   freeClusters = freeClusters + size;
	else  //when flag = REMOVE
	   freeClusters = freeClusters - size;
	getSetFreeCluster (TOTAL_FREE, SET, freeClusters);
  }
}