/*
 * FATFileSystem.cc
 *
 *  Created on: 19.06.2013
 *      Author: dbaldin
 */

#include "FATFileSystem.hh"
#include "inc/memtools.hh"

#include "kernel/Kernel.hh"

extern Kernel* theOS;

static char buffer[1024];

FATFileSystem::FATFileSystem(Partition* myPartition) : FileSystemBase(myPartition) {

	rootDir = 0;
	myPartition->setMountedFileSystem(this);

	// read out first sector
	int error = myPartition->readSectors(0,buffer,1);
	if (error < 0) return;

	memcpy(&this->myFAT_BPB,buffer,sizeof(FAT_BS_BPB));


	if ((myFAT_BPB.BS_jmpBoot[0] == 0xeb) || (myFAT_BPB.BS_jmpBoot[0] == 0xe9)) {

		if (myFAT_BPB.BPB_BytsPerSec <= myPartition->getSectorSize() ) {

			// right now we trust the rest of the table and do checks on demand
			FAT32_BPB* myFAT32BPB = (FAT32_BPB*) &buffer[36];
			FAT16_BPB* myFAT16BPB = (FAT16_BPB*) &buffer[36];


			RootDirSectors = (((myFAT_BPB.BPB_RootEntCnt * 32) + (myFAT_BPB.BPB_BytsPerSec - 1)) / myFAT_BPB.BPB_BytsPerSec);

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


			FirstDataSector = myFAT_BPB.BPB_RsvdSecCnt + (myFAT_BPB.BPB_NumFATs * FATSz) + RootDirSectors;
			LOG(ARCH,INFO,(ARCH,INFO,"FATFileSystem: First FAT Data Sector: %d",FirstDataSector));


			DataSec = TotSec - (myFAT_BPB.BPB_RsvdSecCnt + (myFAT_BPB.BPB_NumFATs * FATSz) + RootDirSectors);
			CountOfClusters = DataSec / myFAT_BPB.BPB_SecPerClus;
			LOG(ARCH,INFO,(ARCH,INFO,"FATFileSystem: Clusters: %d, DataSectors: %d",CountOfClusters,DataSec));

			if (CountOfClusters < 4085) {
				this->myFatType = FAT12;
				LOG(ARCH,WARN,(ARCH,WARN,"FATFileSystem: FAT12 not supported.. not mounting FS"));
			} else if (CountOfClusters < 65525 ) {
				this->myFatType = FAT16;
				this->isValid = true;
				memcpy(&myFATxx_BPB,&buffer[36],sizeof(FAT16_BPB));
				LOG(ARCH,INFO,(ARCH,INFO,"FATFileSystem: found valid FAT16 with %d sectors",myFAT_BPB.BPB_TotSec16));
			}
			else {
				this->myFatType = FAT32;
				this->isValid = true;
				memcpy(&myFATxx_BPB,&buffer[36],sizeof(FAT32_BPB));
				LOG(ARCH,INFO,(ARCH,INFO,"FATFileSystem: found valid FAT32 with %d sectors",myFAT_BPB.BPB_TotSec32));
			}


		} else {
			LOG(ARCH,ERROR,(ARCH,ERROR,"FATFileSystem: invalid Sector Size: %d",myFAT_BPB.BPB_BytsPerSec));
		}
	} else {
		LOG(ARCH,ERROR,(ARCH,ERROR,"FATFileSystem: invalid Filesystem. Invalid BS_jmpBoot field"));
	}


}


unint4 FATFileSystem::getFATTableEntry(unint4 clusterNum) {
	unint4 addr;
	if (this->myFatType == FAT16)
		addr = clusterNum * 2;
	else
		addr = clusterNum * 4;

	// get table entry offset in partition sector
	unint4 fatentoffset = addr % myFAT_BPB.BPB_BytsPerSec;

	// get the partition sector containing the table entry
	unint4 fatsecnum = myFAT_BPB.BPB_RsvdSecCnt + (addr / myFAT_BPB.BPB_BytsPerSec);

	// read the sector
	int error = myPartition->readSectors(fatsecnum,buffer,1);
	if (error < 0) return cError;

	// now get the table entry
	// this is 4 bytes aligned.. however comiler does not know this
	// thus this will produce a warning
	if (myFatType == FAT16)
		return  (* ((unint2*)  &buffer[fatentoffset]));
	else
		return  (* ((unint4*)  &buffer[fatentoffset])) & 0x0FFFFFFF;

}


