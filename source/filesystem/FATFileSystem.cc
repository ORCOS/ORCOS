/*
 * FATFileSystem.cc
 *
 *  Created on: 19.06.2013
 *      Author: dbaldin
 */

#include "FATFileSystem.hh"
#include "inc/memtools.hh"

#include "kernel/Kernel.hh"
#include "inc/stringtools.hh"

extern Kernel* theOS;

static unsigned char buffer[1024] __attribute__((aligned(4))) ATTR_CACHE_INHIBIT;

static unsigned char temp_buf[512] __attribute__((aligned(4))) ATTR_CACHE_INHIBIT;


//! the current entry returned by getEntry iterator
static unint2 it_currentSectorEntry = -1;

//! current cluster used by the getEntry iterator
static unint4 it_currentCluster = -1;

//! curretn sector used  by the getEntry iterator
static unint4 it_currentSector = -1;

/* Global FAT code access mutex. Used to ensure the buffers are not
 * overwritten by concurrent fat operations in other threads.*/
static Mutex* FATAccess = 0;

FATFileSystem::FATFileSystem(Partition* p_myPartition) :
        FileSystemBase(p_myPartition) {

    /* Create the mutex if not created before */
    if (FATAccess == 0)
        FATAccess = new Mutex();

    rootDir = 0;
    myPartition->setMountedFileSystem(this);

    FATAccess->acquire();

    // read out first sector
    int error = myPartition->readSectors(0, buffer, 1);
    if (error < 0) {
        FATAccess->release();
        return;
    }

    memcpy(&this->myFAT_BPB, buffer, sizeof(FAT_BS_BPB));

    // allow division by BPB_BytsPerSec to be performed by a simple shift
    switch (myFAT_BPB.BPB_SecPerClus) {
    case 1:
        sector_shift_value = 0;
        break;
    case 2:
        sector_shift_value = 1;
        break;
    case 4:
        sector_shift_value = 2;
        break;
    case 8:
        sector_shift_value = 3;
        break;
    case 16:
        sector_shift_value = 4;
        break;
    case 32:
        sector_shift_value = 5;
        break;
    case 64:
        sector_shift_value = 6;
        break;
    case 128:
        sector_shift_value = 7;
        break;
    }

    if ((myFAT_BPB.BS_jmpBoot[0] == 0xeb) || (myFAT_BPB.BS_jmpBoot[0] == 0xe9))
    {

        if (myFAT_BPB.BPB_BytsPerSec != 512)
        {
            LOG(ARCH, ERROR,"FATFileSystem: Invalid Sector Size %d.",myFAT_BPB.BPB_BytsPerSec);

        }
        else if (myFAT_BPB.BPB_BytsPerSec <= myPartition->getSectorSize())
        {

            // right now we trust the rest of the table and do checks on demand
            FAT32_BPB* myFAT32BPB = (FAT32_BPB*) &buffer[36];

            RootDirSectors =
                    (((myFAT_BPB.BPB_RootEntCnt * 32)
                            + (myFAT_BPB.BPB_BytsPerSec - 1))
                            / myFAT_BPB.BPB_BytsPerSec);

            // getting the first sector of a cluster N:
            // FirstSectorofCluster = ((N – 2) * BPB_SecPerClus) + FirstDataSector;

            unint4 FATSz;
            if (myFAT_BPB.BPB_FATSz16 != 0)
                FATSz = myFAT_BPB.BPB_FATSz16;
            else
                FATSz = myFAT32BPB->BPB_FATSz32;

            unint4 TotSec;
            if (myFAT_BPB.BPB_TotSec16 != 0)
                TotSec = myFAT_BPB.BPB_TotSec16;
            else
                TotSec = myFAT_BPB.BPB_TotSec32;

            FirstDataSector = myFAT_BPB.BPB_RsvdSecCnt
                    + (myFAT_BPB.BPB_NumFATs * FATSz) + RootDirSectors;
            LOG(ARCH, INFO, "FATFileSystem: First FAT Data Sector: %d",FirstDataSector);

            DataSec = TotSec - (myFAT_BPB.BPB_RsvdSecCnt + (myFAT_BPB.BPB_NumFATs * FATSz) + RootDirSectors);
            CountOfClusters = DataSec / myFAT_BPB.BPB_SecPerClus;
            LOG(ARCH, INFO, "FATFileSystem: Clusters: %d, DataSectors: %d",CountOfClusters,DataSec);

            if (CountOfClusters < 4085)
            {
                this->myFatType = FAT12;
                LOG(ARCH, WARN, "FATFileSystem: FAT12 not supported.. not mounting FS");
            }
            else if (CountOfClusters < 65525)
            {
                this->myFatType = FAT16;
                this->isValid = true;
                memcpy(&myFATxx_BPB, &buffer[36], sizeof(FAT16_BPB));
                LOG(ARCH, INFO, "FATFileSystem: found valid FAT16 with %d sectors",myFAT_BPB.BPB_TotSec16);
            }
            else
            {
                this->myFatType = FAT32;
                this->isValid = true;
                memcpy(&myFATxx_BPB, &buffer[36], sizeof(FAT32_BPB));
                LOG(ARCH, INFO,"FATFileSystem: found valid FAT32 with %d sectors",myFAT_BPB.BPB_TotSec32);
            }

        }
        else
        {
            LOG(ARCH, ERROR, "FATFileSystem: invalid Sector Size: %d",myFAT_BPB.BPB_BytsPerSec);
        }
    }
    else
    {
        LOG(ARCH, ERROR, "FATFileSystem: invalid Filesystem. Invalid BS_jmpBoot field");
    }

    FATAccess->release();

}

