/*
 * FATFileSystem.cc
 *
 *  Created on: 19.06.2013
 *    Copyright & Author: Daniel Baldin
 */

#include "FATFileSystem.hh"
#include "inc/memtools.hh"

#include "kernel/Kernel.hh"
#include "inc/stringtools.hh"

extern Kernel* theOS;

static char buffer[512];
static char temp_buf[512];

/* Global FAT code access mutex. Used to ensure the buffers are not
 * overwritten by concurrent fat operations in other threads.*/
Mutex* FATAccess = 0;

int FAT_SHORTNAME_STRIP(char* &name, unint1 startpos) __attribute__((noinline));


FATFileSystem::FATFileSystem(Partition* p_myPartition) :
        FileSystemBase(p_myPartition) {
    /* Create the mutex if not created before */
    if (FATAccess == 0) {
        FATAccess = new Mutex("FATFs");
    }

    this->isValid = false;
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

    if ((myFAT_BPB.BS_jmpBoot[0] == 0xeb) || (myFAT_BPB.BS_jmpBoot[0] == 0xe9)) {
        if (myFAT_BPB.BPB_BytsPerSec != 512) {
            LOG(ARCH, ERROR, "FATFileSystem: Invalid Sector Size %d.", myFAT_BPB.BPB_BytsPerSec);

        } else if (myFAT_BPB.BPB_BytsPerSec == myPartition->getSectorSize()) {
            // right now we trust the rest of the table and do checks on demand
            FAT32_BPB* myFAT32BPB = reinterpret_cast<FAT32_BPB*>(&buffer[36]);

            RootDirSectors = (((myFAT_BPB.BPB_RootEntCnt * 32) + (myFAT_BPB.BPB_BytsPerSec - 1)) / myFAT_BPB.BPB_BytsPerSec);

            // getting the first sector of a cluster N:
            // FirstSectorofCluster = ((N-2) * BPB_SecPerClus) + FirstDataSector

            unint4 FATSz;
            if (myFAT_BPB.BPB_FATSz16 != 0) {
                FATSz = myFAT_BPB.BPB_FATSz16;
            } else {
                FATSz = myFAT32BPB->BPB_FATSz32;
            }

            unint4 TotSec;
            if (myFAT_BPB.BPB_TotSec16 != 0) {
                TotSec = myFAT_BPB.BPB_TotSec16;
            } else {
                TotSec = myFAT_BPB.BPB_TotSec32;
            }

            FirstDataSector = myFAT_BPB.BPB_RsvdSecCnt + (myFAT_BPB.BPB_NumFATs * FATSz) + RootDirSectors;
            LOG(ARCH, INFO, "FATFileSystem: First FAT Data Sector: %d", FirstDataSector);

            DataSec = TotSec - (myFAT_BPB.BPB_RsvdSecCnt + (myFAT_BPB.BPB_NumFATs * FATSz) + RootDirSectors);
            this->numBlocks = DataSec;
            this->blockSize = myFAT_BPB.BPB_BytsPerSec;

            CountOfClusters = DataSec / myFAT_BPB.BPB_SecPerClus;
            LOG(ARCH, INFO, "FATFileSystem: Clusters   : %u, DataSectors: %u", CountOfClusters, DataSec);
            LOG(ARCH, INFO, "FATFileSystem: ClusterSize: %u ", myFAT_BPB.BPB_SecPerClus * 512);

            if (CountOfClusters < 4085) {
                this->myFatType = FAT12;
                LOG(ARCH, WARN, "FATFileSystem: FAT12 not supported.. not mounting FS");
            } else if (CountOfClusters < 65525) {
                this->myFatType = FAT16;
                this->isValid = true;
                memcpy(&myFATxx_BPB, &buffer[36], sizeof(FAT16_BPB));
                LOG(ARCH, INFO, "FATFileSystem: found valid FAT16 with %d sectors", myFAT_BPB.BPB_TotSec16);
            } else {
                this->myFatType = FAT32;
                this->isValid = true;
                memcpy(&myFATxx_BPB, &buffer[36], sizeof(FAT32_BPB));
                LOG(ARCH, INFO, "FATFileSystem: found valid FAT32 with %d sectors", myFAT_BPB.BPB_TotSec32);
            }

        } else {
            LOG(ARCH, ERROR, "FATFileSystem: sector size: %d does not match partition sector size %d. Unsupported!", myFAT_BPB.BPB_BytsPerSec, myPartition->getSectorSize());
        }
    } else {
        LOG(ARCH, ERROR, "FATFileSystem: invalid Filesystem. Invalid BS_jmpBoot field");
    }

    FATAccess->release();
}

