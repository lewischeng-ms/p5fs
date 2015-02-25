#include "inode.h"

extern super_t super_block;

// inode bitmap.
static bitmap_t imap;

// Block buffer for current operation.
static char block_buf[BLOCKSIZE];

// Remember the first free inode index in imap.
static u32_t first_zero_index;

// Initialize the inode map.
void init_imap()
{
	create_bitmap(&imap, super_block.first_imap_block, super_block.imap_blocks);
	first_zero_index = find_first_zero(&imap, 0, imap.bit_count);
}

// Make a new inode.
static void make_inode(inode_t * pinode, u16_t mode)
{
	if (pinode == NULL) return;

	pinode->mode = mode;
	pinode->links = 0;
	pinode->uid = INODE_INIT_UID;
	pinode->gid = INODE_INIT_GID;
	pinode->size = 0;

	time_t now = time(NULL);
	pinode->build_time = now;
	pinode->alter_time = now;
	pinode->sta_time = now;

	// Initially no zones are occupied.
	int i;
	for (i = 0; i < NR_DIRECT_ZONES; i++)
		pinode->direct[i] = 0;
	pinode->ind1 = 0;
	pinode->ind2 = 0;
	pinode->ind3 = 0;
}

// Apply for an inode for a new file or directory.
// Return the # of inode, 0 for failure.
inodesn_t alloc_inode(inode_t * pinode, u16_t mode)
{
	inodesn_t index;
	
	if (first_zero_index == imap.bit_count)
	{
		index = find_first_zero(&imap, 0, imap.bit_count);
		if (index == imap.bit_count) return 0;
	}
	else
	{
		index = first_zero_index;
		first_zero_index = imap.bit_count;
	}
	set_bit(&imap, index, 1);

	u32_t num = index + 1;
	make_inode(pinode, mode);
	write_inode(num, pinode);

	return num;
}

// Free the inode at the specified number.
// The function won't be checked with imap when not debugging.
void free_inode(inodesn_t num)
{
	assert(num >= 1 && num <= super_block.ninodes);

	u32_t index = num - 1;
	assert(get_bit(&imap, index) == 1);

	if (num == 1) printf("warning: you are freeing inode for root.\n");
	if (index < first_zero_index) first_zero_index = index;
	set_bit(&imap, index, 0);
}

// Read the inode to device at the specified number.
// The function won't be checked with imap when not debugging.
void read_inode(inodesn_t num, inode_t * pinode)
{
	assert(pinode != NULL);
	assert(num >= 1 && num <= super_block.ninodes);

	u32_t index = num - 1;
	assert(get_bit(&imap, index) == 1);

	blockn_t current_block = index / super_block.inodes_per_block + super_block.first_inode_block;
	cached_read_block(current_block, block_buf);

	inodesn_t block_offset = (index % super_block.inodes_per_block) * I_NODE_BYTES;
	memcpy(pinode, block_buf + block_offset, I_NODE_BYTES);
}

// Write the inode to device at the specified number.
// The function won't be checked with imap when not debugging.
void write_inode(inodesn_t num, inode_t * pinode)
{
	assert(pinode != NULL);
	assert(num >= 1 && num <= super_block.ninodes);

	u32_t index = num - 1;
	assert(get_bit(&imap, index) == 1);

	blockn_t current_block = index / super_block.inodes_per_block + super_block.first_inode_block;
	cached_read_block(current_block, block_buf);

	inodesn_t block_offset = (index % super_block.inodes_per_block) * I_NODE_BYTES;
	memcpy(block_buf + block_offset, pinode, I_NODE_BYTES);
	cached_write_block(current_block, block_buf);
}

// Clear all inodes including inode for root.
// This function is called in mkfs.
void clear_inodes()
{
	clear_bits(&imap);
	first_zero_index = 0;
}

// Count occupied inodes.
inodesn_t count_occupied_inodes()
{
	return count_ones(&imap, 0, super_block.ninodes);
}

// Count free inodes.
inodesn_t count_free_inodes()
{
	return count_zeros(&imap, 0, super_block.ninodes);
}