unint4 FATFileSystem::allocateCluster() {
    LOG(ARCH, DEBUG, "FATFileSystem: Trying to allocate new cluster");

    // TODO: add FAT 16 allocate cluster support
    if (this->myFatType == FAT16)
        return (0xFFFF);
    else
    {
        unint4 fsinfo_lba = myFATxx_BPB.myFAT32_BPB.BPB_FSInfo * (myFAT_BPB.BPB_BytsPerSec / this->myPartition->getSectorSize());
        int error = myPartition->readSectors(fsinfo_lba, temp_buf, 1);
        if (error < 0)
            return (cError );

        FAT32_FSInfo *fsinfo = (FAT32_FSInfo *) &temp_buf[0];

        unint4 cluster_start = fsinfo->FSI_Nxt_Free;
        if (cluster_start == 0xFFFFFFFF)
            cluster_start = 2;

        // walk through the table and find a free entry
        unint4 FATEntryValue        = EOC;
        unint4 currentCluster       = cluster_start;
        unint4 currentClusterAddr   = cluster_start * 4;
        unint4 fatsecnum;
        unint4 fatentoffset;

        // iterate over the whole FAT table if needed
        while (FATEntryValue != 0 && currentCluster < CountOfClusters)
        {
            // get table entry offset in partition sector
            // unint4 fatentoffset = addr % myFAT_BPB.BPB_BytsPerSec;
            fatentoffset = currentClusterAddr & (myFAT_BPB.BPB_BytsPerSec - 1);

            // get the partition sector containing the table entry
            fatsecnum = myFAT_BPB.BPB_RsvdSecCnt + (currentClusterAddr / myFAT_BPB.BPB_BytsPerSec);

            // read the sector
            error = myPartition->readSectors(fatsecnum, temp_buf, 1);
            if (error < 0)
                return (EOC);

            /* read all FAT entries found inside this sector if needed */
            while (FATEntryValue != 0 && fatentoffset < myFAT_BPB.BPB_BytsPerSec)
            {
                FATEntryValue = (*((unint4*) __builtin_assume_aligned(&temp_buf[fatentoffset],4))) & 0x0FFFFFFF;
                LOG(ARCH, TRACE, "FATFileSystem: FAT Entry: %u = %x",fatentoffset/4,FATEntryValue);

                /* check next entry if this one not free */
                fatentoffset        += 4;
                currentClusterAddr  += 4;  /* every entry stands for one cluster.. */
                currentCluster++;
            }
        }

        if (FATEntryValue == 0)
        {
            // found free entry
            fatentoffset -= 4;
            currentCluster--;
            // mark as end of chain
            (*((unint4*) __builtin_assume_aligned(&temp_buf[fatentoffset],4))) = 0xffffff8;
            error = myPartition->writeSectors(fatsecnum, temp_buf, 1);
            if (error < 0)
                return (EOC);

            // update FS_INFO
            error = myPartition->readSectors(fsinfo_lba, temp_buf, 1);
            if (error < 0)
                return (EOC);

            fsinfo = (FAT32_FSInfo *) &temp_buf[0];
            fsinfo->FSI_FreeCount--;
            fsinfo->FSI_Nxt_Free = currentCluster + 1;

            error = myPartition->writeSectors(fsinfo_lba, temp_buf, 1);
            if (error < 0)
                return (EOC);

            return (currentCluster);
        }

        return (EOC);
    }

}

bool    FATFileSystem::freeClusterChain(unint4 clusterStart) {

    if (this->myFatType == FAT16)
          return (false);
    else
    {
        unint4  currentClusterAddr = clusterStart * 4;
        unint4  fatsecnum;
        unint4  fatentoffset;
        int     error;
        unint4 FATEntryValue        = 0;

        // get table entry offset in partition sector
        // unint4 fatentoffset = addr % myFAT_BPB.BPB_BytsPerSec;
        fatentoffset = currentClusterAddr & (myFAT_BPB.BPB_BytsPerSec - 1);

        // get the partition sector containing the table entry
        fatsecnum = myFAT_BPB.BPB_RsvdSecCnt + (currentClusterAddr / myFAT_BPB.BPB_BytsPerSec);

        // read the sector
        error = myPartition->readSectors(fatsecnum, temp_buf, 1);
        if (error < 0)
            return (false);

         // read all FAT entries found inside this sector if needed
         while (FATEntryValue != 0x0ffffff8 && fatentoffset < myFAT_BPB.BPB_BytsPerSec)
         {
             FATEntryValue = (*((unint4*) __builtin_assume_aligned(&temp_buf[fatentoffset],4))) & 0x0FFFFFFF;
             if (FATEntryValue == 0) {
                 LOG(FILESYSTEM, WARN, "FATFileSystem: Cluster Chain Entry already free. Stopping.");
                 break;
             }
             LOG(FILESYSTEM, DEBUG, "FATFileSystem: free FAT Entry: %u Cluster: %u, Next Cluster: %x, ",fatentoffset/4,currentClusterAddr/4,FATEntryValue);

             (*((unint4*) __builtin_assume_aligned(&temp_buf[fatentoffset],4))) = 0;

             currentClusterAddr = FATEntryValue * 4;
             fatentoffset = currentClusterAddr & (myFAT_BPB.BPB_BytsPerSec - 1);

             // get the partition sector containing the table entry
             unint4 nextfatsecnum = myFAT_BPB.BPB_RsvdSecCnt + (currentClusterAddr / myFAT_BPB.BPB_BytsPerSec);
             if (nextfatsecnum != fatsecnum) {
                 /* write back sector and get next one!*/
                 error = myPartition->writeSectors(fatsecnum, temp_buf, 1);
                 fatsecnum = nextfatsecnum;
                 error = myPartition->readSectors(fatsecnum, temp_buf, 1);
             }

         }

         /* done.. write last sector back */
         error = myPartition->writeSectors(fatsecnum, temp_buf, 1);

        return (true);
    }

}


