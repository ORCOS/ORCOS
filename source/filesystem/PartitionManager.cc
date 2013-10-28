/*
 * PartitionManager.cc
 *
 *  Created on: 19.06.2013
 *      Author: dbaldin
 */

#include "PartitionManager.hh"
#include "inc/memtools.hh"

#include "kernel/Kernel.hh"
#include "filesystem/DOSPartition.hh"
#include "filesystem/FATFileSystem.hh"

extern Kernel* theOS;

#define DOS_PART_DISKSIG_OFFSET	0x1b8
#define DOS_PART_TBL_OFFSET	0x1be
#define DOS_PART_MAGIC_OFFSET	0x1fe
#define DOS_PBR_FSTYPE_OFFSET	0x36
#define DOS_PBR32_FSTYPE_OFFSET	0x52
#define DOS_PBR_MEDIA_TYPE_OFFSET	0x15
#define DOS_MBR	0
#define DOS_PBR	1

static unsigned char buffer[1024] __attribute__((aligned(4)));

static inline int is_extended(int part_type)
{
    return (part_type == 0x5 ||
	    part_type == 0xf ||
	    part_type == 0x85);
}


static char EFI_Signature[8] = {0x45,0x46,0x49,0x20,0x50,0x41,0x52,0x54};

typedef struct {
	char 	signature[8];
	unint4 	revision;
	unint4	header_size_le;
	unint4  header_crc32;
	unint4  reserved;
	unint8  header_lba;
	unint8  backup_lba;
	unint8  partitions_start_lba;	// first lba used for partitions
	unint8	partitions_end_lba;
	char	disk_GUID[16];
	unint8	pt_lba;					// start of the partition table
	unint4	num_pte;				// number of pt entries
	unint4	pte_size;
	unint4	pt_crc32;
} EFI_PT_Header;


typedef struct {
	unint1 bootable;
	char   chs_first[3];
	unint1 type;
	char   chs_last[3];
	unint4 lba_start;
	unint4 lba_size;
} PT_Entry;


ErrorT PartitionManager::handleEFIPartitionTable(BlockDeviceDriver* bdev) {

	int error = bdev->readBlock(1,(char*)buffer,1);
	if (error < 0) return cError;

	EFI_PT_Header* efi_header = (EFI_PT_Header*) buffer;

	if (memcmp(efi_header->signature,EFI_Signature,8) != 0) {
		LOG(ARCH,INFO,(ARCH,INFO,"PartitionManager: EFI Signature validation failed."));
		return cError;
	}

	LOG(ARCH,INFO,(ARCH,INFO,"PartitionManager: Found EFI Header."));
	LOG(ARCH,INFO,(ARCH,INFO,"PartitionManager: EFI Partitions: %d.",efi_header->num_pte));

	if (efi_header->num_pte > 2) {
		LOG(ARCH,INFO,(ARCH,INFO,"PartitionManager: No support for more than 2 EFI partitions."));
		return cError;
	}

	LOG(ARCH,INFO,(ARCH,INFO,"PartitionManager: EFI Support missing."));

	return cOk;
}

