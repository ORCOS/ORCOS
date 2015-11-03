/*
 * FATDirectory.cc
 *
 *  Created on: 01.11.2015
 *      Author: Daniel Baldin
 */
#include "FATFileSystem.hh"
#include "inc/memtools.hh"
#include "inc/stringtools.hh"
#include "kernel/Kernel.hh"

extern Kernel* theOS;

//! the current entry returned by getEntry iterator
static unint2 it_currentSectorEntry = -1;

//! current cluster used by the getEntry iterator
static unint4 it_currentCluster = -1;

//! curretn sector used  by the getEntry iterator
static unint4 it_currentSector = -1;

extern Mutex* FATAccess;

static char buffer[512];


/*****************************************************************************
 *   FAT FILESYSTEM HELPER METHODS
 *******************************************************************************/

int FAT_SHORTNAME_STRIP(char* &name, unint1 startpos) {
    for (int i = startpos; i > 0; i--) {
        if (name[i] == ' ') {
            name[i] = 0;
        } else {
            return (i);
        }
    }

    /* should not happen as this should be a filename violation
     however, be safe here */
    return (0);
}


/*****************************************************************************
 * Method: ChkSum(unsigned char *pFcbName)
 *
 * @description
 *    Returns an unsigned byte checksum computed on an unsigned byte
 *    array.  The array must be 11 bytes long and is assumed to contain
 *    a name stored in the format of a MS-DOS directory entry.
 *    Passed:     pFcbName    Pointer to an unsigned byte array assumed to be
 *                          11 bytes long.
 *    Returns: Sum         An 8-bit unsigned checksum of the array pointed
 *                           to by pFcbName.
 *******************************************************************************/
unsigned char ChkSum(unsigned char *pFcbName) {
    unint2 FcbNameLen;
    unsigned char Sum;

    Sum = 0;
    for (FcbNameLen = 11; FcbNameLen != 0; FcbNameLen--) {
        // NOTE: The operation is an unsigned char rotate right
        Sum = ((Sum & 1) ? 0x80 : 0) + (Sum >> 1) + *pFcbName++;
    }
    return (Sum);
}


/*****************************************************************************
 *   FAT Directory implementation
 *******************************************************************************/


FATDirectory::FATDirectory(FATFileSystem* parentFileSystem, unint4 cluster_num, const char* p_name) :
        Directory(p_name) {
    this->myFS          = parentFileSystem;
    this->mycluster_num = cluster_num;
    this->populated     = false;
    this->sector        = myFS->ClusterToSector(mycluster_num);
    last_entry          = 0;
    last_entry_sector   = sector; /*set when populate directory is called */
}

FATDirectory::FATDirectory(FATFileSystem* parentFileSystem, unint4 cluster_num, const char* p_name, unint4 u_sector) :
        Directory(p_name) {
    this->myFS          = parentFileSystem;
    this->mycluster_num = cluster_num;
    this->populated     = false;
    this->sector        = u_sector;
    last_entry          = 0;
    last_entry_sector   = sector; /*set when populate directory is called */
}


/*****************************************************************************
 * Method: FATDirectory::readBytes(char* bytes, unint4& length)
 *
 * @description
 *
 *******************************************************************************/
ErrorT FATDirectory::readBytes(char* bytes, unint4& length) {
    if (!populated)
        this->populateDirectory();
    return (Directory::readBytes(bytes, length));
}



/*****************************************************************************
 * Method: FATDirectory::populateDirectory()
 *
 * @description
 *
 *******************************************************************************/