unint4 FATFileSystem::getFATTableEntry(unint4 clusterNum, bool allocate) {
    LOG(FILESYSTEM, DEBUG, "FATFileSystem:getFATTableEntry: %u",clusterNum);

    unint4 addr;
    if (this->myFatType == FAT16)
        addr = clusterNum * 2;
    else
        addr = clusterNum * 4;

    // get table entry offset in partition sector
    // unint4 fatentoffset = addr % myFAT_BPB.BPB_BytsPerSec;
    unint4 fatentoffset = addr & (myFAT_BPB.BPB_BytsPerSec - 1);

    // get the partition sector containing the table entry
    unint4 fatsecnum = myFAT_BPB.BPB_RsvdSecCnt + (addr / myFAT_BPB.BPB_BytsPerSec);

    // read the sector
    int error = myPartition->readSectors(fatsecnum, buffer, 1);
    if (error < 0)
        return (EOC);

    // now get the table entry
    // this is 4 bytes aligned.. however compiler does not know this
    // thus this may produce a warning
    if (myFatType == FAT16)
    {
        unint2 nextCluster = (*((unint2*) __builtin_assume_aligned(&buffer[fatentoffset],2)));

        if ((nextCluster >= 0xFFF8) && allocate)
        {
            nextCluster = (unint2) allocateCluster();

            int error = myPartition->readSectors(fatsecnum, buffer, 1);
            /* set FAT entry */
            (*((unint2*) __builtin_assume_aligned(&buffer[fatentoffset],2))) = nextCluster;
            error = myPartition->writeSectors(fatsecnum, buffer, 1);
            if (error < 0)
                return (EOC);
        }

        return (nextCluster);
    }
    else
    {
        unint4 nextCluster = (*((unint4*) __builtin_assume_aligned(&buffer[fatentoffset],4))) & 0x0FFFFFFF;
        LOG(FILESYSTEM, DEBUG, "FATFileSystem:getFATTableEntry: entry: %u",nextCluster);

        if ((nextCluster >= 0xFFFFFF8) && allocate)
        {
            nextCluster = allocateCluster();
            LOG(FILESYSTEM, TRACE, "FATFileSystem: allocated cluster: %u",nextCluster);

            int error = myPartition->readSectors(fatsecnum, buffer, 1);
            /* set FAT entry */
            (*((unint4*) __builtin_assume_aligned(&buffer[fatentoffset],4))) = nextCluster;
            /* write back the part of the FAT table */
            error = myPartition->writeSectors(fatsecnum, buffer, 1);
            if (error < 0)
                return (EOC);
        }
        return (nextCluster);
    }

}

unint4 FATFileSystem::getNextSector(unint4 currentSector, unint4 &currentCluster, bool allocate) {

    // check if next sector would lie inside another cluster
    // if so follow cluster chain
    // if ( (currentSector / myFAT_BPB.BPB_SecPerClus) != ((currentSector+1) / myFAT_BPB.BPB_SecPerClus)) {
    if ((currentSector >> sector_shift_value) != ((currentSector + 1) >> sector_shift_value))
    {
        // next sector is in another clusters
        // lookup the FAT table to find the successor cluster
        unint4 nextClusterNum = getFATTableEntry(currentCluster, allocate);

        currentCluster = EOC;
        if (this->myFatType == FAT16)
        {
            if (nextClusterNum >= 0xFFF8)  return (EOC);
        }
        else if (nextClusterNum >= 0xFFFFFF8) return (EOC);

        currentCluster = nextClusterNum;
        return (ClusterToSector(nextClusterNum));
    }
    else
    {
        return (currentSector + 1);
    }
}

static int FAT_SHORTNAME_STRIP(char* &name, unint1 startpos) __attribute__((noinline));

static int FAT_SHORTNAME_STRIP(char* &name, unint1 startpos) {

    for (int i = startpos; i > 0; i--)
    {
        if (name[i] == ' ')
            name[i] = 0;
        else
            return (i);
    }

    /* should not happen as this should be a filename violation
     however, be safe here */
    return (0);

}

FATFileSystem::~FATFileSystem() {
    // get orcos mount directory
    Directory* mntdir = theOS->getFileManager()->getDirectory("/mnt/");

    if (mntdir != 0 && rootDir != 0)
    {
        // remove root directory to system
        mntdir->remove(rootDir);
        delete rootDir;
    }

}

ErrorT FATFileSystem::initialize() {

    if (this->isValid)
    {

        /* get orcos mount directory */
        Directory* mntdir = theOS->getFileManager()->getDirectory("/mnt/");

        if (myFatType == FAT32)
        {
            /* get FSInfo */
            unint4 fsinfo_lba = myFATxx_BPB.myFAT32_BPB.BPB_FSInfo  * (myFAT_BPB.BPB_BytsPerSec  / this->myPartition->getSectorSize());
            int error = myPartition->readSectors(fsinfo_lba, buffer, 1);
            if (error < 0)
                return (cError );

            FAT32_FSInfo *fsinfo = (FAT32_FSInfo *) &buffer[0];
            if (fsinfo->FSI_LeadSig != 0x41615252)
            {
                LOG(ARCH, WARN, "FATFileSystem: FS_INFO Lead Sig Validation failed: %x != 0x41615252.",fsinfo->FSI_LeadSig);
            }
            if (fsinfo->FSI_StrucSig != 0x61417272)
            {
                LOG(ARCH, WARN, "FATFileSystem: FS_INFO Struc Sig Validation failed: %x != 0x61417272",fsinfo->FSI_StrucSig);
            }

            LOG(ARCH, INFO, "FATFileSystem: First Cluster Count: %x",fsinfo->FSI_FreeCount);

            if (mntdir == 0)
            {
                LOG(ARCH, ERROR, "FATFileSystem: no mount directory '/mnt/' found. Not mounting");
                return (cError );
            }

            char *name = (char*) theOS->getMemoryManager()->alloc(12);
            memcpy(name, myFATxx_BPB.myFAT32_BPB.BS_VolLab, 11);
            name[11] = 0;
            FAT_SHORTNAME_STRIP(name, 10);

            if (!(strcmp2("NO NAME", myFATxx_BPB.myFAT32_BPB.BS_VolLab, 7) != 0))
            {
                /* Read the volume label from the root directory */
                int error = myPartition->readSectors(ClusterToSector(myFATxx_BPB.myFAT32_BPB.BPB_RootClus), buffer, 1);
                if (error < 0) return (cError );
                FAT32_DirEntry *fsrootdir_entry = (FAT32_DirEntry *) &buffer[0];
                if (fsrootdir_entry->DIR_Attr & ATTR_VOLUME_ID)
                {
                    /* take this entry name */
                    memcpy(name, fsrootdir_entry->DIR_Name, 11);
                    FAT_SHORTNAME_STRIP(name, 10);
                }
            }

            LOG(ARCH, INFO, "FATFileSystem: Volume: '%s' ID: %x",name,myFATxx_BPB.myFAT32_BPB.BS_VolID);

            /* add root directory to system */
            rootDir = new FATDirectory(this, myFATxx_BPB.myFAT32_BPB.BPB_RootClus, name);
            mntdir->add(rootDir);
            LOG(ARCH, INFO,"FATFileSystem: FAT Filesystem mounted to '/mnt/%s/'.",name);

            /* go on and mount this fs */
            return (cOk );
        }
        else
        {
            // FAT 16

            char *name = (char*) theOS->getMemoryManager()->alloc(12);
            name[11] = 0;
            memcpy(name, myFATxx_BPB.myFAT16_BPB.BS_VolLab, 11);

            FAT_SHORTNAME_STRIP(name, 10);

            LOG(FILESYSTEM, INFO, "FATFileSystem: Volume: '%s' ID: %x",name,myFATxx_BPB.myFAT16_BPB.BS_VolID);

            unint4 FirstRootDirSecNum = myFAT_BPB.BPB_RsvdSecCnt + (myFAT_BPB.BPB_NumFATs * myFAT_BPB.BPB_FATSz16);
            rootDir = new FATDirectory(this, 0, name, FirstRootDirSecNum);
            mntdir->add(rootDir);

            LOG(FILESYSTEM, INFO, "FATFileSystem: FAT Filesystem mounted to '/mnt/%s/'.",name);

            // go on and mount this fs
            return (cOk );

        }

    }

    return (cError );

}