/*****************************************************************************
 * Method: FATFileSystem::isFATFileSystem(Partition* part)
 *
 * @description
 *  Checks if a given partition contains a FAT filesystem.
 *
 * @returns
 *  bool     True if the partition contains a FAT filesystem.
 *           False otherwise.
 *******************************************************************************/
bool FATFileSystem::isFATFileSystem(Partition* part) {
    /* Create the mutex if not created before */
    if (FATAccess == 0) {
       FATAccess = new Mutex("FATFs");
    }

    FATAccess->acquire();

    /* read out first sector */
    int error = part->readSectors(0, buffer, 1);
    if (error < 0) {
        FATAccess->release();
        return (false);
    }

    FAT_BS_BPB* myFAT_BPB = reinterpret_cast<FAT_BS_BPB*>(buffer);
    if ((myFAT_BPB->BS_jmpBoot[0] == 0xeb) || (myFAT_BPB->BS_jmpBoot[0] == 0xe9)) {
        if (myFAT_BPB->BPB_BytsPerSec != 512) {
            FATAccess->release();
            return (false);
        }

        FATAccess->release();
        return (true);
    }

    FATAccess->release();
    return (false);
}

/*****************************************************************************
 * Method: FATFileSystem::allocateCluster()
 *
 * @description
 *
 *******************************************************************************/
unint4 FATFileSystem::allocateCluster() {
    LOG(ARCH, DEBUG, "FATFileSystem: Trying to allocate new cluster");

    // TODO: add FAT 16 allocate cluster support
    if (this->myFatType == FAT16) {
        return (0xFFFF);
    } else {
        unint4 fsinfo_lba = myFATxx_BPB.myFAT32_BPB.BPB_FSInfo * (myFAT_BPB.BPB_BytsPerSec / this->myPartition->getSectorSize());
        int error         = myPartition->readSectors(fsinfo_lba, temp_buf, 1);
        if (error < 0) {
            return (cError);
        }

        FAT32_FSInfo *fsinfo = reinterpret_cast<FAT32_FSInfo *>(&temp_buf[0]);

        /* use next free cluster as hint.. if not set start searching at cluster 2*/
        unint4 cluster_start = fsinfo->FSI_Nxt_Free;
        if (cluster_start == 0xFFFFFFFF) {
            cluster_start = 2;
        }

        // walk through the table and find a free entry
        unint4 FATEntryValue      = EOC;
        unint4 currentCluster     = cluster_start;
        unint4 currentClusterAddr = cluster_start * 4;
        unint4 fatsecnum;
        unint4 fatentoffset;

        // iterate over the whole FAT table if needed
        while (FATEntryValue != 0 && currentCluster < CountOfClusters) {
            // get table entry offset in partition sector
            // unint4 fatentoffset = addr % myFAT_BPB.BPB_BytsPerSec;
            fatentoffset = currentClusterAddr & (/*myFAT_BPB.BPB_BytsPerSec*/ 512 - 1);

            // get the partition sector containing the table entry
            fatsecnum = myFAT_BPB.BPB_RsvdSecCnt + (currentClusterAddr / /*myFAT_BPB.BPB_BytsPerSec*/ 512);

            // read the sector
            error = myPartition->readSectors(fatsecnum, temp_buf, 1);
            if (error < 0) {
                return (EOC);
            }

            /* read all FAT entries found inside this sector if needed */
            while (FATEntryValue != 0 && fatentoffset < /*myFAT_BPB.BPB_BytsPerSec*/ 512) {
                FATEntryValue = (*(reinterpret_cast<unint4*>(__builtin_assume_aligned(&temp_buf[fatentoffset], 4)))) & 0x0FFFFFFF;
                LOG(ARCH, TRACE, "FATFileSystem: FAT Entry: %u = %x", fatentoffset / 4, FATEntryValue);

                /* check next entry if this one not free */
                fatentoffset       += 4;
                currentClusterAddr += 4; /* every entry stands for one cluster.. */
                currentCluster++;
            }
        }

        if (FATEntryValue == 0) {
            // found free entry
            fatentoffset -= 4;
            currentCluster--;
            // mark as end of chain
            (*(reinterpret_cast<unint4*>(__builtin_assume_aligned(&temp_buf[fatentoffset], 4)))) = 0xffffff8;
            error = myPartition->writeSectors(fatsecnum, temp_buf, 1);
            if (error < 0) {
                return (EOC);
            }

            // update FS_INFO
            error = myPartition->readSectors(fsinfo_lba, temp_buf, 1);
            if (error < 0) {
                return (EOC);
            }

            fsinfo = reinterpret_cast<FAT32_FSInfo *>(&temp_buf[0]);
            fsinfo->FSI_FreeCount--;
            fsinfo->FSI_Nxt_Free = currentCluster + 1;

            error = myPartition->writeSectors(fsinfo_lba, temp_buf, 1);
            if (error < 0) {
                return (EOC);
            }

            return (currentCluster);
        }

        return (EOC);
    }
}

