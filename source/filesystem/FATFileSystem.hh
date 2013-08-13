/*
 * FATFileSystem.hh
 *
 *  Created on: 19.06.2013
 *      Author: dbaldin
 */

#ifndef FATFILESYSTEM_HH_
#define FATFILESYSTEM_HH_

#include "FileSystemBase.hh"
#include "filesystem/Directory.hh"
#include "filesystem/File.hh"

typedef struct __attribute__((packed)) {
	char 	BS_jmpBoot[3];
	char 	BS_OEMName[8];
	unint2  BPB_BytsPerSec;
	unint1  BPB_SecPerClus;
	unint2  BPB_RsvdSecCnt;
	unint1  BPB_NumFATs;
	unint2  BPB_RootEntCnt;
	unint2  BPB_TotSec16;
	unint1  BPB_Media;
	unint2  BPB_FATSz16;
	unint2  BPB_SecPerTrk;
	unint2  BPB_NumHeads;
	unint4  BPB_HiddSec;
	unint4  BPB_TotSec32;

} FAT_BS_BPB;

typedef struct __attribute__((packed)) {
	unint1 BS_DrvNum;
	unint1 BS_Reserved1;
	unint1 BS_BootSig;
	unint4 BS_VolID;
	char   BS_VolLab[11];
	char   BS_FilSysType[8];
} FAT16_BPB;

typedef struct __attribute__((packed)) {
	unint4 BPB_FATSz32;
	unint2 BPB_ExtFlags;
	unint2 BPB_FSVer;
	unint4 BPB_RootClus;
	unint2 BPB_FSInfo;
	unint2 BPB_BkBootSec;
	char   BPB_Reserved[12];
	unint1 BS_DrvNum;
	unint1 Reserved1;
	unint1 BS_BootSig;
	unint4 BS_VolID;
	char   BS_VolLab[11];
	char   BS_FilSysType[8];

} FAT32_BPB;


typedef union {
	FAT16_BPB myFAT16_BPB;
	FAT32_BPB myFAT32_BPB;
} FATxx_BPB;

typedef struct __attribute__((packed)) {
	unint4 FSI_LeadSig;
	unint1 FSI_Reserved[480];
	unint4 FSI_StrucSig;
	unint4 FSI_FreeCount;
	unint4 FSI_Nxt_Free;
} FAT32_FSInfo;

typedef struct __attribute__((packed)) {
	char 	DIR_Name[11];
	unint1	DIR_Attr;
	unint1  DIR_NTRes;
	unint1	DIR_CrtTimeTenth;
	unint2	DIR_CrtTime;
	unint2	DIR_CrtDate;
	unint2	DIR_LstAccDate;
	unint2	DIR_FstClusHI;
	unint2	DIR_WrtTime;
	unint2 	DIR_WrtDate;
	unint2	DIR_FstClusLO;
	unint4	DIR_FileSize;
} FAT32_DirEntry;

typedef struct __attribute__((packed)) {
	unint1	LDIR_Ord;
	char 	LDIR_Name[10];
	unint1	LDIR_Attr;
	unint1  LDIR_Type;
	unint1	LDIR_Chksum;
	char	LDIR_Name2[12];
	unint2	LDIR_FstClusLO;
	char    LDIR_Name3[4];
} FAT32_LongDirEntry;


//File attributes:
#define ATTR_READ_ONLY   	0x01
#define ATTR_HIDDEN 		0x02
#define ATTR_SYSTEM 		0x04
#define ATTR_VOLUME_ID 		0x08
#define ATTR_DIRECTORY		0x10
#define ATTR_ARCHIVE  		0x20


#define ATTR_LONG_NAME	(ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)


#define ATTR_LONG_NAME_MASK		(ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID | ATTR_DIRECTORY | ATTR_ARCHIVE)



typedef enum {
	FAT12,
	FAT16,
	FAT32
} FAT_Type;

#define EOC 0x0FFFFFFF
#define CLUSTER_TO_SECTOR(clusternum)  FirstDataSector + ((clusternum -2) * myFAT_BPB.BPB_SecPerClus);

class FATDirectory;

class FATFileSystem: public FileSystemBase {
friend class FATDirectory;
friend class FATFile;
private:
	FAT_Type myFatType;

	FAT_BS_BPB myFAT_BPB;

	FATxx_BPB myFATxx_BPB;

	unint4 RootDirSectors;

	unint4 FirstDataSector;

	unint4 CountOfClusters;

	unint4 DataSec;

	FATDirectory *rootDir;

	unint4 getNextSector(unint4 currentSector, unint4 &currentCluster);

	unint4 getFATTableEntry(unint4 clusterNum);
public:
	FATFileSystem(Partition* myPartition);

	virtual ~FATFileSystem();

	unint4 ClusterToSector(unint4 clusternum) {
		 return FirstDataSector + ((clusternum -2) * myFAT_BPB.BPB_SecPerClus);

	}

	ErrorT initialize();
};

class FATFile: public File {

private:
	// The start cluster of this file
	unint4 clusterStart;

	// The current sector we are in
	// Base::position can be used to calculate the current byte in the sector
	unint4 currentSector;


	unint4 currentCluster;

	// my file system
	FATFileSystem * myFS;

public:
	FATFile(char* name, FAT32_DirEntry* entry, FATFileSystem * fs);

	virtual ~FATFile();

	ErrorT readBytes( char *bytes, unint4 &length );

	ErrorT resetPosition();
};


/*!
 *	Class representing a FAT Filesyste directory.
 *
 *  The contents is populated on demand to safe memory.
 */
class FATDirectory : public Directory {

private:
	// shortcut to my directory entry cluter number
	unint4 mycluster_num;

	// the starting sector of this director
	unint4 sector;

	// pointer to the filesystem we are located in
	FATFileSystem* myFS;

	// has this directory been populated?
	bool 	populated;

	// reads the file system directory and creates
	// ORCOS directory objects for them
	void 	populateDirectory();

public:
	FATDirectory(FATFileSystem* parentFileSystem, unint4 cluster_num,  const char* name);

	// FAT Directory Constructor for FAT16 Fs
	FATDirectory(FATFileSystem* parentFileSystem, unint4 cluster_num,  const char* name, unint4 sector_num);

    //! Adds another resource to this directory
   //ErrorT add( Resource* res ) { };

    //! gets the resource with name 'name'. Maybe null if nonexistent
   Resource* get( const char* name );

   //! Returns the amount of entries in this directory
   int getNumEntries() {
	   if (!populated) populateDirectory();

	   return num_entries;
   }

   //! Returns the content of this directory
   LinkedListDatabase* getContent() {
	   if (!populated) populateDirectory();

	   return &dir_content;
   }

    //! Returns the contents information of this directory as encoded string
    ErrorT readBytes( char *bytes, unint4 &length );

	~FATDirectory();
};


#endif /* FATFILESYSTEM_HH_ */