FATDirectory::FATDirectory(FATFileSystem* parentFileSystem, unint4 cluster_num, const char* p_name) :
        Directory(p_name) {

    this->myFS = parentFileSystem;
    this->mycluster_num = cluster_num;
    this->populated = false;
    this->sector = myFS->ClusterToSector(mycluster_num);
    last_entry = 0;
}

FATDirectory::FATDirectory(FATFileSystem* parentFileSystem, unint4 cluster_num, const char* p_name, unint4 u_sector) :
        Directory(p_name) {

    this->myFS = parentFileSystem;
    this->mycluster_num = cluster_num;
    this->populated = false;
    this->sector = u_sector;
    last_entry = 0;
}

ErrorT FATDirectory::readBytes(char* bytes, unint4& length) {

    if (!populated)
        this->populateDirectory();
    return (Directory::readBytes(bytes, length));
}

//-----------------------------------------------------------------------------
//	ChkSum()
//	Returns an unsigned byte checksum computed on an unsigned byte
//	array.  The array must be 11 bytes long and is assumed to contain
//	a name stored in the format of a MS-DOS directory entry.
//	Passed:	 pFcbName    Pointer to an unsigned byte array assumed to be
//                          11 bytes long.
//	Returns: Sum         An 8-bit unsigned checksum of the array pointed
//                           to by pFcbName.
//------------------------------------------------------------------------------
unsigned char ChkSum(unsigned char *pFcbName) {
    short FcbNameLen;
    unsigned char Sum;

    Sum = 0;
    for (FcbNameLen = 11; FcbNameLen != 0; FcbNameLen--)
    {
        // NOTE: The operation is an unsigned char rotate right
        Sum = ((Sum & 1) ? 0x80 : 0) + (Sum >> 1) + *pFcbName++;
    }
    return (Sum);
}

void FATDirectory::populateDirectory() {

    unint4 currentCluster = mycluster_num;
    // cluster number to sector number conversion
    // get root sector number
    unint4 sector_number = this->sector;

    last_entry = 0;

    int error = myFS->myPartition->readSectors(sector_number, buffer, 1);
    if (error < 0)
        return;

    FAT32_DirEntry *fsrootdir_entry = (FAT32_DirEntry *) &buffer[0];
    unint4 entries_per_sector = myFS->myPartition->getSectorSize() / 32;

    unint2 currentEntry     = 0;  /* current directory entry number inside the sector 0 .. */
    char*  longname         = 0;  /* the current extracted long name  */
    unint1 chksum           = 0;  /* the current chksum value */
    unint2 longNameEntry    = -1; /* the current long name entry number inside the sector */
    unint4 longNameSector   = -1;

    while (fsrootdir_entry->DIR_Name[0] != 0)
    {
        FAT32_LongDirEntry* fslongentry = (FAT32_LongDirEntry*) fsrootdir_entry;

        // check if this is not a free entry
        if (fsrootdir_entry->DIR_Name[0] != 0xE5)
        {

            if ((fslongentry->LDIR_Attr & ATTR_LONG_NAME_MASK) == ATTR_LONG_NAME)
            {
                /*   Found an active long name sub-component.   */
                unint1 n = fslongentry->LDIR_Ord & 0x3F;
                if ((fslongentry->LDIR_Ord & 0x40) == 0x40)
                {
                    longname = (char*) theOS->getMemoryManager()->alloc((13 * n) + 1);
                    longname[13 * n]    = 0;
                    chksum              = fslongentry->LDIR_Chksum;
                    longNameEntry       = currentEntry;
                    longNameSector      = sector_number;
                }

                // drop long name on chsum mismatch (treat as orphan)
                if (fslongentry->LDIR_Chksum != chksum)
                {
                    LOG(ARCH, DEBUG,"FAT Longname Entry treated as orphan. Checksum Mismatch:  %x != %x",fslongentry->LDIR_Chksum, chksum);
                    longname = 0;
                    longNameEntry = -1;
                }

                if (longname != 0 && n > 0)
                {

                    // now copy 16 bit unicode to longname
                    // unicode is in little-endian byte order
                    char* pos = &longname[13 * (n - 1)];
                    pos[0] = fslongentry->LDIR_Name[0];
                    pos[1] = fslongentry->LDIR_Name[2];
                    pos[2] = fslongentry->LDIR_Name[4];
                    pos[3] = fslongentry->LDIR_Name[6];
                    pos[4] = fslongentry->LDIR_Name[8];
                    pos[5] = fslongentry->LDIR_Name2[0];
                    pos[6] = fslongentry->LDIR_Name2[2];
                    pos[7] = fslongentry->LDIR_Name2[4];
                    pos[8] = fslongentry->LDIR_Name2[6];
                    pos[9] = fslongentry->LDIR_Name2[8];
                    pos[10] = fslongentry->LDIR_Name2[10];
                    pos[11] = fslongentry->LDIR_Name3[0];
                    pos[12] = fslongentry->LDIR_Name3[2];
                }

            }
            else
            {
                // we have some short entry
                if (fsrootdir_entry->DIR_Attr != ATTR_VOLUME_ID)
                {
                    unint4 clusterid = (fsrootdir_entry->DIR_FstClusHI << 16)  | fsrootdir_entry->DIR_FstClusLO;
                    char* p_name;

                    if   ((longname != 0) && (ChkSum((unsigned char*) fsrootdir_entry->DIR_Name) != chksum))
                    {
                        LOG(FILESYSTEM, DEBUG, "FAT longname treated as orphan. Checksum Mismatch:  %x != %x",ChkSum((unsigned char*) fsrootdir_entry->DIR_Name), chksum);
                        delete longname;
                        longname = 0;
                    }

                    if (longname == 0)
                    {
                        p_name = (char*) theOS->getMemoryManager()->alloc(16);
                        p_name[11] = 0;
                        memcpy(p_name, fsrootdir_entry->DIR_Name, 11);
                    } else
                    {
                        p_name = longname;
                        FAT_SHORTNAME_STRIP(p_name, (unint1) (strlen(p_name) - 1));
                    }

                    if ((fsrootdir_entry->DIR_Attr & ATTR_DIRECTORY) != 0)
                    {
                        /* new directory entry */
                        if (longname == 0) FAT_SHORTNAME_STRIP(p_name, 9);
                        FATDirectory *fatDir = new FATDirectory(this->myFS, clusterid, p_name);
                        this->add(fatDir);
                    }
                    else
                    {
                        /* add the file to this directory */
                        if (longname == 0)
                        {
                            int lastpos = FAT_SHORTNAME_STRIP(p_name, 7);
                            lastpos++;

                            // file extension handling for fat names
                            char ext[3];
                            ext[0] = p_name[8];
                            ext[1] = p_name[9];
                            ext[2] = p_name[10];
                            if (ext[0] != ' ')
                            {
                                p_name[lastpos] = '.';
                                p_name[lastpos + 1] = ext[0];
                                p_name[lastpos + 2] = ext[1];
                                p_name[lastpos + 3] = ext[2];
                                p_name[lastpos + 4] = 0;
                            }
                            else {
                                p_name[lastpos] = 0;
                            }
                        }

                        FATFile *file = new FATFile(p_name,
                                                    fsrootdir_entry,
                                                    myFS,
                                                    sector_number,
                                                    currentEntry,
                                                    longNameSector,
                                                    longNameEntry);
                        this->add(file);

                    }

                    longname = 0;
                    longNameEntry = -1;

                    LOG(FILESYSTEM, TRACE, "'%s'\t%x\t%d\t%d",p_name,fsrootdir_entry->DIR_Attr,clusterid,fsrootdir_entry->DIR_FileSize);

                }  // if not ATTR_VOLUME_ID
            }  // if not long name entry
        }  // if valid entry

        fsrootdir_entry++;
        currentEntry++;
        if (currentEntry >= entries_per_sector)
        {
            // load next sector
            sector_number = this->myFS->getNextSector(sector_number, currentCluster);
            if (sector_number == EOC)
                break;

            error = myFS->myPartition->readSectors(sector_number, buffer, 1);
            if (error < 0)
                return;

            currentEntry    = 0;
            fsrootdir_entry = (FAT32_DirEntry *) &buffer[0];
        }

    }
    last_entry        = currentEntry;
    last_entry_sector = sector_number;

    this->populated = true;

}

