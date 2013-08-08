/*
 * DOSPartition.cc
 *
 *  Created on: 19.06.2013
 *      Author: dbaldin
 */

#include "DOSPartition.hh"
#include "kernel/Kernel.hh"
extern Kernel* theOS;


static inline int is_extended(int part_type)
{
    return (part_type == 0x5 ||
	    part_type == 0xf ||
	    part_type == 0x85);
}

static inline int is_bootable(dos_partition_t *p)
{
	return p->boot_ind == 0x80;
}

static void print_one_part(dos_partition_t *p, int ext_part_sector,
			   int part_num, unsigned int disksig)
{
	int lba_start = ext_part_sector + le32_to_int (p->start4);
	int lba_size  = le32_to_int (p->size4);

	printf("%3d\t%-10d\t%-10d\t%08x-%02x\t%02x%s%s\r",
		part_num, lba_start, lba_size, disksig, part_num, p->sys_ind,
		(is_extended(p->sys_ind) ? " Extd" : ""),
		(is_bootable(p) ? " Boot" : ""));
}


DOSPartition::DOSPartition(BlockDeviceDriver *bdev, dos_partition_t *mypartition, unint1 part_num, unint4 disksig) :
	Partition(bdev,"")
{
	memcpy((void*) &this->mypartition, (void*) mypartition, sizeof(dos_partition_t));

	char* myname = (char*) theOS->getMemManager()->alloc(9);
	sprintf(myname,"DOS_%04x",disksig);

	this->name = myname;
	this->partition_number = part_num;
	this->sectors 	= le32_to_int (mypartition->size4);
	this->lba_start = le32_to_int (mypartition->start4);

	printf("%s:\r",myname);
	printf("Part\tStart Sector\tNum Sectors\tUUID\t\tType\r\n");
	print_one_part(mypartition, 0, part_num, disksig);
}

DOSPartition::~DOSPartition() {

	// this->name is deleted by Resource
	//delete this->name;

}

