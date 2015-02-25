#include "cache.h"

#define CACHED_BLOCKS 8191

// The pool for caching blocks.
static struct {
	int dirty;
	u32_t num;
	char block[BLOCKSIZE];
} pool[CACHED_BLOCKS];

int cached_read_block(int block_num, char * block)
{
	int hash = block_num % CACHED_BLOCKS;
	if (pool[hash].num != block_num)
	{
		if (pool[hash].dirty == 1)
			write_block(pool[hash].num, pool[hash].block);
		pool[hash].num = block_num;
		read_block(block_num, pool[hash].block);
		pool[hash].dirty = 0;
	}
	memcpy(block, pool[hash].block, BLOCKSIZE);
	return 0;
}

int cached_write_block(int block_num, char * block)
{
	int hash = block_num % CACHED_BLOCKS;
	if (pool[hash].num != block_num)
		return write_block(block_num, block);
	memcpy(pool[hash].block, block, BLOCKSIZE);
	pool[hash].dirty = 1;
	return 0;
}