Resource* FATDirectory::get(const char* p_name, unint1 name_len) {
    // if this directory is already populated forward to base class
    if (!populated)
        populateDirectory();

    /*TODO: we might return a single resource without populating the whole directory
     just lookup the entries on the block devices partition and create a resource for it.
     This would be much more memory efficient, however much slower
     */

    return (Directory::get(p_name, name_len));

}

File* FATDirectory::createFile(char* p_name, unint4 flags) {

    /* avoid creating identical file names*/
    if (get(p_name,strlen(p_name))) {
        LOG(FILESYSTEM,ERROR,"createFile failed for '%s': file exists.",p_name);
        return (0);
    }

    /* get exclusive access to the FAT buffers */
    FATAccess->acquire();

    unint4 fileEntrySector      = -1;
    unint2 fileSectorEntry      = -1;

    unint4 longNameEntrySector  = -1;
    unint2 longNameSectorEntry  = -1;

    unint4 namelen = strlen(p_name);
    unint1 long_entries = (unint1) ((namelen / 13) + 1);

    /* we stupidly put new files at the end of the file...
     * not very efficient .. however simple */
    FAT32_DirEntry *fsrootdir_entry  = resetToFirstEntry();
    while (fsrootdir_entry != 0 && fsrootdir_entry->DIR_Name[0] != 0)
    {
        fsrootdir_entry = getNextEntry(false,false);
    }
    if (fsrootdir_entry == 0)
        fsrootdir_entry = getNextEntry(false,true);

    /* we will overwrite the trailing entry with the new long name entries
     * and append a new 'end of entries' entry at the very end */

    /* end found. get the new cluster for the file */
    unint4 fileCluster = myFS->allocateCluster();
    if (fileCluster == EOC)
    {
        FATAccess->release();
        return (0);
    }
    LOG(FILESYSTEM, INFO,"FATDirectory::createFile() new File allocated to Cluster %d",fileCluster);

    /* create entries at the end */
    unsigned char shortname[11];
    /* generate the DOS shortname */
    generateShortName(shortname, p_name);
    char temp_name[13];

    FAT32_LongDirEntry* fslongentry;
    longNameEntrySector = it_currentSector; /* remember the longname sector */
    longNameSectorEntry = it_currentSectorEntry;

    /* create long entries first */
    for (int i = 0; i < long_entries; i++)
    {
        fslongentry = (FAT32_LongDirEntry*) fsrootdir_entry;
        memset(fslongentry, 0, sizeof(FAT32_LongDirEntry));

        fslongentry->LDIR_Attr      = ATTR_LONG_NAME;
        fslongentry->LDIR_Chksum    = ChkSum(shortname);
        fslongentry->LDIR_FstClusLO = 0;
        fslongentry->LDIR_Type      = 0;
        if (i == 0)
            fslongentry->LDIR_Ord = (unint1) ((long_entries - i) | 0x40);
        else
            fslongentry->LDIR_Ord = (unint1) (long_entries - i);

        /* now set name fields */
        char* pos = &p_name[13 * ((long_entries - i) - 1)];
        int len = 13;
        if (i == 0)
        {
            len = namelen - ((long_entries - 1) * 13);
            if (len < 13)
                len++;  /* be sure trailing 0 is copied */
        }

        /* be sure unused name fields are set to 0xffff */
        memset(temp_name, 0xff, 13);
        memcpy(temp_name, pos, len);

        /* set unicode name parts */
        ascii2unicode(&temp_name[0] ,(char*) &fslongentry->LDIR_Name[0] ,5);
        ascii2unicode(&temp_name[5] ,(char*) &fslongentry->LDIR_Name2[0],6);
        ascii2unicode(&temp_name[11],(char*) &fslongentry->LDIR_Name3[0],2);

        /* get next entry .. check if last entry in sector */
        /* get (allocate) next sector if last entry */
       fsrootdir_entry = getNextEntry(true,true);
    }

    fileSectorEntry = it_currentSectorEntry;
    fileEntrySector = it_currentSector;

    /* write last (short) entry containing file information */
    memset(fsrootdir_entry, 0, sizeof(FAT32_DirEntry));
    memcpy(fsrootdir_entry->DIR_Name, shortname, 11);
    fsrootdir_entry->DIR_FstClusHI  = (unint2) (fileCluster >> 16);
    fsrootdir_entry->DIR_FstClusLO  = fileCluster & 0xffff;
    fsrootdir_entry->DIR_Attr       = 0x20;  // mark as backup

    char* namepcpy = (char*) theOS->getMemoryManager()->alloc(namelen + 1);
    memcpy(namepcpy, p_name, namelen + 1);

    FATFile *file = new FATFile(namepcpy,
                                fsrootdir_entry,
                                myFS,
                                fileEntrySector,
                                fileSectorEntry,
                                longNameEntrySector,
                                longNameSectorEntry);
    this->add(file);

    /* set new end, allocate cluster if needed */
    fsrootdir_entry = getNextEntry(true,true);
    fsrootdir_entry->DIR_Name[0] = 0;
    /* final synch of the current buffer entries */
    synchEntries();

    /* buffers are now free to be used by another thread */
    FATAccess->release();

    return (file);
}


