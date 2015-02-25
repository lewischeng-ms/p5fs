#include "inode.h"
#include "zone.h"

// Super block of the device. Also used as buf for super block.
super_t super_block;

// Buffer for block.
static char block_buf[BLOCKSIZE];

// Fill the variable super_block.
static void fill_super_block()
{
	// Fields that don't need calculation.
	super_block.imap_blocks = IMAP_BLOCKS;
	super_block.zmap_blocks = ZMAP_BLOCKS;
	super_block.firstdatazone = FIRSTDATAZONE;
	super_block.log_zone_size = LOG_ZONE_SIZE;
	super_block.version = FS_VER;
	super_block.inodes_per_block = INODES_PER_BLOCK;
	super_block.first_imap_block = FIRST_IMAP_BLOCK;
	super_block.first_zmap_block = FIRST_ZMAP_BLOCK;
	super_block.first_inode_block = FIRST_INODE_BLOCK;
	super_block.zones = ZONES;

	// 'ninodes' means how many entries in the inode table.
	// So it equals capacity of the inode table timed by inodes per block.
	// We know the zone map stays just after the inode table so...
	super_block.ninodes = (FIRST_ZMAP_BLOCK - FIRST_INODE_BLOCK) * INODES_PER_BLOCK;

	// 'dzmap_size' means how many blocks data zone occupies.
	super_block.dzmap_size = ZONES * (1 << LOG_ZONE_SIZE);
}

// Make a new and clean file system on the simulated device.
void my_mkfs()
{
	// Device is not available.
	if (dev_open() == -1)
	{
		printf("Error while opening the device.\n");
		return;
	}

	// Fill the super block and write it to the device.
	fill_super_block();
	memcpy(block_buf, &super_block, sizeof(super_t));
	write_block(0, block_buf);

	// Init maps.
	init_imap();
	init_zmap();

	// Mark the first inode for the root directory in the imap.
	clear_inodes();

	// Clear the data zone.
	clear_zones();

	// Build the first inode for the root directory.
	inode_t root_inode;
	inodesn_t result = alloc_inode(&root_inode, DIR_FILE);

	// # of root dir must be 1.
	assert(result == 1);
}

// Print the disk layout of the file system.
void print_disk_layout()
{
	printf("\nbasics:\n");
	printf("1 block = %d bytes.\n", BLOCKSIZE);
	printf("1 inode = %d bytes.\n", I_NODE_BYTES);
	printf("1 zone = %d blocks.\n", 1 << super_block.log_zone_size);
	printf("\ndisk layout:\n");
	printf("block[0]: super block.\n");
	printf("block[%d]...block[%d]: inode bitmap, total count of bits = %d.\n",
			super_block.first_imap_block, super_block.first_imap_block + super_block.imap_blocks - 1,
			super_block.imap_blocks * BIT_PER_BLOCK);
	printf("block[%d]...block[%d]: inode table, total count of inodes = %d.\n",
			super_block.first_inode_block, super_block.first_zmap_block - 1,
			super_block.ninodes);
	printf("block[%d]...block[%d]: zone bitmap, total count of bits = %d.\n",
			super_block.first_zmap_block, super_block.first_zmap_block + super_block.zmap_blocks - 1,
			super_block.zmap_blocks * BIT_PER_BLOCK);
	printf("block[%d]...block[%d]: data zone, total count of zones = %d.\n",
			super_block.firstdatazone, super_block.firstdatazone + super_block.zones - 1,
			super_block.zones);
	printf("\nusage:\n");
	printf("inode occupancy = %d, free = %d.\n", count_occupied_inodes(), count_free_inodes());
	printf("zone occupancy = %d, free = %d.\n", count_occupied_zones(), count_free_zones());
}
