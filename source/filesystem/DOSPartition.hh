/*
 * DOSPartition.hh
 *
 *  Created on: 19.06.2013
 *      Author: dbaldin
 */

#ifndef DOSPARTITION_HH_
#define DOSPARTITION_HH_

#include "Partition.hh"

#define DOS_PART_DISKSIG_OFFSET	0x1b8U
#define DOS_PART_TBL_OFFSET	0x1beU
#define DOS_PART_MAGIC_OFFSET	0x1feU
#define DOS_PBR_FSTYPE_OFFSET	0x36U
#define DOS_PBR32_FSTYPE_OFFSET	0x52U
#define DOS_PBR_MEDIA_TYPE_OFFSET	0x15U
#define DOS_MBR	0
#define DOS_PBR	1

typedef struct dos_partition {
    unsigned char boot_ind; /* 0x80 - active			*/
    unsigned char head; /* starting head			*/
    unsigned char sector; /* starting sector			*/
    unsigned char cyl; /* starting cylinder		*/
    unsigned char sys_ind; /* What partition type		*/
    unsigned char end_head; /* end head					*/
    unsigned char end_sector; /* end sector				*/
    unsigned char end_cyl; /* end cylinder				*/
    unsigned char start4[4]; /* starting sector counting from 0	*/
    unsigned char size4[4]; /* nr of sectors in partition		*/
} dos_partition_t;

/* Convert char[4] in little endian format to the host format integer
 */
static inline int le32_to_int(unsigned char *le32) {
    return ((le32[3] << 24) + (le32[2] << 16) + (le32[1] << 8) + le32[0]);
}

class DOSPartition: public Partition {

private:
    // my partition table
    dos_partition_t mypartition;

public:
    DOSPartition(BlockDeviceDriver *bdev, dos_partition_t *mypartition, unint1 part_num, unint4 disksig);

    virtual ~DOSPartition();

};

#endif /* DOSPARTITION_HH_ */