ErrorT FATFile::rename(char* newName) {
    // TODO: update directory entries... this may get complicated

    /* finally call base method */
    File::rename(newName);
    return (cOk);

}

ErrorT FATFile::seek(int4 seek_value) {
    // TODO: support seeking in FAT files
    return (cError );
}

void FATDirectory::generateShortName(unsigned char* shortname, char* p_name) {

    int dotpos = -1;
    int len = 0;

    for (int i = 0; i < 255; i++)
    {
        if (p_name[i] == '.')
            dotpos = i;
        if (p_name[i] == 0)
        {
            len = i;
            break;
        }
    }

    // TODO: create ~1..n based names..
    // windows only recognizes the long names if we use the ~ based
    // short name ... stupid windows .. linux works correctly
    // fatgen103.doc (FAT Standard) explicitly says the numerical tail is optional
    // however, microsoft does not treat it as optional??
    // fill everything with blanks
    memset(shortname, ' ', 11);

    if (dotpos == -1)
    {
        if (len > 10)
            len = 8;
        memcpy(shortname, p_name, len);
    }
    else
    {
        // get extension length
        unint1 ext_len = (unint1) ((len - dotpos) - 1);
        if (ext_len > 3)
            ext_len = 3;
        // copy extension, shrink to 3 chars
        memcpy(&shortname[8], &p_name[dotpos + 1], ext_len);

        // copy name before extension
        // shrink name
        if (dotpos > 10)
            dotpos = 8;
        memcpy(shortname, p_name, dotpos);

    }

    // FATtiyfy the name
    // not completely correct... trailing spaces should be kept
    // intermediate spaces should be replaced...
    for (int i = 0; i < 11; i++)
    {
        unint1 val = shortname[i];
        if (val < 32)
            val = '_';
        if (val > 122)
            val = '_';
        if (val > 96)
            val = (unint1) (val - 32);
        shortname[i] = val;
    }

}

FATDirectory::~FATDirectory() {

}

FAT32_DirEntry* FATDirectory::getNextEntry(bool writeBack, bool allocate) {

  if (it_currentSectorEntry == -1) {
      return (resetToFirstEntry());
  }

 unint4 entries_per_sector  = myFS->myPartition->getSectorSize() / 32;
 FAT32_DirEntry* entries    = (FAT32_DirEntry*) buffer;

 /* no more entries?*/
 if (it_currentSectorEntry == last_entry && it_currentSector == last_entry_sector) {
     if (allocate != true)
         return (0);

     it_currentSectorEntry++;

     /* allocate a new entry*/
     /* end of sector reached? */
     if (it_currentSectorEntry >= entries_per_sector)
     {
         /* first write back the current sector */
         if (writeBack) {
             if (isError(myFS->myPartition->writeSectors(it_currentSector, buffer, 1)))
                 return (0);
         }

         /* load next sector, allocate if cluster ends */
         it_currentSector =  this->myFS->getNextSector(it_currentSector, it_currentCluster);
         /* EOC can happen, allocate next chain cluster then */
         if (it_currentSector == EOC)
         {
             it_currentSector = this->myFS->getNextSector(it_currentSector, it_currentCluster, true);
             if (it_currentSector == EOC)
             {
                 LOG(FILESYSTEM, WARN, "FATDirectory::createFile() No more memory.. ");
                 return (0);
             }
             /* clear directory table */
             memset(buffer, 0, 512);
         }
         else
         {
             if (isError(myFS->myPartition->readSectors(it_currentSector, buffer, 1)))
                 return (0);
         }

         last_entry_sector = it_currentSector;
         it_currentSectorEntry = 0;
     }

     last_entry = it_currentSectorEntry;
     /* return the next entry*/
     return (&entries[it_currentSectorEntry]);
 }

 it_currentSectorEntry++;
 if (it_currentSectorEntry >= entries_per_sector)
 {
     // first write back the sector
     if (writeBack) {
         if (isError(myFS->myPartition->writeSectors(it_currentSector, buffer, 1)))
             return (0);
     }

     // load next sector, allocate if cluster ends!
     it_currentSector =  this->myFS->getNextSector(it_currentSector, it_currentCluster);
     // EOC must not happen
     if (it_currentSector == EOC)
     {
         LOG(FILESYSTEM,ERROR,"EOC encountered before end of directory.. Directory table corrupt.");
         return (0);
     }
     else
     {
         if (isError(myFS->myPartition->readSectors(it_currentSector, buffer, 1)))
             return (0);
     }
     it_currentSectorEntry = 0;
 }

 return (&entries[it_currentSectorEntry]);

}