void FATDirectory::populateDirectory() {
    unint4 currentCluster = mycluster_num;
    /* cluster number to sector number conversion
     * get root sector number */
    unint4 sector_number = this->sector;
    last_entry = 0;

    int error = myFS->myPartition->readSectors(sector_number, buffer, 1);
    if (error < 0) return;

    FAT32_DirEntry *fsrootdir_entry = reinterpret_cast<FAT32_DirEntry *>(&buffer[0]);
    unint4 entries_per_sector       = myFS->myPartition->getSectorSize() / 32;

    unint2 currentEntry     = 0;    /* current directory entry number inside the sector 0 .. */
    char*  longname         = 0;    /* the current extracted long name  */
    unint1 chksum           = 0;    /* the current chksum value */
    unint2 longNameEntry    = -1;   /* the current long name entry number inside the sector */
    unint4 longNameSector   = -1;

    /* while not end of dir entries */
    while (fsrootdir_entry->DIR_Name[0] != 0) {
        FAT32_LongDirEntry* fslongentry = reinterpret_cast<FAT32_LongDirEntry*>(fsrootdir_entry);

        //printf("Entry: %u Sector: %u:\r\n", currentEntry, sector_number);
        //memdump(fslongentry, sizeof(FAT32_LongDirEntry));

        /* check if this is not a free entry */
        if (fsrootdir_entry->DIR_Name[0] != 0xE5) {
            if ((fslongentry->LDIR_Attr & ATTR_LONG_NAME_MASK) == ATTR_LONG_NAME) {
                /* Found an active long name sub-component.   */
                unint1 n = fslongentry->LDIR_Ord & 0x3F;
                if ((fslongentry->LDIR_Ord & 0x40) == 0x40) {
                    longname            = new char[(13*n) +1];
                    longname[13 * n]    = 0;
                    chksum              = fslongentry->LDIR_Chksum;
                    longNameEntry       = currentEntry;
                    longNameSector      = sector_number;
                }

                // drop long name on chksum mismatch (treat as orphan)
                if (fslongentry->LDIR_Chksum != chksum) {
                    LOG(ARCH, DEBUG, "FAT Longname Entry treated as orphan. Checksum Mismatch:  %x != %x", fslongentry->LDIR_Chksum, chksum);
                    delete[] longname;
                    longname = 0;
                    longNameEntry = -1;
                }

                if (longname != 0 && n > 0) {
                    /* now copy 16 bit unicode to longname
                     * unicode is in little-endian byte order */
                    char* pos = &longname[13 * (n - 1)];
                    unicode2ascii(&fslongentry->LDIR_Name[0] , &pos[0] , 5);
                    unicode2ascii(&fslongentry->LDIR_Name2[0], &pos[5] , 6);
                    unicode2ascii(&fslongentry->LDIR_Name3[0], &pos[11], 2);
                }

            } else {
                /* we have some short entry */
                if (fsrootdir_entry->DIR_Attr != ATTR_VOLUME_ID) {
                    unint4 clusterid = (fsrootdir_entry->DIR_FstClusHI << 16) | fsrootdir_entry->DIR_FstClusLO;
                    char* p_name;

                    if ((longname != 0) && (ChkSum((unsigned char*) fsrootdir_entry->DIR_Name) != chksum)) {
                        LOG(FILESYSTEM, DEBUG, "FAT longname treated as orphan. Checksum Mismatch:  %x != %x",
                                               ChkSum((unsigned char*) fsrootdir_entry->DIR_Name), chksum);
                        delete[] longname;
                        longname = 0;
                    }

                    if (longname == 0) {
                        p_name      =  new char[16];
                        p_name[11]  = 0;
                        memcpy(p_name, fsrootdir_entry->DIR_Name, 11);
                        /* check windows lowercase filename convention on DOS 8.3 names*/
                        if (fsrootdir_entry->DIR_CrtTimeTenth & (1 << 3)) {
                            strnlower(p_name, 8);     /* lowercase basename */
                        }
                        if (fsrootdir_entry->DIR_CrtTimeTenth & (1 << 4)) {
                            strnlower(&p_name[8], 3); /* lowercase extension */
                        }
                    } else {
                        p_name = longname;
                        FAT_SHORTNAME_STRIP(p_name, (unint1) (strlen(p_name) - 1));
                    }

                    if ((fsrootdir_entry->DIR_Attr & ATTR_DIRECTORY) != 0) {
                        /* new directory entry */
                        if (longname == 0)
                            FAT_SHORTNAME_STRIP(p_name, 9);

                        FATDirectory *fatDir = new FATDirectory(this->myFS, clusterid, p_name);
                        this->add(fatDir);
                    } else {
                        /* add the file to this directory */
                        if (longname == 0) {
                            int lastpos = FAT_SHORTNAME_STRIP(p_name, 7);
                            lastpos++;

                            // file extension handling for fat names
                            char ext[3];
                            memcpy(ext, &p_name[8], 3); // to be unrolled by compiler
                            if (ext[0] != ' ') {
                                p_name[lastpos]     = '.';
                                memcpy(&p_name[lastpos + 1], ext,3); // to be unrolled by compiler
                                p_name[lastpos + 4] = 0;
                            } else {
                                p_name[lastpos] = 0;
                            }
                        }

                        FATFile *file = new FATFile(p_name, this, fsrootdir_entry, myFS, sector_number, currentEntry, longNameSector, longNameEntry);
                        this->add(file);
                    }

                    longname = 0;
                    longNameEntry = -1;

                    LOG(FILESYSTEM, TRACE, "'%s'\t%x\t%d\t%d", p_name, fsrootdir_entry->DIR_Attr, clusterid, fsrootdir_entry->DIR_FileSize);
                }  // if not ATTR_VOLUME_ID
            }  // if not long name entry
        }  // if valid entry

        fsrootdir_entry++;
        currentEntry++;
        /* valid entries in [0, entries_per_sector-1]*/
        if (currentEntry >= entries_per_sector) {
            /* get next sector number from FAT table */
            unint4 next_sector_number = this->myFS->getNextSector(sector_number, currentCluster);
            if (next_sector_number == EOC) {
                currentEntry--;
                break;
            }

            sector_number  = next_sector_number;
            currentCluster = this->myFS->SectorToCluster(sector_number);

            /* load sector content into buffer */
            error = myFS->myPartition->readSectors(sector_number, buffer, 1);
            if (error < 0)
                return;

            /* set to first entry inside sector */
            currentEntry = 0;
            fsrootdir_entry = reinterpret_cast<FAT32_DirEntry *>(&buffer[0]);
        }
    }

    last_entry        = currentEntry;
    last_entry_sector = sector_number;

    LOG(FILESYSTEM, DEBUG, "Populated '%s' lastentry=%u, last_entry_sector=%u", this->name, last_entry, last_entry_sector);

    this->populated = true;
}