unint4 FATFileSystem::getNextSector(unint4 currentSector, unint4 &currentCluster) {

	/*if (this->myFatType == FAT16) {
		if (currentCluster == 0) {
			if (currentSector == myFAT_BPB.BPB_RootEntCnt) return EOC;
		}
	}*/

	// check if next sector would lie inside another cluster
	// if so follow cluster chain
	if ( (currentSector / myFAT_BPB.BPB_SecPerClus) != ((currentSector+1) / myFAT_BPB.BPB_SecPerClus)) {

		// next sector is in another clusters
		// lookup the FAT table to find the successor cluster
		unint4 nextClusterNum = getFATTableEntry(currentCluster);
		if (nextClusterNum != EOC) {
			currentCluster = nextClusterNum;
			return ClusterToSector(nextClusterNum);
		} else {
			currentCluster = EOC;
			return EOC;
		}


	} else {
		return currentSector+1;
	}

}


static int FAT_SHORTNAME_STRIP(char* name, unint1 startpos) {

	for (int i = startpos; i > 0; i--) {
		if (name[i] == ' ')
			name[i] = 0;
		else return (i);
	}

	// should not happen as this sould be a filename violation
	// however, be safe here
	return 0;


}

FATFileSystem::~FATFileSystem() {
	// get orcos mount directory
	Directory* mntdir = theOS->getFileManager()->getDirectory("/mnt/");

	if (mntdir != 0 && rootDir != 0) {

		// remove root directory to system
		mntdir->remove(rootDir);

		delete rootDir;
	}

}

ErrorT FATFileSystem::initialize() {

	if (this->isValid) {

		// get orcos mount directory
		Directory* mntdir = theOS->getFileManager()->getDirectory("/mnt/");

		if (myFatType == FAT32) {

			// get FSInfo
			unint4 fsinfo_lba = myFATxx_BPB.myFAT32_BPB.BPB_FSInfo * ( myFAT_BPB.BPB_BytsPerSec / this->myPartition->getSectorSize());
			int error = myPartition->readSectors(fsinfo_lba,buffer,1);
			if (error < 0) return cError;

			FAT32_FSInfo *fsinfo = (FAT32_FSInfo *) &buffer[0];
			if (fsinfo->FSI_LeadSig != 0x41615252) {
				LOG(ARCH,ERROR,(ARCH,ERROR,"FATFileSystem: FS_INFO Lead Sig Validation failed: %x != 0x41615252",fsinfo->FSI_LeadSig));
			}
			if (fsinfo->FSI_StrucSig != 0x61417272) {
				LOG(ARCH,ERROR,(ARCH,ERROR,"FATFileSystem: FS_INFO Struc Sig Validation failed: %x != 0x61417272",fsinfo->FSI_StrucSig));
			}

			LOG(ARCH,INFO,(ARCH,INFO,"FATFileSystem: Free Clusters: %x",fsinfo->FSI_FreeCount));


			if (mntdir == 0) {
				LOG(ARCH,ERROR,(ARCH,ERROR,"FATFileSystem: no mount directory '/mnt/' found. Not mounting"));
				return cError;
			}

			char *name = (char*) theOS->getMemManager()->alloc(12);
			name[11] = 0;
			memcpy(name,myFATxx_BPB.myFAT32_BPB.BS_VolLab,11);

			FAT_SHORTNAME_STRIP(name,10);

			LOG(ARCH,INFO,(ARCH,INFO,"FATFileSystem: Volume: '%s' ID: %x",name,myFATxx_BPB.myFAT32_BPB.BS_VolID));

			// add root directory to system
			rootDir = new FATDirectory(this,myFATxx_BPB.myFAT32_BPB.BPB_RootClus,name);

			mntdir->add(rootDir);

			LOG(ARCH,INFO,(ARCH,INFO,"FATFileSystem: FAT Filesystem mounted to '/mnt/%s/'.",name));

			// go on and mount this fs
			return cOk;
		} else {
			// FAT 16

			char *name = (char*) theOS->getMemManager()->alloc(12);
			name[11] = 0;
			memcpy(name,myFATxx_BPB.myFAT16_BPB.BS_VolLab,11);

			FAT_SHORTNAME_STRIP(name,10);

			LOG(ARCH,INFO,(ARCH,INFO,"FATFileSystem: Volume: '%s' ID: %x",name,myFATxx_BPB.myFAT16_BPB.BS_VolID));

			unint4 FirstRootDirSecNum = myFAT_BPB.BPB_RsvdSecCnt + (myFAT_BPB.BPB_NumFATs * myFAT_BPB.BPB_FATSz16);
			rootDir = new FATDirectory(this,0,name,FirstRootDirSecNum);
			mntdir->add(rootDir);

			LOG(ARCH,INFO,(ARCH,INFO,"FATFileSystem: FAT Filesystem mounted to '/mnt/%s/'.",name));

			// go on and mount this fs
			return cOk;

		}

	}

	return cError;

}