void FATDirectory::synchEntries() {
   myFS->myPartition->writeSectors(it_currentSector, buffer, 1);
}

FAT32_DirEntry* FATDirectory::resetToFirstEntry()
{
    it_currentSectorEntry = 0;
    it_currentSector = sector;
    it_currentCluster = myFS->SectorToCluster(sector);
    myFS->myPartition->readSectors(sector, buffer, 1);
    FAT32_DirEntry* entries = (FAT32_DirEntry*) buffer;
    return (&entries[0]);
}

ErrorT FATDirectory::remove(Resource *res) {

    if (!(res->getType() & (cFile | cDirectory)))
        return (cWrongResourceType );

    FATAccess->acquire();
    FATFile* file = (FATFile*) res;

    it_currentSector      = file->directory_sector;
    it_currentSectorEntry = file->directory_entry_number;

    if (file->longname_entry_number != -1) {
        /* start removing the entries at longname entries */
        it_currentSector      = file->longname_entry_sector;
        it_currentSectorEntry = file->longname_entry_number;
    }

    it_currentCluster     = myFS->SectorToCluster(it_currentSector);

    /* fill up the iterator buffer at the current sector */
    int error =  myFS->myPartition->readSectors(it_currentSector, buffer, 1);
    if (error < 0)
    {
        FATAccess->release();
        return (error);
    }

    FAT32_DirEntry *fsrootdir_entry = &((FAT32_DirEntry *) buffer)[it_currentSectorEntry];

    /* iterate over all longname entries and the final short entry and mark them as free */
    do {
        memset(fsrootdir_entry,0,sizeof(FAT32_DirEntry));
        fsrootdir_entry->DIR_Name[0] = 0xE5;
        fsrootdir_entry = getNextEntry(true,false);
    } while (it_currentSector != file->directory_sector && it_currentSectorEntry != file->directory_entry_number);

    memset(fsrootdir_entry,0,sizeof(FAT32_DirEntry));
    fsrootdir_entry->DIR_Name[0] = 0xE5;

    /* check if the last entry was also the last entry inside the dir table*/
    FAT32_DirEntry *nextEntry = getNextEntry(true,false);
    if (!nextEntry) {
        /* reached the end */
        fsrootdir_entry->DIR_Name[0] = 0x0;
    }

    /* be sure last entry is written back as well */
    synchEntries();

    /* now free up the FAT cluster chain entries used by this file */
    if (!myFS->freeClusterChain(file->clusterStart)) {
        LOG(FILESYSTEM, ERROR, "FATFile::freeClusterChain failed");
    }

    FATAccess->release();

    /* finally remove internal dir object */
    return (Directory::remove(res));
}


ErrorT FATDirectory::ioctl(int request, void* args) {

    if (request == 1) {
        buffer_t* buf = (buffer_t*) args;

        FATAccess->acquire();

        if (!populated)
            populateDirectory();

        FAT32_DirEntry *entry = resetToFirstEntry();
        unint4 len = 0;
        if (buf->len == 0) return (cError);

        while (entry && ((len + 79) < buf->len)) {
#if 0
            for (int i = 0; i < 32; i++) {
                sprintf(&buf->buffer[len+1],"%x", ((char*) entry)[i]);
            }
            len += 32;
#endif
            FAT32_LongDirEntry* fslongentry = (FAT32_LongDirEntry*) entry;

            if ((fslongentry->LDIR_Attr & ATTR_LONG_NAME_MASK) == ATTR_LONG_NAME)
            {
                memset(&buf->buffer[len],' ',13);
                buf->buffer[len+0] = fslongentry->LDIR_Name[0];
                buf->buffer[len+1] = fslongentry->LDIR_Name[2];
                buf->buffer[len+2] = fslongentry->LDIR_Name[4];
                buf->buffer[len+3] = fslongentry->LDIR_Name[6];
                buf->buffer[len+4] = fslongentry->LDIR_Name[8];
                buf->buffer[len+5] = fslongentry->LDIR_Name2[0];
                buf->buffer[len+6] = fslongentry->LDIR_Name2[2];
                buf->buffer[len+7] = fslongentry->LDIR_Name2[4];
                buf->buffer[len+8] = fslongentry->LDIR_Name2[6];
                buf->buffer[len+9] = fslongentry->LDIR_Name2[8];
                buf->buffer[len+10] = fslongentry->LDIR_Name2[10];
                buf->buffer[len+11] = fslongentry->LDIR_Name3[0];
                buf->buffer[len+12] = fslongentry->LDIR_Name3[2];
                for (int i =0; i < 13; i++){
                    if ( buf->buffer[len+i] == 0)
                        buf->buffer[len+i] = ' ';
                }
                len += 13;
                sprintf(&buf->buffer[len]," %2x %2x ",fslongentry->LDIR_Attr, fslongentry->LDIR_Chksum);
                len += 7;
                sprintf(&buf->buffer[len],"%2x %2x ",fslongentry->LDIR_Ord, fslongentry->LDIR_Type);
                len += 6;
                sprintf(&buf->buffer[len],"%4x"LINEFEED,fslongentry->LDIR_FstClusLO);
                len += 6;
            } else {
                /* short name entry */
                memset(&buf->buffer[len],' ',12);
                sprintf(&buf->buffer[len],"%s",entry->DIR_Name);
                for (int i =0; i < 12; i++){
                    if ( buf->buffer[len+i] == 0)
                       buf->buffer[len+i] = ' ';
                }
                len += 12;
                sprintf(&buf->buffer[len],"%2x %10u  ",entry->DIR_Attr, entry->DIR_FileSize);
                len += 15;
                sprintf(&buf->buffer[len],"%4u %4x ",(entry->DIR_FstClusHI << 16) | entry->DIR_FstClusLO,entry->DIR_LstAccDate);
                len += 10;
                sprintf(&buf->buffer[len],"%4x %2x %4x ",entry->DIR_CrtTime,entry->DIR_CrtTimeTenth,entry->DIR_CrtDate);
                len += 13;
                sprintf(&buf->buffer[len],"%2x %4x %4x",entry->DIR_NTRes,entry->DIR_WrtDate,entry->DIR_WrtTime);
                len += 12;
                if (entry->DIR_Name[0] == 0xE5) {
                    sprintf(&buf->buffer[len]," -free-"LINEFEED);
                    len += 9;
                } else {
                    sprintf(&buf->buffer[len],LINEFEED);
                    len += 2;
                }
            }
            entry = getNextEntry(false,false);
        }

        buf->buffer[len] = 0;
        FATAccess->release();

        return (cOk);
    }

    return (cError);

}