ErrorT PartitionManager::tryDOSMBR(BlockDeviceDriver* bdev) {

	memset(buffer,0,1024);
	// read first sector
	int error = bdev->readBlock(0,(char*)buffer,1);
	if (error < 0) return cError;

	if((buffer[DOS_PART_MAGIC_OFFSET + 0] != 0x55) || (buffer[DOS_PART_MAGIC_OFFSET + 1] != 0xaa) ) {
			return (cError);
		} /* no DOS Signature at all */

	int disksig = le32_to_int(&buffer[DOS_PART_DISKSIG_OFFSET]);
	LOG(ARCH,INFO,(ARCH,INFO,"PartitionManager: Disk Signature: %x",disksig));

	if (memcmp((char *)&buffer[DOS_PBR_FSTYPE_OFFSET],"FAT",3)==0 ||
		memcmp((char *)&buffer[DOS_PBR32_FSTYPE_OFFSET],"FAT32",5)==0) {

		// handle partition table .. mount every partition if possible
		PT_Entry *pt_entry = (PT_Entry*) (buffer + DOS_PART_TBL_OFFSET);
		for (int i = 0; i < 4; i++, pt_entry++) {

			switch(pt_entry->type) {
				case 0: break;

				case 6: // FAT 16
				case 12 : // FAT 32
				case 13 : // FAT 32 (LBA)
				case 14 : // FAT 16 (LBA)
					{
					Partition* partition = new Partition(bdev,"PT",pt_entry->lba_start,pt_entry->lba_size,i);
					this->add(partition);

					// FAT Filesystem
					FileSystemBase* fs = new FATFileSystem(partition);
					if (fs->isValidFileSystem()) fs->initialize();

					break;
				}
				case 0xee: {
					// handle efi partition tables here
					// EFI Partition mit Legacy MBR
					return handleEFIPartitionTable(bdev);
					break;
				}
				default: {
					LOG(ARCH,WARN,(ARCH,WARN,"PartitionManager: No Partition support for Type: %x",pt_entry->type));
				}

			}
		}


		return cOk; /* is PBR */
	}

	dos_partition_t *pt;

	int part_num = 1;

	/* Print all primary/logical partitions */
	pt = (dos_partition_t *) (buffer + DOS_PART_TBL_OFFSET);
	for (int i = 0; i < 4; i++, pt++) {
		/*
		 * fdisk does not show the extended partitions that
		 * are not in the MBR
		 */

		if ((pt->sys_ind != 0)) /*&&
			(!is_extended (pt->sys_ind)) ) */{

			switch (pt->sys_ind) {
				case 6: // FAT 16
				case 12 :
				case 13 : // FAT 32 (LBA)
				case 14 : // FAT 16 (LBA)
					{
					Partition* partition = new DOSPartition(bdev,pt,part_num,disksig);
					this->add(partition);	    /* Is MBR */

					// FAT Filesystem
					FileSystemBase* fs = new FATFileSystem(partition);
					if (fs->isValidFileSystem()) fs->initialize();

					break;
				}
				case 0xee: {
					// handle efi partition tables here
					// EFI Partition mit Legacy MBR
					return handleEFIPartitionTable(bdev);
					break;
				}
				default: {
					LOG(ARCH,WARN,(ARCH,WARN,"PartitionManager: No FileSystem Support for SYS_IND: %x",pt->sys_ind));
				}
			}

		}

		/* Reverse engr the fdisk part# assignment rule! */
		if ((pt->sys_ind != 0 && !is_extended (pt->sys_ind)) ) {
			part_num++;
		}
	}


	return cOk;
}

PartitionManager::PartitionManager() : Directory("partitions") {
	// make ourself visible inside the filesystem
	theOS->getFileManager()->registerResource(this);

}

void PartitionManager::registerBlockDevice(BlockDeviceDriver* bdev) {

	// try and find the correct partition type of the device
	if (tryDOSMBR(bdev) == cOk) return;

	// TODO: try to mount directly as filesystem as no MBR might be present
	LOG(ARCH,WARN,(ARCH,WARN,"PartitionManager: No supported partition/MBR found for '%s'",bdev->getName()));

}

void PartitionManager::unregisterBlockDevice(BlockDeviceDriver* bdev) {

	LinkedListDatabaseItem* litem = this->dir_content.getHead();


	// search all partitions for this device
	// as the device may have multiple partitions we cant stop on the first found one
	while (litem != 0) {

		Partition* p = (Partition*) litem->getData();
		if (p->myBlockDevice == bdev) {

			// trigger filesystem deletion
			delete p->mountedFileSystem;

			litem = litem->getSucc();
			dir_content.remove(p);
			delete p;

		} else {
			litem = litem->getSucc();
		}
	}

}


PartitionManager::~PartitionManager() {
	// delete all filesystems
	// however the partition manager should not be deleted
}