/*****************************************************************************
 * Method: FATDirectory::get(const char* p_name, unint1 name_len)
 *
 * @description
 *
 *******************************************************************************/
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




/*****************************************************************************
 * Method: FATDirectory::allocateEntry(char* p_name,
 *                    int DIR_Attr,
 *                    FAT32_DirEntry* &fsrootdir_entry,
 *                    unint4 &fileEntrySector,
 *                    unint2 &fileSectorEntry,
 *                    unint4 &longNameEntrySector,
 *                    unint2 &longNameSectorEntry,
 *                    unint4 cluster)
 *
 * @description
 *   Tries to allocate a new fat entry inside the given fat directory.
 *
 * @returns
 *   fsrootdir_entry    Pointer to the directory entry
 *******************************************************************************/
ErrorT FATDirectory::allocateEntry(char*            p_name,
                                   int              DIR_Attr,
                                   FAT32_DirEntry*  &fsrootdir_entry,
                                   unint4           &fileEntrySector,
                                   unint2           &fileSectorEntry,
                                   unint4           &longNameEntrySector,
                                   int2             &longNameSectorEntry,
                                   unint4           cluster) {
    if (get(p_name, strlen(p_name))) {
          LOG(FILESYSTEM, ERROR, " FATDirectory::allocateEntry failed for '%s': entry exists.", p_name);
          return (cError);
    }

    /* get exclusive access to the FAT buffers */
     FATAccess->acquire();

     fileEntrySector        = -1;
     fileSectorEntry        = -1;

     longNameEntrySector    = -1;
     longNameSectorEntry    = -1;

     unint4 namelen         = strlen(p_name);
     if (namelen > 255) {
         LOG(FILESYSTEM, ERROR, "FATDirectory::allocateEntry() entry name length %d too big", namelen);
         return (cError);
     }
     unint2 long_entries    = (unint2) ((namelen / 13));
     if (namelen - (long_entries * 13) > 0) {
         long_entries++;
     }
     unint2 freeEntries     = 0; /* Number of consecutive free entries found */
     unint4 freeEntrySector = 0; /* Sector of the consecutive free entries start */
     unint4 freeEntryNum    = 0; /* Entry number inside the sector */

     /* search for enough consecutive free directory entries for this new entry
      * or until we reach the end of the directory table */
     fsrootdir_entry = resetToFirstEntry();
     if (!fsrootdir_entry){
         LOG(FILESYSTEM, ERROR, "FATDirectory::allocateEntry() Directory sector corrupt..");
         FATAccess->release();
         return (cError);
     }

     while (fsrootdir_entry != 0 && fsrootdir_entry->DIR_Name[0] != 0) {
         if (fsrootdir_entry->DIR_Name[0] == 0xE5) {
             freeEntries++;
             if (freeEntrySector == 0) {
                 freeEntrySector = it_currentSector;
                 freeEntryNum    = it_currentSectorEntry;
             }
             /* stop iterating over entries as we have found enough free slots to overwrite */
             if (freeEntries == (long_entries +1)) break;
         } else {
             freeEntries     = 0;
             freeEntrySector = 0;
         }
         fsrootdir_entry = getNextEntry(false, false);
     }
     if (fsrootdir_entry == 0) {
         fsrootdir_entry = getNextEntry(false, true);
     }
     if (freeEntries == (long_entries +1)) {
         /* reset to first entry to overwrite */
         fsrootdir_entry = moveToEntry(freeEntryNum, freeEntrySector);
     }

     /* position to insert found. get the new cluster for the file */
     unint4 fileCluster = cluster;
     if (fileCluster == 0) {
         fileCluster = myFS->allocateCluster();
         if (fileCluster == EOC) {
             LOG(FILESYSTEM, ERROR, "FATDirectory::allocateEntry() Out of clusters");
             FATAccess->release();
             return (cError);
         }
         LOG(FILESYSTEM, INFO, " FATDirectory::allocateEntry() new Entry allocated to Cluster %d", fileCluster);
     }
     /* create entries at the end */
     unsigned char shortname[11];
     /* generate the DOS shortname */
     generateShortName(shortname, p_name);
     char temp_name[13];

     FAT32_LongDirEntry* fslongentry;
     longNameEntrySector = it_currentSector; /* remember the longname sector */
     longNameSectorEntry = it_currentSectorEntry;
     LOG(FILESYSTEM, INFO, " FATDirectory::allocateEntry() Longname sector         : %u", longNameEntrySector);
     LOG(FILESYSTEM, INFO, " FATDirectory::allocateEntry() Longname entry in sector: %u", longNameSectorEntry);

     /* create long entries first */
     for (int i = 0; i < long_entries; i++) {
         fslongentry = reinterpret_cast<FAT32_LongDirEntry*>(fsrootdir_entry);
         memset(fslongentry, 0, sizeof(FAT32_LongDirEntry));

         fslongentry->LDIR_Attr      = ATTR_LONG_NAME;
         fslongentry->LDIR_Chksum    = ChkSum(shortname);
         fslongentry->LDIR_FstClusLO = 0;
         fslongentry->LDIR_Type      = 0;
         if (i == 0) {
             fslongentry->LDIR_Ord = (unint1) ((long_entries - i) | 0x40);
         } else {
             fslongentry->LDIR_Ord = (unint1) (long_entries - i);
         }

         /* now set name fields */
         char* pos = &p_name[13 * ((long_entries - i) - 1)];
         int len = 13;
         if (i == 0) {
             len = namelen - ((long_entries - 1) * 13);
             if (len < 13)
                 len++; /* be sure trailing 0 is copied */
         }

         /* be sure unused name fields are set to 0xffff */
         memset(temp_name, 0xff, 13);
         memcpy(temp_name, pos, len);

         /* set unicode name parts */
         ascii2unicode(&temp_name[0],  &fslongentry->LDIR_Name[0] , 5);
         ascii2unicode(&temp_name[5],  &fslongentry->LDIR_Name2[0], 6);
         ascii2unicode(&temp_name[11], &fslongentry->LDIR_Name3[0], 2);

         /* get (allocate) next sector if last entry
          * and write back if sector changes */
         fsrootdir_entry = getNextEntry(true, true);
     }

     if (fsrootdir_entry == 0) {
         LOG(FILESYSTEM, ERROR, "FATDirectory::allocateEntry() could not allocate/get short name entry");
         FATAccess->release();
         return (cError);
     }

     fileSectorEntry = it_currentSectorEntry;
     fileEntrySector = it_currentSector;

     LOG(FILESYSTEM, INFO, " FATDirectory::allocateEntry() Shortname sector         : %d", fileEntrySector);
     LOG(FILESYSTEM, INFO, " FATDirectory::allocateEntry() Shortname entry in sector: %d", fileSectorEntry);

     /* write last (short) entry containing file information */
     memset(fsrootdir_entry, 0, sizeof(FAT32_DirEntry));
     memcpy(fsrootdir_entry->DIR_Name, shortname, 11);
     // TODO endianess?
     fsrootdir_entry->DIR_FstClusHI  = (unint2) (fileCluster >> 16);
     fsrootdir_entry->DIR_FstClusLO  = fileCluster & 0xffff;
     fsrootdir_entry->DIR_Attr       = DIR_Attr;  // mark as backup

     static FAT32_DirEntry retEntry;
     memcpy(&retEntry, fsrootdir_entry, sizeof(FAT32_DirEntry));
     fsrootdir_entry = &retEntry;

     /* check if we are appending.. if so add end of table entry */
     if (freeEntries != (long_entries +1)) {
         /* we will overwrite the trailing entry with the new long name entry
          * and append a new 'end of entries' entry at the very end */
         /* set new end, allocate cluster if needed */
         FAT32_DirEntry* next_entry = getNextEntry(true, true);
         if (!next_entry) {
             LOG(FILESYSTEM, ERROR, "FATDirectory::allocateEntry() could not allocate end of directory entry");
             FATAccess->release();
             return (cError);
         }
         memset(next_entry, 0, sizeof(FAT32_DirEntry));
     }

     /* final synch of the current buffer entries */
     synchEntries();

     /* buffers are now free to be used by another thread */
     FATAccess->release();

     return (cOk);
}



