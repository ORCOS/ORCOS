/*
 * PartitionManager.cc
 *
 *  Created on: 19.06.2013
 *    Copyright &  Author: dbaldin
 */

#include "PartitionManager.hh"
#include "inc/memtools.hh"
#include "inc/endian.h"
#include "kernel/Kernel.hh"
#include "filesystem/DOSPartition.hh"
#include "filesystem/fat/FATFileSystem.hh"

extern Kernel* theOS;

static char buffer[1024] __attribute__((aligned(4)));

/*****************************************************************************
 * Method: is_extended(int part_type)
 *
 * @description
 *
 *******************************************************************************/
static inline int is_extended(int part_type) {
    return (part_type == 0x5 || part_type == 0xf || part_type == 0x85);
}

static char EFI_Signature[8] = { 0x45, 0x46, 0x49, 0x20, 0x50, 0x41, 0x52, 0x54 };

typedef struct {
    char    signature[8];
    unint4  revision;
    unint4  header_size_le;
    unint4  header_crc32;
    unint4  reserved;
    unint8  header_lba;
    unint8  backup_lba;
    unint8  partitions_start_lba;    // first lba used for partitions
    unint8  partitions_end_lba;
    char    disk_GUID[16];
    unint8  pt_lba;                 // start of the partition table
    unint4  num_pte;                // number of pt entries
    unint4  pte_size;
    unint4  pt_crc32;
}__attribute__((packed)) EFI_PT_Header;

typedef struct {
    unint1  bootable;
    char    chs_first[3];
    unint1  type;
    char    chs_last[3];
    unint4  lba_start;
    unint4  lba_size;
}__attribute__((packed)) PT_Entry;

/*****************************************************************************
 * Method: PartitionManager::handleEFIPartitionTable(BlockDeviceDriver* bdev)
 *
 * @description
 *
 *******************************************************************************/
ErrorT PartitionManager::handleEFIPartitionTable(BlockDeviceDriver* bdev) {
    int error = bdev->readBlock(1, buffer, 1);
    if (error < 0) {
        return (cError);
    }

    EFI_PT_Header* efi_header = reinterpret_cast<EFI_PT_Header*>(buffer);

    if (memcmp(efi_header->signature, EFI_Signature, 8) != 0) {
        LOG(FILESYSTEM, INFO, "PartitionManager: EFI Signature validation failed.");
        return (cError);
    }

    LOG(FILESYSTEM, INFO, "PartitionManager: Found EFI Header.");
    LOG(FILESYSTEM, INFO, "PartitionManager: EFI Partitions: %d.", efi_header->num_pte);

    if (efi_header->num_pte > 2) {
        LOG(FILESYSTEM, INFO, "PartitionManager: No support for more than 2 EFI partitions.");
        return (cError);
    }

    LOG(FILESYSTEM, INFO, "PartitionManager: EFI Support missing.");

    return (cOk);
}

/*****************************************************************************
 * Method: PartitionManager::tryDOSMBR(BlockDeviceDriver* bdev)
 *
 * @description
 *
 *******************************************************************************/
