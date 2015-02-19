/*
 * FATFileSystem.hh
 *
 *  Created on: 19.06.2013
 *      Copyright & Author: dbaldin
 */

#ifndef FATFILESYSTEM_HH_
#define FATFILESYSTEM_HH_

#include "FileSystemBase.hh"
#include "filesystem/Directory.hh"
#include "filesystem/File.hh"

typedef struct   __attribute__((__packed__, __aligned__(4))) {
    char BS_jmpBoot[3];
    char BS_OEMName[8];
    unint2 BPB_BytsPerSec;
    unint1 BPB_SecPerClus;
    unint2 BPB_RsvdSecCnt;
    unint1 BPB_NumFATs;
    unint2 BPB_RootEntCnt;
    unint2 BPB_TotSec16;
    unint1 BPB_Media;
    unint2 BPB_FATSz16;
    unint2 BPB_SecPerTrk;
    unint2 BPB_NumHeads;
    unint4 BPB_HiddSec;
    unint4 BPB_TotSec32;
} FAT_BS_BPB;

typedef struct   __attribute__((__packed__, __aligned__(4))) {
    unint1 BS_DrvNum;
    unint1 BS_Reserved1;
    unint1 BS_BootSig;
    unint4 BS_VolID;
    char BS_VolLab[11];
    char BS_FilSysType[8];
} FAT16_BPB;

typedef struct   __attribute__((__packed__, __aligned__(4))) {
    unint4 BPB_FATSz32;
    unint2 BPB_ExtFlags;
    unint2 BPB_FSVer;
    unint4 BPB_RootClus;
    unint2 BPB_FSInfo;
    unint2 BPB_BkBootSec;
    char BPB_Reserved[12];
    unint1 BS_DrvNum;
    unint1 Reserved1;
    unint1 BS_BootSig;
    unint4 BS_VolID;
    char BS_VolLab[11];
    char BS_FilSysType[8];
} FAT32_BPB;

typedef union {
    FAT16_BPB myFAT16_BPB;
    FAT32_BPB myFAT32_BPB;
} FATxx_BPB;

typedef struct   __attribute__((__packed__, __aligned__(4))) {
    unint4 FSI_LeadSig;
    unint1 FSI_Reserved[480];
    unint4 FSI_StrucSig;
    unint4 FSI_FreeCount;
    unint4 FSI_Nxt_Free;
} FAT32_FSInfo;

typedef struct   __attribute__((__packed__, __aligned__(4))) {
    char DIR_Name[11];
    unint1 DIR_Attr;
    unint1 DIR_NTRes;
    unint1 DIR_CrtTimeTenth;
    unint2 DIR_CrtTime;
    unint2 DIR_CrtDate;
    unint2 DIR_LstAccDate;
    unint2 DIR_FstClusHI;
    unint2 DIR_WrtTime;
    unint2 DIR_WrtDate;
    unint2 DIR_FstClusLO;
    unint4 DIR_FileSize;
} FAT32_DirEntry;

typedef struct  __attribute__((__packed__, __aligned__(4))) {
    unint1 LDIR_Ord;
    char LDIR_Name[10];
    unint1 LDIR_Attr;
    unint1 LDIR_Type;
    unint1 LDIR_Chksum;
    char LDIR_Name2[12];
    unint2 LDIR_FstClusLO;
    char LDIR_Name3[4];
} FAT32_LongDirEntry;

typedef enum {
   FAT12, FAT16, FAT32
} FAT_Type;

//File attributes:
#define ATTR_READ_ONLY      0x01
#define ATTR_HIDDEN         0x02
#define ATTR_SYSTEM         0x04
#define ATTR_VOLUME_ID      0x08
#define ATTR_DIRECTORY      0x10
#define ATTR_ARCHIVE        0x20

#define ATTR_LONG_NAME          (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)
#define ATTR_LONG_NAME_MASK     (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID | ATTR_DIRECTORY | ATTR_ARCHIVE)
#define EOC                     0x0FFFFFFF

#define CLUSTER_TO_SECTOR(clusternum)  FirstDataSector + ((clusternum -2) * myFAT_BPB.BPB_SecPerClus);

class FATDirectory;

class FATFileSystem: public FileSystemBase {
    friend class FATDirectory;
    friend class FATFile;
private:
    FAT_Type    myFatType;

    /* copy of the FAT BS BPB */
    FAT_BS_BPB  myFAT_BPB;

    /* copy of the FAT16/32 PBP */
    FATxx_BPB   myFATxx_BPB;

    unint4      RootDirSectors;

    unint4      FirstDataSector;

    unint4      CountOfClusters;

    unint4      DataSec;

    // performance enhancement for division by BPB_SecPerClus
    unint1      sector_shift_value;

    /*
     * Returns the next sector following 'currentSector'.
     *
     * params: currenCluster , the cluster the sector is contained in (could be calculated)
     *            allocate    : if the current sector is the last sector of a cluster and no cluster is following
     *                          a new cluster will be allocated if allocate is true.
     *
     */
    unint4  getNextSector(unint4 currentSector, unint4 &currentCluster, bool allocate =
                                 false);

    /*
     * Allocates a new cluster and marks it with the EOC value.
     * Returns the allocated cluster number or 0 if none found.
     */
    unint4  allocateCluster();

    /*
     * Frees a cluster chain starting at the given cluster.
     */
    bool    freeClusterChain(unint4 clusterStart);

    /*
     * Returns the fat table entry (cluster number) for a given cluster number.
     * If the entry is EOC and allocate is true a new cluster will be allocated and linked
     */
    unint4  getFATTableEntry(unint4 clusterNum, bool allocate = false);

public:
    explicit FATFileSystem(Partition* myPartition);