/*****************************************************************************
 * Method: FATDirectory::createFile(char* p_name, unint4 flags)
 *
 * @description
 *
 *******************************************************************************/
File* FATDirectory::createFile(char* p_name, unint4 flags) {
    unint4 fileEntrySector = -1;
    unint2 fileSectorEntry = -1;

    unint4 longNameEntrySector = -1;
    int2   longNameSectorEntry = -1;
    FAT32_DirEntry* fsrootdir_entry;

    ErrorT ret = allocateEntry(p_name, ATTR_ARCHIVE, fsrootdir_entry, fileEntrySector, fileSectorEntry, longNameEntrySector, longNameSectorEntry);
    if (isError(ret)) {
        return (0);
    }

    FATFile *file = new FATFile(p_name, this, fsrootdir_entry, myFS, fileEntrySector, fileSectorEntry, longNameEntrySector, longNameSectorEntry);
    this->add(file);

    /* be sure data is written to device */
    this->myFS->myPartition->flushCache();

    return (file);
}



/*****************************************************************************
 * Method: FATDirectory::createDirectory(char* name, unint4 flags)
 *
 * @description
 *
 *******************************************************************************/
Directory* FATDirectory::createDirectory(char* p_name, unint4 flags) {
    unint4 fileEntrySector = -1;
    unint2 fileSectorEntry = -1;

    unint4 longNameEntrySector = -1;
    int2   longNameSectorEntry = -1;
    FAT32_DirEntry* fsrootdir_entry;

    unint4 namelen         = strlen(p_name);
    char* namepcpy = new char[namelen +1];
    memcpy(namepcpy, p_name, namelen + 1);

    ErrorT ret = allocateEntry(namepcpy, ATTR_DIRECTORY, fsrootdir_entry, fileEntrySector, fileSectorEntry, longNameEntrySector, longNameSectorEntry);
    if (isError(ret)) {
        return (0);
    }

    unint4 clusterNum =  (((unint4)fsrootdir_entry->DIR_FstClusHI) << 16) | (fsrootdir_entry->DIR_FstClusLO & 0xffff);
    FATDirectory* dir = new FATDirectory(myFS, clusterNum, namepcpy);
    dir->initialize(this->mycluster_num);

    this->add(dir);

    /* be sure data is written to device */
    this->myFS->myPartition->flushCache();

    return (dir);
}