/*****************************************************************************
 * Method: FATFileSystem::freeClusterChain(unint4 clusterStart)
 *
 * @description
 *
 *******************************************************************************/
bool FATFileSystem::freeClusterChain(unint4 clusterStart) {
    if (this->myFatType == FAT16) {
        return (false);
    } else {
        unint4 FATEntryAddr = clusterStart * 4;
        unint4 fatsecnum;
        unint4 fatentoffset;
        int error;
        unint4 freeCount = 0;
        unint4 FATEntryValue = 0;

        /* get table entry offset in partition sector
         * unint4 fatentoffset = addr % myFAT_BPB.BPB_BytsPerSec; */
        fatentoffset = FATEntryAddr & (/*myFAT_BPB.BPB_BytsPerSec*/ 512 - 1);

        /* get the partition sector containing the table entry */
        fatsecnum = myFAT_BPB.BPB_RsvdSecCnt + (FATEntryAddr / /*myFAT_BPB.BPB_BytsPerSec*/ 512);

        /* read the sector */
        LOG(FILESYSTEM, DEBUG, "FATFileSystem::freeClusterChain() clusterchain start at cluster %d. reading sector %d, ", clusterStart, fatsecnum);

        error = myPartition->readSectors(fatsecnum, temp_buf, 1);
        if (error < 0) {
            return (false);
        }

        /* read all FAT entries found inside this sector if needed */
        while (FATEntryValue != 0x0ffffff8 && fatentoffset < myFAT_BPB.BPB_BytsPerSec) {
            FATEntryValue = (*(reinterpret_cast<unint4*>(__builtin_assume_aligned(&temp_buf[fatentoffset], 4)))) & 0x0FFFFFFF;
            if (FATEntryValue == 0) {
                LOG(FILESYSTEM, WARN, "FATFileSystem: Cluster Chain Entry already free. Stopping.");
                break;
            }

            LOG(FILESYSTEM, DEBUG, "FATFileSystem: free FAT Entry: %u Cluster: %u, Next Cluster: %x, ", fatentoffset / 4, FATEntryAddr / 4, FATEntryValue);

            (*(reinterpret_cast<unint4*>(__builtin_assume_aligned(&temp_buf[fatentoffset], 4)))) = 0;
            freeCount++;

            /* stop if no next cluster in the chain before reading the next sector below*/
            if (FATEntryValue >= 0xffffff8) break;

            /* if we get here more entries are following as we did not reach the end of the
             * cluster chain yet..
             * calculate next fat entry..*/
            FATEntryAddr       = FATEntryValue * 4;
            fatentoffset       = FATEntryAddr & (/*myFAT_BPB.BPB_BytsPerSec*/ 512 - 1);

            /* get the partition sector containing the next fat table entry
             * check if we need to read the next sector..*/
            unint4 nextfatsecnum = myFAT_BPB.BPB_RsvdSecCnt + (FATEntryAddr / /*myFAT_BPB.BPB_BytsPerSec*/ 512);
            if (nextfatsecnum != fatsecnum) {
                /* write back sector and get next one!*/
                LOG(FILESYSTEM, DEBUG, "FATFileSystem::freeClusterChain() writing sector %d ", fatsecnum);
                error = myPartition->writeSectors(fatsecnum, temp_buf, 1);
                fatsecnum = nextfatsecnum;
                LOG(FILESYSTEM, DEBUG, "FATFileSystem::freeClusterChain() reading sector %d ", fatsecnum);
                error = myPartition->readSectors(fatsecnum, temp_buf, 1);
            }
        }

        /* done.. write last sector back */
        error = myPartition->writeSectors(fatsecnum, temp_buf, 1);

        // update FS_INFO
        unint4 fsinfo_lba = myFATxx_BPB.myFAT32_BPB.BPB_FSInfo * (myFAT_BPB.BPB_BytsPerSec / this->myPartition->getSectorSize());
        error = myPartition->readSectors(fsinfo_lba, temp_buf, 1);

        FAT32_FSInfo *fsinfo = reinterpret_cast<FAT32_FSInfo *>(&temp_buf[0]);
        fsinfo->FSI_FreeCount += freeCount;

        if (fsinfo->FSI_Nxt_Free > clusterStart) {
            fsinfo->FSI_Nxt_Free = clusterStart;
        }

        error = myPartition->writeSectors(fsinfo_lba, temp_buf, 1);

        if (isOk(error)) {
            return (true);
        } else {
            return (false);
        }
    }
}