ErrorT PartitionManager::tryMBR(BlockDeviceDriver* bdev) {
    if (bdev->sector_size > 1024) {
        LOG(FILESYSTEM, ERROR, "PartitionManager: Sector size %d > 1024 unsupported", bdev->sector_size);
        return (cError);
    }

    memset(buffer, 0xde, sizeof(buffer));

    /* read first sector */
    int error = bdev->readBlock(0, buffer, 1);
    if (error < 0) {
        LOG(FILESYSTEM, ERROR, "PartitionManager: Could not read first block. Error %d", error);
        return (cError);
    }

    if ((buffer[DOS_PART_MAGIC_OFFSET + 0] != 0x55) || (buffer[DOS_PART_MAGIC_OFFSET + 1] != 0xaa)) {
        LOG(FILESYSTEM, INFO, "PartitionManager: No MBR found");
        return (cError );
    } /* no DOS Signature at all */

    MBRType mbrType = STANDARD_MBR;
    int table_offset = DOS_PART_TBL_OFFSET;
    int tableEntries = 4;
    if (buffer[428] == 0x78 && buffer[429] == 0x56) {
        mbrType = AAP_MBR;
        table_offset = 0x1be;
    }
    if (buffer[380] == 0x5a && buffer[381] == 0xa5) {
        mbrType = ASTNEC_MBR;
        table_offset = 0x17e;
        tableEntries = 8;
    }
    if (buffer[252] == 0xaa && buffer[253] == 0x55) {
        mbrType = DISKMANAGER_MBR;
        table_offset = 0xfe;
        tableEntries = 16;
    }
    int disksig = 0;

    if (mbrType == STANDARD_MBR) {
        disksig = le32_to_int(&buffer[DOS_PART_DISKSIG_OFFSET]);
        LOG(FILESYSTEM, INFO, "PartitionManager: Disk Signature: %x", disksig);
    }

    int ret = cError;



    if (memcmp(&buffer[DOS_PBR_FSTYPE_OFFSET], "FAT", 3) == 0 ||
        memcmp(&buffer[DOS_PBR32_FSTYPE_OFFSET], "FAT32", 5) == 0) {
        // handle partition table .. mount every partition if possible
        PT_Entry *pt_entry = reinterpret_cast<PT_Entry*>(buffer + table_offset);
        for (int i = 0; i < tableEntries; i++, pt_entry++) {
            switch (pt_entry->type) {
                case 0:
                    break;

    #if HAS_FileSystems_FATFileSystemCfd
                case 6:  // FAT 16
                case 12:  // FAT 32
                case 13:  // FAT 32 (LBA)
                case 14:  // FAT 16 (LBA)
                {
                    static char tmpname[100];
                    char* name = new char[40];
                    name[39] = 0;
                    sprintf(tmpname, "<unnamed>@%s", bdev->getName());
                    strncpy(name, tmpname, 40);
                    Partition* partition = new Partition(bdev, name, pt_entry->lba_start, pt_entry->lba_size, i);
                    this->add(partition);

                    // FAT Filesystem
                    FileSystemBase* fs = new FATFileSystem(partition);
                    if (fs->isValidFileSystem())
                        fs->initialize();

                    ret = cOk;
                    break;
                }
    #endif
                case 0xee: {
                    // handle efi partition tables here
                    // EFI Partition with Legacy MBR
                    return (handleEFIPartitionTable(bdev));
                }
                default: {
                    LOG(FILESYSTEM, WARN, "PartitionManager: No Partition support for Type: 0x%x lba: 0x%x sectors:0x%x", pt_entry->type, pt_entry->lba_start,  pt_entry->lba_size);
                }
            } // switch
        }

        return (ret); /* is PBR */
    }

    dos_partition_t *pt;
    unint1 part_num = 1;

    /* Print all primary/logical partitions */
    pt = reinterpret_cast<dos_partition_t *> (buffer + DOS_PART_TBL_OFFSET);
    for (int i = 0; i < 4; i++, pt++) {
        if ((pt->sys_ind != 0)) {
            switch (pt->sys_ind) {
#if HAS_FileSystems_FATFileSystemCfd
                case 6:  // FAT 16
                case 0xb:
                case 0xc:  // FAT 32 (LBA)
                case 0xe:  // FAT 16 (LBA)
                {
                    Partition* partition = new DOSPartition(bdev, pt, part_num, disksig);
                    this->add(partition); /* Is MBR */
                    // FAT Filesystem
                    FileSystemBase* fs = new FATFileSystem(partition);
                    if (fs->isValidFileSystem())
                        fs->initialize();
                    break;
                }
#endif
                case 0xee: {
                    // handle efi partition tables here
                    // EFI Partition mit Legacy MBR
                    return (handleEFIPartitionTable(bdev));
                }
                default: {
                    LOG(FILESYSTEM, WARN, "PartitionManager: No FileSystem Support for SYS_IND: 0x%x", pt->sys_ind);
                }
            } // switch
        }

        /* Reverse engr the fdisk part# assignment rule! */
        if ((pt->sys_ind != 0 && !is_extended(pt->sys_ind))) {
            part_num++;
        }
    }

    return (cOk );
}

PartitionManager::PartitionManager() :
        Directory("partitions") {
    // make ourself visible inside the filesystem
    theOS->getFileManager()->registerResource(this);
}

/*****************************************************************************
 * Method: PartitionManager::registerBlockDevice(BlockDeviceDriver* bdev)
 *
 * @description
 *
 *******************************************************************************/
void PartitionManager::registerBlockDevice(BlockDeviceDriver* bdev) {
    // try and find the correct partition type of the device
    if (tryMBR(bdev) == cOk)
        return;
    /* No MBR? .. try mounting the partition directly using the whole block device */

    static char tmpname[100];
    char* name = new char[40];
    name[39]   = 0;
    sprintf(tmpname, "<unnamed>@%s", bdev->getName());
    strncpy(name, tmpname, 40);

    Partition* partition = new Partition(bdev, name, 0, -1, 0);
    if (FATFileSystem::isFATFileSystem(partition)) {
        /* FAT FS found.. probably windows created FS.
         * Windows does not create a MBR. It just plain initializes the FAT FS
         * on the first sector of the device..*/
        FileSystemBase* fs = new FATFileSystem(partition);
        if (fs->isValidFileSystem()) {
            this->add(partition);
            fs->initialize();
        }
        return;
    }

    /* No Filesystem found.. well just drop partition..
     * we may directly delete here as no thread may have
     * accessed the partition yet*/
    delete partition;

    // TODO: try to mount directly as filesystem as no MBR might be present
    LOG(FILESYSTEM, WARN, "PartitionManager: No supported partition/MBR found for '%s'", bdev->getName());
}

/*****************************************************************************
 * Method: PartitionManager::unregisterBlockDevice(BlockDeviceDriver* bdev)
 *
 * @description
 *
 *******************************************************************************/
void PartitionManager::unregisterBlockDevice(BlockDeviceDriver* bdev) {
    LinkedListItem* litem = this->dir_content.getHead();

    /* search all partitions for this device
     * as the device may have multiple partitions we cant stop on the first found one */
    while (litem != 0) {
        Partition* p = reinterpret_cast<Partition*>(litem->getData());
        if (p->myBlockDevice == bdev) {
            litem = litem->getSucc();
            dir_content.remove(p);
            /* schedule deletion. will also delete the mounted filesystem */
            theOS->getMemoryManager()->scheduleDeletion(p);
        } else {
            litem = litem->getSucc();
        }
    }
}

PartitionManager::~PartitionManager() {
    // delete all filesystems
    // however the partition manager should not be deleted
}