/*****************************************************************************
 * Method: FATDirectory::initialize(unint4 parentCluster)
 *
 * @description
 *  Initializes a new fat directory with its . and .. entries.
 *******************************************************************************/
void FATDirectory::initialize(unint4 parentCluster) {
    /* get exclusive access to the FAT buffers */
    FATAccess->acquire();
    FAT32_DirEntry* fsrootdir_entry;
    fsrootdir_entry = resetToFirstEntry();

    memset(fsrootdir_entry, 0, sizeof(FAT32_DirEntry));
    memcpy(fsrootdir_entry->DIR_Name, ".          ", 11);
    fsrootdir_entry->DIR_FstClusHI = (unint2) (this->mycluster_num >> 16);
    fsrootdir_entry->DIR_FstClusLO = mycluster_num & 0xffff;
    fsrootdir_entry->DIR_Attr       = ATTR_DIRECTORY;

    fsrootdir_entry = getNextEntry(false,true);
    memset(fsrootdir_entry, 0, sizeof(FAT32_DirEntry));
    memcpy(fsrootdir_entry->DIR_Name,"..         ",11);
    fsrootdir_entry->DIR_FstClusHI = (unint2) (parentCluster >> 16);
    fsrootdir_entry->DIR_FstClusLO = parentCluster & 0xffff;
    fsrootdir_entry->DIR_Attr      = ATTR_DIRECTORY;

    /* create end entry */
    FAT32_DirEntry* next_entry = getNextEntry(false, true);
    memset(next_entry, 0, sizeof(FAT32_DirEntry));

    synchEntries();

    /* be sure data is really written back */
    myFS->myPartition->flushCache();

    FATAccess->release();
}