/*****************************************************************************
 * Method: FATFileSystem::getFATTableEntry(unint4 clusterNum, bool allocate)
 *
 * @description
 *
 *******************************************************************************/
unint4 FATFileSystem::getFATTableEntry(unint4 clusterNum, bool allocate) {
    LOG(FILESYSTEM, DEBUG, "FATFileSystem:getFATTableEntry: %u", clusterNum);

    unint4 addr;
    if (this->myFatType == FAT16) {
        addr = clusterNum * 2;
    } else {
        addr = clusterNum * 4;
    }

    // get table entry offset in partition sector
    // unint4 fatentoffset = addr % myFAT_BPB.BPB_BytsPerSec;
    unint4 fatentoffset = addr & (/*myFAT_BPB.BPB_BytsPerSec*/ 512 - 1);

    // get the partition sector containing the table entry
    unint4 fatsecnum = myFAT_BPB.BPB_RsvdSecCnt + (addr / /*myFAT_BPB.BPB_BytsPerSec*/ 512);

    // read the sector
    int error = myPartition->readSectors(fatsecnum, buffer, 1);
    if (error < 0) {
        return (EOC);
    }

    // now get the table entry
    // this is 4 bytes aligned.. however compiler does not know this
    // thus this may produce a warning
    if (myFatType == FAT16) {
        unint2 nextCluster =  (*(reinterpret_cast<unint2*>(__builtin_assume_aligned(&buffer[fatentoffset], 2))));

        if ((nextCluster >= 0xFFF8) && allocate) {
            nextCluster = (unint2) allocateCluster();

            int error = myPartition->readSectors(fatsecnum, buffer, 1);
            /* set FAT entry */
            (*(reinterpret_cast<unint2*>(__builtin_assume_aligned(&buffer[fatentoffset], 2)))) = nextCluster;
            error = myPartition->writeSectors(fatsecnum, buffer, 1);
            if (error < 0)
                return (EOC);
        }

        return (nextCluster);
    } else {
        unint4 nextCluster = (*(reinterpret_cast<unint4*>(__builtin_assume_aligned(&buffer[fatentoffset], 4)))) & 0x0FFFFFFF;
        LOG(FILESYSTEM, DEBUG, "FATFileSystem:getFATTableEntry: entry: %u", nextCluster);

        if ((nextCluster >= 0xFFFFFF8) && allocate) {
            nextCluster = allocateCluster();
            LOG(FILESYSTEM, TRACE, "FATFileSystem: allocated cluster: %u", nextCluster);

            int error = myPartition->readSectors(fatsecnum, buffer, 1);
            /* set FAT entry */
            (*(reinterpret_cast<unint4*>(__builtin_assume_aligned(&buffer[fatentoffset], 4)))) = nextCluster;
            /* write back the part of the FAT table */
            error = myPartition->writeSectors(fatsecnum, buffer, 1);
            if (error < 0)
                return (EOC);
        }
        return (nextCluster);
    }
}

/*****************************************************************************
 * Method: FATFileSystem::getNextSector(unint4 currentSector,
 *                                      unint4 &currentCluster,
 *                                      bool allocate)
 *
 * @description
 *
 *******************************************************************************/
unint4 FATFileSystem::getNextSector(unint4 currentSector, unint4 currentCluster, bool allocate) {
    // check if next sector lies inside another cluster
    // if so follow cluster chain
    // if ( (currentSector / myFAT_BPB.BPB_SecPerClus) != ((currentSector+1) / myFAT_BPB.BPB_SecPerClus)) {
    if ((currentSector >> sector_shift_value) != ((currentSector + 1) >> sector_shift_value)) {
        // next sector is in another clusters
        // lookup the FAT table to find the successor cluster
        unint4 nextClusterNum = getFATTableEntry(currentCluster, allocate);

        currentCluster = EOC;
        if (this->myFatType == FAT16) {
            if (nextClusterNum >= 0xFFF8) {
                return (EOC);
            }
        } else if (nextClusterNum >= 0xFFFFFF8) {
            return (EOC);
        }

        currentCluster = nextClusterNum;
        return (ClusterToSector(nextClusterNum));
    } else {
        return (currentSector + 1);
    }
}