FATDirectory::FATDirectory(FATFileSystem* parentFileSystem, unint4 cluster_num,
		const char* name) : Directory(name) {

	this->myFS 			= parentFileSystem;
	this->mycluster_num = cluster_num;
	this->populated 	= false;
	this->sector 		= myFS->ClusterToSector(mycluster_num);
}

FATDirectory::FATDirectory(FATFileSystem* parentFileSystem, unint4 cluster_num,
		const char* name, unint4 sector) : Directory(name) {

	this->myFS 			= parentFileSystem;
	this->mycluster_num = cluster_num;
	this->populated 	= false;
	this->sector 		= sector;
}


ErrorT FATDirectory::readBytes(char* bytes, unint4& length) {

	if (!populated) this->populateDirectory();
	return Directory::readBytes(bytes,length);
}

void FATDirectory::populateDirectory() {


	unint4 currentCluster = mycluster_num;
	// cluster number to sector number conversion
	// get root sector number
	unint4 sector_number = this->sector;

	int error = myFS->myPartition->readSectors(sector_number,buffer,1);
	if (error < 0) return;


	FAT32_DirEntry *fsrootdir_entry = (FAT32_DirEntry *) &buffer[0];
	// check if this is the dedicated volume entry

	unint4 entries_per_sector = myFS->myPartition->getSectorSize() / 32;
	unint4 num = 0;


	while(fsrootdir_entry->DIR_Name[0] != 0) {
		FAT32_LongDirEntry* fslongentry = (FAT32_LongDirEntry*) fsrootdir_entry;

		// check if this is not a free entry
		if (fsrootdir_entry->DIR_Name[0] != 0xE5) {

			if  ((fslongentry->LDIR_Attr & ATTR_LONG_NAME_MASK) == ATTR_LONG_NAME)
			{
				/*   Found an active long name sub-component.   */
			} else {
				// we have some short entry
				if (fsrootdir_entry->DIR_Attr != ATTR_VOLUME_ID) {

					unint4 clusterid = (fsrootdir_entry->DIR_FstClusHI << 16) | fsrootdir_entry->DIR_FstClusLO;

					char* name = (char*) theOS->getMemManager()->alloc(12);
					name[11] = 0;
					memcpy(name,fsrootdir_entry->DIR_Name,11);

					int lastpos = FAT_SHORTNAME_STRIP(name,7);
					lastpos++;

					if ((fsrootdir_entry->DIR_Attr & ATTR_DIRECTORY) != 0) {
						FAT_SHORTNAME_STRIP(name,10);
						FATDirectory *fatDir = new FATDirectory(this->myFS,clusterid,name);
						this->add(fatDir);
					} else {
						// stupid file extension handling for fat names
						char ext[3];
						ext[0] = name[8];
						ext[1] = name[9];
						ext[2] = name[10];
						if (ext[0] != ' ') {
							name[lastpos] = '.';
							name[lastpos+1] = ext[0];
							name[lastpos+2] = ext[1];
							name[lastpos+3] = ext[2];
							name[lastpos+4] = 0;
						} else name[lastpos] = 0;

						FATFile *file = new FATFile(name,fsrootdir_entry,myFS);
						this->add(file);

					}


					LOG(ARCH,TRACE,(ARCH,TRACE,"'%s'\t%x\t%d\t%d",name,fsrootdir_entry->DIR_Attr,clusterid,fsrootdir_entry->DIR_FileSize));

				} // if not ATTR_VOLUME_ID
			} // if not long name entry
		} // if valid entry

		fsrootdir_entry++;
		num++;
		if (num >= entries_per_sector) {
			// load next sector
			sector_number = this->myFS->getNextSector(sector_number,currentCluster);
			if (sector_number == EOC) break;

			error = myFS->myPartition->readSectors(sector_number,buffer,1);
			if (error < 0) return;
			num = 0;
			fsrootdir_entry = (FAT32_DirEntry *) &buffer[0];
		}

	}

	this->populated = true;

}