FATFile::FATFile(char* p_name,
                 FAT32_DirEntry* p_entry,
                 FATFileSystem * p_fs,
                 unint4 directory_sector,
                 unint2 directory_entry_number,
                 unint4 longname_entry_sector,
                 unint2 longname_entry_number) :
        File(p_name, p_entry->DIR_FileSize, p_entry->DIR_Attr)
{

    this->clusterStart      = ((p_entry->DIR_FstClusHI << 16) | p_entry->DIR_FstClusLO);
    this->currentCluster    = clusterStart;
    this->currentSector     = p_fs->ClusterToSector(clusterStart);
    this->myFS              = p_fs;
    this->directory_sector  = directory_sector;
    this->directory_entry_number = directory_entry_number;
    this->longname_entry_sector  = longname_entry_sector;
    this->longname_entry_number  = longname_entry_number;
}

FATFile::~FATFile() {
    // check if anybody is holding a handle to this..

}

ErrorT FATFile::readBytes(char* bytes, unint4& length) {
    LOG(FILESYSTEM, DEBUG, "FATFile::readBytes length: %d",length);
    unint4 sector_pos;
    unint4 sector_read_len;
    unint4 pos = 0;

    unint4 readlength = length;
    // be sure we are not reading over the end of the file
    if ((this->filesize - this->position) < readlength)
        readlength = (this->filesize - this->position);

    unint4 sector_size = this->myFS->myPartition->getSectorSize();
    bool sector_changed = false;

    FATAccess->acquire();

    while (readlength > 0)
    {

        // position inside our current sector
        //sector_pos 		= this->position % sector_size;
        // sector size must be a multiple of 2
        sector_pos = this->position & (sector_size - 1);

        // set readlength to remaining bytes in this sector
        sector_read_len = sector_size - sector_pos;

        // check if we read less than the remaining bytes in this sector
        if (readlength < sector_read_len)
            sector_read_len = readlength;
        else
        {
            sector_changed = true;
        }

        LOG(FILESYSTEM, DEBUG, "FATFile::readBytes reading Sector: %u, cluster: %u",currentSector,currentCluster);
        // read the sector
        int error = myFS->myPartition->readSectors(currentSector, buffer, 1);
        if (error < 0)
        {
            FATAccess->release();
            return (error);
        }

        // copy the desired bytes in this sector into the buffer
        memcpy(&bytes[pos], &buffer[sector_pos], sector_read_len);
        // increase position by bytes read
        this->position += sector_read_len;
        pos += sector_read_len;

        // check if we reached the sector boundary
        if (sector_changed)
        {
            currentSector = myFS->getNextSector(currentSector, currentCluster);
            if (currentCluster == EOC)
            {
                // end of file reached
                length = pos;
                FATAccess->release();
                return (cOk );
            }
        }

        sector_changed = false;
        // decrease total amount to read by read amount of bytes
        readlength -= sector_read_len;

    }

    FATAccess->release();
    LOG(FILESYSTEM, DEBUG, "FATFile::readBytes Bytes read: %d",length);

    length = pos;
    return (cOk );

}

ErrorT FATFile::writeBytes(const char* bytes, unint4 length) {
    LOG(FILESYSTEM, DEBUG, "FATFile::writeBytes length: %d",length);

    unint4 sector_size = this->myFS->myPartition->getSectorSize();
    unint4 sector_pos;
    unint4 sector_write_len;
    bool sector_changed = false;
    unint4 pos = 0;  // position inside the bytes array
    bool new_sector = false;
    int error;

    FATAccess->acquire();
    // keep writing while we have bytes to write

    while (length > 0)
    {
        // check if the next write operation is overwriting something or appending
        sector_pos = this->position & (sector_size - 1);  // position inside the sector
        sector_write_len = sector_size - sector_pos;  // the length we are writing inside this sector

        // check if we read less than the remaining bytes in this sector
        if (length < sector_write_len)
            sector_write_len = length;
        else
        {
            sector_changed = true;
        }
        LOG(FILESYSTEM, DEBUG, "FATFile::writeBytes sector: %u, cluster: %u, length: %u",currentSector,currentCluster,sector_write_len);

        // read the sector if we are overwriting something
        if (!new_sector)
        {
            error = myFS->myPartition->readSectors(currentSector, buffer, 1);
            if (error < 0)
            {
                FATAccess->release();
                return (error);
            }
        }

        // copy the desired bytes in this sector into the buffer
        memcpy(&buffer[sector_pos], &bytes[pos], sector_write_len);

        error = myFS->myPartition->writeSectors(currentSector, buffer, 1);
        if (error < 0)
        {
            FATAccess->release();
            return (error);
        }

        // increase position by bytes read
        this->position += sector_write_len;
        pos += sector_write_len;

        // check if we reached the sector boundary
        if (sector_changed)
        {
            // append if neccessary
            currentSector = myFS->getNextSector(currentSector, currentCluster, true);
            if (currentCluster == EOC)
            {
                // no more memory..
                FATAccess->release();
                return (cDeviceMemoryExhausted );
            }
            // if we reach a new sector and the position > filesize
            // we can omit reading the contents of that sector
            new_sector = position > filesize;
        }

        sector_changed = false;
        // decrease total amount to read by read amount of bytes
        length -= sector_write_len;

    }

    // directly update the filesize!
    if (this->position > this->filesize)
    {
        // we appended something... update the entries filesize
        error = myFS->myPartition->readSectors(this->directory_sector, buffer, 1);
        if (error < 0)
        {
            FATAccess->release();
            return (error);
        }

        this->filesize = this->position;

        FAT32_DirEntry* entries = (FAT32_DirEntry*) buffer;
        entries[this->directory_entry_number].DIR_FileSize = this->filesize;

        error =  myFS->myPartition->writeSectors(this->directory_sector, buffer, 1);
        if (error < 0)
        {
            FATAccess->release();
            return (error);
        }
    }

    FATAccess->release();
    return (cOk );

}

ErrorT FATFile::resetPosition() {
    this->currentCluster = clusterStart;
    this->currentSector = myFS->ClusterToSector(clusterStart);
    this->position = 0;
    return (cOk );
}