FATFileSystem::~FATFileSystem() {
    // get orcos mount directory
    Directory* mntdir = theOS->getFileManager()->getDirectory("/mnt/");

    if (mntdir != 0 && rootDir != 0) {
        /* remove root directory to system */
        mntdir->remove(rootDir);
        /* schedule deletion instead of directly freeing the memory
         * as some threads may be accessing the directory contents
         * right now!*/
        theOS->getMemoryManager()->scheduleDeletion(rootDir);
    }
}

/*****************************************************************************
 * Method: FATFileSystem::initialize()
 *
 * @description
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT FATFileSystem::initialize() {
    if (this->isValid) {
        /* get orcos mount directory */
        Directory* mntdir = theOS->getFileManager()->getDirectory("/mnt/");

        if (myFatType == FAT32) {
            /* get FSInfo */
            unint4 fsinfo_lba = myFATxx_BPB.myFAT32_BPB.BPB_FSInfo * (myFAT_BPB.BPB_BytsPerSec / this->myPartition->getSectorSize());
            int error = myPartition->readSectors(fsinfo_lba, buffer, 1);
            if (error < 0)
                return (cError );

            FAT32_FSInfo *fsinfo = reinterpret_cast<FAT32_FSInfo *>(&buffer[0]);
            if (fsinfo->FSI_LeadSig != 0x41615252) {
                LOG(ARCH, WARN, "FATFileSystem: FS_INFO Lead Sig Validation failed: %x != 0x41615252.", fsinfo->FSI_LeadSig);
            }
            if (fsinfo->FSI_StrucSig != 0x61417272) {
                LOG(ARCH, WARN, "FATFileSystem: FS_INFO Struc Sig Validation failed: %x != 0x61417272", fsinfo->FSI_StrucSig);
            }

            LOG(ARCH, INFO, "FATFileSystem: Free Cluster Count: %d", fsinfo->FSI_FreeCount);
            this->freeBlocks = fsinfo->FSI_FreeCount * this->myFAT_BPB.BPB_SecPerClus;

            if (mntdir == 0) {
                LOG(ARCH, ERROR, "FATFileSystem: no mount directory '/mnt/' found. Not mounting");
                return (cError );
            }

            char *name = reinterpret_cast<char*>(theOS->getMemoryManager()->alloc(12));
            memcpy(name, myFATxx_BPB.myFAT32_BPB.BS_VolLab, 11);
            name[11] = 0;
            FAT_SHORTNAME_STRIP(name, 10);

            if (!(strncmp("NO NAME", myFATxx_BPB.myFAT32_BPB.BS_VolLab, 7) != 0)) {
                /* Read the volume label from the root directory */
                int error = myPartition->readSectors(ClusterToSector(myFATxx_BPB.myFAT32_BPB.BPB_RootClus), buffer, 1);
                if (error < 0)
                    return (cError );
                FAT32_DirEntry *fsrootdir_entry = reinterpret_cast<FAT32_DirEntry *>(&buffer[0]);
                if (fsrootdir_entry->DIR_Attr & ATTR_VOLUME_ID) {
                    /* take this entry name */
                    memcpy(name, fsrootdir_entry->DIR_Name, 11);
                    FAT_SHORTNAME_STRIP(name, 10);
                }
            }

            LOG(ARCH, INFO, "FATFileSystem: Volume: '%s' ID: %x", name, myFATxx_BPB.myFAT32_BPB.BS_VolID);

            /* add root directory to system */
            rootDir = new FATDirectory(this, myFATxx_BPB.myFAT32_BPB.BPB_RootClus, name);
            mntdir->add(rootDir);
            LOG(ARCH, INFO, "FATFileSystem: FAT Filesystem mounted to '/mnt/%s/'.", name);

            /* go on and mount this fs */
            return (cOk);
        } else {
            // FAT 16
            char *name = reinterpret_cast<char*>(theOS->getMemoryManager()->alloc(12));
            name[11] = 0;
            memcpy(name, myFATxx_BPB.myFAT16_BPB.BS_VolLab, 11);

            FAT_SHORTNAME_STRIP(name, 10);

            LOG(ARCH, INFO, "FATFileSystem: Volume: '%s' ID: %x", name, myFATxx_BPB.myFAT16_BPB.BS_VolID);

            unint4 FirstRootDirSecNum = myFAT_BPB.BPB_RsvdSecCnt + (myFAT_BPB.BPB_NumFATs * myFAT_BPB.BPB_FATSz16);
            rootDir = new FATDirectory(this, 0, name, FirstRootDirSecNum);
            mntdir->add(rootDir);

            LOG(ARCH, INFO, "FATFileSystem: FAT Filesystem mounted to '/mnt/%s/'.", name);

            // go on and mount this fs
            return (cOk);
        }
    }

    return (cError);
}