/*****************************************************************************
 * Method: FATDirectory::generateShortName(unsigned char* shortname, char* p_name)
 *
 * @description
 *  Generates the DOS short name for the given long name
 *******************************************************************************/
void FATDirectory::generateShortName(unsigned char* shortname, char* p_name) {
    int dotpos = -1;
    int len = 0;

    for (int i = 0; i < 255; i++) {
        if (p_name[i] == '.')
            dotpos = i;
        if (p_name[i] == 0) {
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

    if (dotpos == -1) {
        if (len > 10)
            len = 8;
        memcpy(shortname, p_name, len);
    } else {
        // get extension length
        unint1 ext_len = (unint1) (len - dotpos) - 1;
        if (ext_len > 3)
            ext_len = 3;
        // copy extension, shrink to 3 chars
        memcpy(&shortname[8], &p_name[dotpos + 1], ext_len);

        // copy name before extension
        // shrink name
        if (dotpos > 8)
            dotpos = 8;
        memcpy(shortname, p_name, dotpos);
    }

    // FATtiyfy the name
    // not completely correct... trailing spaces should be kept
    // intermediate spaces should be replaced...
    for (int i = 0; i < 11; i++) {
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

/*****************************************************************************
 * Method: FATDirectory::getNextEntry(bool writeBack, bool allocate)
 *
 * @description
 *  Returns the next directory entry
 *******************************************************************************/
FAT32_DirEntry* FATDirectory::getNextEntry(bool writeBack, bool allocate) {
    if (it_currentSectorEntry == -1) {
        return (resetToFirstEntry());
    }

    unint4 entries_per_sector   = myFS->myPartition->getSectorSize() / 32;
    FAT32_DirEntry* entries     = reinterpret_cast<FAT32_DirEntry*>(buffer);

    /* no more entries?*/
    if (it_currentSectorEntry == last_entry && it_currentSector == last_entry_sector) {
        if (allocate != true)
            return (0);

        /* allocate a new entry*/
        /* end of sector reached? */
        if ((unint4)(it_currentSectorEntry +1) >= entries_per_sector) {
            /* first write back the current sector */
            if (writeBack) {
                if (isError(myFS->myPartition->writeSectors(it_currentSector, buffer, 1))) {
                    return (0);
                }
            }

            /* load next sector, allocate if cluster ends */
            unint4 nextSector = this->myFS->getNextSector(it_currentSector, it_currentCluster);
            /* EOC can happen, allocate next chain cluster then */
            if (nextSector == EOC) {
                it_currentSector = this->myFS->getNextSector(it_currentSector, it_currentCluster, true);
                if (it_currentSector == EOC) {
                    LOG(FILESYSTEM, WARN, "FATDirectory::getNextEntry() No more memory for allocation. ");
                    return (0);
                }
                it_currentCluster = this->myFS->SectorToCluster(it_currentSector);
                /* clear directory table */
                memset(buffer, 0, 512);
            } else {
                it_currentSector = nextSector;
                if (isError(myFS->myPartition->readSectors(it_currentSector, buffer, 1))) {
                    return (0);
                }
            }

            last_entry_sector     = it_currentSector;
            it_currentSectorEntry = 0;
        } else {
            it_currentSectorEntry++;
        }

        last_entry = it_currentSectorEntry;
        /* return the next entry*/
        return (&entries[it_currentSectorEntry]);
    }

    it_currentSectorEntry++;
    if (it_currentSectorEntry >= entries_per_sector) {
        // first write back the sector
        if (writeBack) {
            if (isError(myFS->myPartition->writeSectors(it_currentSector, buffer, 1)))
                return (0);
        }

        // load next sector, no allocation here as it can not be the end
        it_currentSector = this->myFS->getNextSector(it_currentSector, it_currentCluster);
        // EOC must not happen
        if (it_currentSector == EOC) {
            LOG(FILESYSTEM, ERROR, "EOC encountered before end of directory.. Directory table corrupt?!. ");
            LOG(FILESYSTEM, ERROR, "it_currentSectorEntry=%u, last_entry =%u, it_currentSector=%u, last_entry_sector=%u", it_currentSectorEntry,last_entry,it_currentSector,last_entry_sector);
            return (0);
        } else {
            it_currentCluster = this->myFS->SectorToCluster(it_currentSector);
            if (isError(myFS->myPartition->readSectors(it_currentSector, buffer, 1)))
                return (0);
        }
        it_currentSectorEntry = 0;
    }

    return (&entries[it_currentSectorEntry]);
}

/*****************************************************************************
 * Method: FATDirectory::synchEntries()
 *
 * @description
 *  Ensures temporary directory entry changes are written back
 *  to the device
 *******************************************************************************/
void FATDirectory::synchEntries() {
    if (it_currentSector >= this->sector && it_currentSector <= this->last_entry_sector) {
        ErrorT status = myFS->myPartition->writeSectors(it_currentSector, buffer, 1);
        if (isError(status)) {
            LOG(FILESYSTEM, ERROR, " FATDirectory::synchEntries() writeSectors failed: %d", status);
        }
    } else {
        LOG(FILESYSTEM, ERROR, " FATDirectory::synchEntries() failed..");
    }
}

FAT32_DirEntry* FATDirectory::moveToEntry(unint2 entryInSector, unint2 entrysector) {
    unint4 entries_per_sector   = myFS->myPartition->getSectorSize() / 32;
    if (entryInSector >= entries_per_sector)
    {
        LOG(FILESYSTEM, ERROR, "FATDirectory::moveToEntry() invalid entry in sector: %u. entries per sector: %u", entryInSector, entries_per_sector);
        return (0);
    }
    if (entrysector >= myFS->myPartition->getNumSectors())
    {
        LOG(FILESYSTEM, ERROR, "FATDirectory::moveToEntry() invalid sector: %u. Last sector: %u", entrysector, myFS->myPartition->getNumSectors());
        return (0);
    }
    it_currentSectorEntry   = entryInSector;
    it_currentSector        = entrysector;
    it_currentCluster       = myFS->SectorToCluster(entrysector);
    myFS->myPartition->readSectors(entrysector, buffer, 1);
    FAT32_DirEntry* entries = reinterpret_cast<FAT32_DirEntry*>(buffer);
    return (&entries[it_currentSectorEntry]);
}

/*****************************************************************************
 * Method: FATDirectory::resetToFirstEntry()
 *
 * @description
 *
 *******************************************************************************/
FAT32_DirEntry* FATDirectory::resetToFirstEntry() {
    it_currentSectorEntry   = 0;
    it_currentSector        = sector;
    if (sector > myFS->myPartition->getNumSectors())
    {
        LOG(FILESYSTEM, ERROR, "FATDirectory::moveToEntry() invalid sector: %u", sector);
        return (0);
    }
    it_currentCluster       = myFS->SectorToCluster(sector);
    myFS->myPartition->readSectors(sector, buffer, 1);
    FAT32_DirEntry* entries = reinterpret_cast<FAT32_DirEntry*>(buffer);
    return (&entries[0]);
}



/*****************************************************************************
 * Method: FATDirectory::removeFileEntries(FATFile* file, bool freeClusterChain)
 *
 * @description
 *
 *******************************************************************************/
ErrorT FATDirectory::removeFileEntries(FATFile* file, bool freeClusterChain) {
    FAT32_DirEntry *fsrootdir_entry;
    FATAccess->acquire(this);

    if (file->longname_entry_number != -1) {
       fsrootdir_entry = moveToEntry(file->longname_entry_number, file->longname_entry_sector);

       /* iterate over all longname entries and the final short entry and mark them as free */
       do {
           if (!fsrootdir_entry)
           {
              FATAccess->release();
              LOG(FILESYSTEM, ERROR, " FATDirectory::removeFileEntries() File entry corrupt.. not deleting..");
              return (cError);
           }

           memset(fsrootdir_entry, 0, sizeof(FAT32_DirEntry));
           LOG(FILESYSTEM, INFO, " FATDirectory::removeFileEntries() invalidating longname entry %d in sector %d", it_currentSectorEntry, it_currentSector);
           fsrootdir_entry->DIR_Name[0] = 0xE5;
           fsrootdir_entry              = getNextEntry(true, false);
       } while (!(it_currentSector == file->directory_sector && it_currentSectorEntry == file->directory_entry_number));
    }

    /* be sure to move to short name entry anyway */
    fsrootdir_entry = moveToEntry(file->directory_entry_number, file->directory_sector);


    if (!fsrootdir_entry)
    {
       FATAccess->release();
       LOG(FILESYSTEM, ERROR, " FATDirectory::removeFileEntries() cannot move to directory entry..");
       return (cError);
    }

    LOG(FILESYSTEM, INFO, " FATDirectory::removeFileEntries() invalidating shortname entry %d in sector %d", it_currentSectorEntry, it_currentSector);

    /* remove short name entry */
    if (fsrootdir_entry) {
        /* this basically disallows undelete as all entry attributes are nulled */
        memset(fsrootdir_entry, 0, sizeof(FAT32_DirEntry));
        fsrootdir_entry->DIR_Name[0] = 0xE5;
    }

    /* check if the last entry was also the last entry inside the dir table*/
    FAT32_DirEntry *nextEntry = getNextEntry(true, false);
    if (!nextEntry) {
       /* reached the end */
        memset(fsrootdir_entry, 0, sizeof(FAT32_DirEntry));
    }

    /* be sure entries are all written back */
    synchEntries();

    if (freeClusterChain) {
        LOG(FILESYSTEM, DEBUG, " FATDirectory::removeFileEntries() Freeing cluster chain");
        /* now free up the FAT cluster chain entries used by this file */
        if (!myFS->freeClusterChain(file->clusterStart)) {
            LOG(FILESYSTEM, ERROR, "FATFile::freeClusterChain failed");
        }
    }

    // be sure operation is really written to device
    this->myFS->myPartition->flushCache();

    FATAccess->release();
    return (cOk);
}

/*****************************************************************************
 * Method: FATDirectory::remove(Resource *res)
 *
 * @description
 *  Removes the file from the FAT directory.
 *  Directly updates the partition on the device freeing
 *  all blocks used by the file.
 *******************************************************************************/
ErrorT FATDirectory::remove(Resource *res) {
    // TODO support directory removal
    /* Currently only removal of files supported */
    if (!(res->getType() & (cFile)))
        return (cWrongResourceType);

    FATFile* file = static_cast<FATFile*>(res);
    removeFileEntries(file, true);

    /* be sure data is really written back */
    myFS->myPartition->flushCache();

    /* finally remove internal dir object */
    ErrorT ret = Directory::remove(res);
    theOS->getMemoryManager()->scheduleDeletion(file);
    return (ret);
}

/*****************************************************************************
 * Method: FATDirectory::ioctl(int request, void* args)
 *
 * @description
 *******************************************************************************/
ErrorT FATDirectory::ioctl(int request, void* args) {
    return (cError);
}