    ~FATFileSystem();

    static bool isFATFileSystem(Partition* part);

    /*
     * Returns the cluster number the sector is contained in
     */
    inline unint4  SectorToCluster(unint4 sectornum) {
        return (((sectornum - FirstDataSector) >> sector_shift_value) + 2);
    }

    /*
     * Returns the starting sector number of a cluster
     */
    inline unint4  ClusterToSector(unint4 clusternum) {
        // return FirstDataSector + ((clusternum -2) * myFAT_BPB.BPB_SecPerClus);
        return (FirstDataSector + ((clusternum - 2) << sector_shift_value));
    }

    ErrorT  initialize();
};



/*!
 *    Class representing a FAT Filesystem directory.
 *
 *  The contents is populated on demand to safe memory.
 */
class FATDirectory: public Directory {
friend class FATFile;
private:
    //! shortcut to my directory entry cluster number
    unint4 mycluster_num;

    //! the starting sector of this directory
    unint4 sector;

    //! Entry number of the last entry inside this FAT directory structure
    unint2 last_entry;

    //! sector containing the last entry
    unint4 last_entry_sector;

    //! pointer to the Filesystem we are located in
    FATFileSystem* myFS;

    //! has this directory been populated?
    bool    populated;

    /*!
     * Reads the file system directory and creates
     *  ORCOS directory objects for them
     * */
    void populateDirectory();

    //! Generates the windows 8.3 shortname for a given long name
    void generateShortName(unsigned char* shortname, char* name);


    ErrorT allocateEntry(char* name,
                         int DIR_Attr,
                         FAT32_DirEntry* &fsrootdir_entry,
                         unint4 &dirEntrySector,
                         unint2 &dirSectorEntry,
                         unint4 &longNameEntrySector,
                         unint2 &longNameSectorEntry);


protected:
    /*****************************************************************************
     * Method: initialize();
     *
     * @description
     *  Initializes the directory to its initial state containing no
     *  entries. Calling this method on an non empty directory will
     *  remove all entries withouc correctly freeing them! Thus, only
     *  call this on newly created directories to avoid data loss.
     *******************************************************************************/
    void initialize(unint4 parentCluster);

public:
    // FAT Directory Constructor for FAT32 filesystems
    FATDirectory(FATFileSystem* parentFileSystem, unint4 cluster_num, const char* name);

    // FAT Directory Constructor for FAT16 filesystems
    FATDirectory(FATFileSystem* parentFileSystem, unint4 cluster_num, const char* name, unint4 sector_num);

    // Destructor
    ~FATDirectory();

    //! Tries to delete the resource from the directory
    ErrorT          remove(Resource *res);

    //! gets the resource with name 'name'. Maybe null if nonexistent
    Resource*       get(const char* name, unint1 name_len = 0);

    /*
     * Returns the next directory entry.
     * Returns 0 if no such exists and allocate == false
     */
    FAT32_DirEntry* getNextEntry(bool writeBack, bool allocate);

    /*
     * Resets the directory iterator to the first entry
     */
    FAT32_DirEntry* resetToFirstEntry();

    /* writes back the current temp buffer*/
    void            synchEntries();

    //! Returns the amount of entries in this directory
    unint2          getNumEntries() {
        if (!populated)
            populateDirectory();
        return (num_entries);
    }

    //! Returns the content of this directory
    LinkedList*     getContent() {
        if (!populated)
            populateDirectory();
        return (&dir_content);
    }

    //! Returns the contents information of this directory as encoded string
    ErrorT          readBytes(char *bytes, unint4 &length);

    /*
     * Creates a new FAT File inside this directory..
     * Updates the FAT Directory entry and allocates a cluster for it.
     * Updates the FAT as well.
     */
    File*           createFile(char* name, unint4 flags);


    Directory*      createDirectory(char* name, unint4 flags);

    /*
     * Provides directory entry raw data access
     */
    ErrorT          ioctl(int request, void* args);
};


class FATFile: public File {
    friend class FATDirectory;
private:
    /* The start cluster of this file */
    unint4 clusterStart;

    /* The current sector we are in
       Base::position can be used to calculate the current byte in the sector */
    unint4 currentSector;

    // TODO: check if calculation for this can be used and is correct
    unint4 currentCluster;

    //! my file system
    FATFileSystem* myFS;

    //! the sector number for the directory entry of this file
    unint4 directory_sector;

    //! the entry number for this file inside the directory table of this file
    unint2 directory_entry_number;

    //! the long name entry for this file inside the directory table
    unint2 longname_entry_number;

    //! the sector containing the first longname entry
    unint4 longname_entry_sector;

    bool hasLongName() {
        return (longname_entry_number != -1);
    }

public:
    FATFile(char* name,
            FAT32_DirEntry* p_entry,
            FATFileSystem * fs,
            unint4 directory_sector,
            unint2 directory_entry_number,
            unint4 longname_entry_sector,
            unint2 longname_entry_number);

    ~FATFile();

    ErrorT readBytes(char *bytes, unint4 &length);

    ErrorT writeBytes(const char *bytes, unint4 length);

    ErrorT resetPosition();

    ErrorT seek(int4 seek_value);

    ErrorT onClose() {
          this->myFS->myPartition->flushCache();
          return (cOk);
    }

    /* Provides renaming functionality of fat files inside a fat directory.
     * Tries to in place rename the file. If the new name is longer than the
     * directory entries allow, new entries are allocated while moreving the old ones.*/
    ErrorT rename(char* newName);
};

#endif /* FATFILESYSTEM_HH_ */