Resource* FATDirectory::get(const char* name) {
	// if this directory is already populated forward to base class
	if (!populated) populateDirectory();


	//TODO: we might return a single resource without populating the whole directory
	// just lookup the entries on the block devices partition and create a resource for it.

	return Directory::get(name);


}

FATDirectory::~FATDirectory() {


}



FATFile::FATFile(char* name, FAT32_DirEntry* entry, FATFileSystem* fs) : File(name,entry->DIR_FileSize,entry->DIR_Attr) {

	this->clusterStart = ((entry->DIR_FstClusHI << 16) | entry->DIR_FstClusLO);
	this->currentCluster = clusterStart;
	this->currentSector = fs->ClusterToSector(clusterStart);
	this->myFS = fs;
}

FATFile::~FATFile() {
	// check if anybody is holding a handle to this..

}

ErrorT FATFile::readBytes(char* bytes, unint4& length) {

	unint4 sector_pos;
	unint4 sector_read_len;
	unint4 pos = 0;

	unint4 readlength 	= length;
	// be sure we are not reading over the end of the file
	if ((this->filesize - this->position) < readlength) readlength =  (this->filesize - this->position);

	unint4 sector_size 	= this->myFS->myPartition->getSectorSize();
	bool sector_changed = false;

	while (readlength > 0) {

		// position inside our current sector
		sector_pos 		= this->position % sector_size;
		// set readlength to remaining bytes in this sector
		sector_read_len = sector_size - sector_pos;

		// check if we read less than the remaining bytes in this sector
		if (readlength < sector_read_len) sector_read_len = readlength;
		else {
			sector_changed = true;
		}

		// read the sector
		int error = myFS->myPartition->readSectors(currentSector,buffer,1);
		if (error < 0) return cError;

		// copy the desired bytes in this sector into the buffer
		memcpy(&bytes[pos],&buffer[sector_pos],sector_read_len);
		// increase position by bytes read
		this->position += sector_read_len;
		pos += sector_read_len;

		// check if we reached the sector boundary
		if (sector_changed) {
			currentSector = myFS->getNextSector(currentSector,currentCluster);
			if (currentCluster == EOC) {
				// end of file reached
				length = pos;
				return cOk;
			}
		}

		sector_changed = false;
		// decrease total amount to read by read amount of bytes
		readlength-= sector_read_len;

	}

	length = pos;
	return cOk;

}

ErrorT FATFile::resetPosition() {
	this->currentCluster = clusterStart;
	this->currentSector = myFS->ClusterToSector(clusterStart);
	this->position = 0;
	return cOk;
}
