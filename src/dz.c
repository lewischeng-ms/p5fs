#include "dz.h"

// Load iterator with info from inode.
void dzi_make_iterator(inode_t * pinode, dz_iterator_t * pdzi)
{
	assert(pinode != NULL);
	assert(pdzi != NULL);

	pdzi->pinode = pinode;
	pdzi->place = IP_DIRECT;
	pdzi->count = pinode->size / BLOCKSIZE;
	if (pinode->size > 0) pdzi->count++;
	pdzi->num = 0;
	pdzi->index = 0;
	pdzi->index2 = 0;
	pdzi->znum = 0;
	pdzi->znum1 = 0;
	pdzi->znum2 = 0;
	pdzi->asize = 0;
}

static inline void read_at_znum1(dz_iterator_t * pdzi, u32_t new_blk1)
{
	if (pdzi->znum1 != new_blk1)
		read_zone(new_blk1, (char *)pdzi->zbuf1);
	pdzi->znum1 = new_blk1;
}

static inline void read_at_znum2(dz_iterator_t * pdzi, u32_t new_blk2)
{
	if (pdzi->znum2 != new_blk2)
		read_zone(new_blk2, (char *)pdzi->zbuf2);
	pdzi->znum2 = new_blk2;
}

// Move the iterator to the zone at the specified number.
// num can be 1 to count + 1.
// Return 0 for success and -1 for failure.
int dzi_move_to(dz_iterator_t * pdzi, u32_t num)
{
	assert(pdzi != NULL);
	assert(num > 0);

	if (num > pdzi->count + 1) return -1;
	
	inode_t * pinode = pdzi->pinode;
	pdzi->num = num;

	// Calculate 'asize'.
	pdzi->asize = (pdzi->num == pdzi->count ? (pinode->size % BLOCKSIZE) : BLOCKSIZE);

	// Calculate fields in iterator.
	u32_t temp;
	if (num <= NR_DIRECT_ZONES)
	{ // Just need direct zones in inode.
		pdzi->place = IP_DIRECT;
		pdzi->index = num - 1;
		pdzi->znum = pinode->direct[pdzi->index];
	}
	else if (num <= NR_DIRECT_ZONES + NR_IND1_ZONES)
	{ // Need ind1.
		pdzi->place = IP_IND1;
		pdzi->index = num - NR_DIRECT_ZONES - 1;
		if (pinode->ind1 == 0)
		{ // ind1 zone not found, automatically build it.
			temp = alloc_zone();
			if (temp == 0) return -1;
			pinode->ind1 = temp;
		}
		read_at_znum1(pdzi, pinode->ind1);
		if (pdzi->zbuf1[pdzi->index] == 0)
			return -1;
		pdzi->znum = pdzi->zbuf1[pdzi->index];
	}
	else if (num <= NR_DIRECT_ZONES + NR_IND1_ZONES + NR_IND2_ZONES)
	{ // Need ind2.
		//printf("***************************\n");
		pdzi->place = IP_IND2;
		u32_t zones_still_need = num - NR_DIRECT_ZONES - NR_IND1_ZONES;
		pdzi->index2 = zones_still_need / NR_IND1_ZONES;
		pdzi->index = zones_still_need % NR_IND1_ZONES - 1;
		if (pinode->ind2 == 0)
		{ // ind2 zone not found, automatically build it.
			temp = alloc_zone();
			if (temp == 0) return -1;
			pinode->ind2 = temp;
		}
		read_at_znum2(pdzi, pinode->ind2);
		if (pdzi->zbuf2[pdzi->index2] == 0)
		{ // ind1 zone in ind2 zone not found, automatically build it.
			temp = alloc_zone();
			if (temp == 0) return -1;
			pdzi->zbuf2[pdzi->index2] = temp;
			write_zone(pdzi->znum2, (char *)pdzi->zbuf2);
		}
		read_at_znum1(pdzi, pdzi->zbuf2[pdzi->index2]);
		if (pdzi->zbuf1[pdzi->index] == 0)
			return -1;
		pdzi->znum = pdzi->zbuf1[pdzi->index];
	}
	else
	{ // Need ind3 or more, not supported!.
		pdzi->place = IP_NIL;
		return -1;
	}
	return 0;
}

// Modify the iterator pointing to a new zone
// and modify the inode or its referencing data zone.
// This function will write back ind1 zone if needed.
void dzi_redirect(dz_iterator_t * pdzi, u32_t new_num)
{
	switch (pdzi->place)
	{
	case IP_NIL:
		return;
	case IP_DIRECT:
		pdzi->pinode->direct[pdzi->index] = new_num;
		break;
	case IP_IND1:
	case IP_IND2:
		pdzi->zbuf1[pdzi->index] = new_num;
		write_zone(pdzi->znum1, (char *)pdzi->zbuf1);
		break;
	}
	pdzi->znum = new_num;
}

// Clear all data zones owned by the inode.
void clear_data_zones(inode_t * pinode)
{
	assert(pinode != NULL);

	static u32_t temp[U32_PER_BLOCK];
	static u32_t temp1[U32_PER_BLOCK];

	// Free direct zones.
	u32_t i;
	for (i = 0; i < NR_DIRECT_ZONES; i++)
	{
		if (pinode->direct[i] == 0)
			return;
		free_zone(pinode->direct[i]);
	}

	// Free ind1.
	if (pinode->ind1 == 0)
		return;
	read_zone(pinode->ind1, (char *)temp);
	for (i = 0; i < U32_PER_BLOCK; i++)
	{
		if (temp[i] == 0)
			break;
		free_zone(temp[i]);
	}
	free_zone(pinode->ind1);

	// Free ind2.
	if (pinode->ind2 == 0)
		return;
	read_zone(pinode->ind2, (char *)temp);
	for (i = 0; i < U32_PER_BLOCK; i++)
	{
		if (temp[i] == 0)
			break;
		read_zone(temp[i], (char *)temp1);
		u32_t j;
		int end = 0;
		for (j = 0; j < U32_PER_BLOCK; j++)
		{
			if (temp1[j] == 0)
			{
				end = 1;
				break;
			}
			free_zone(temp1[j]);
		}
		free_zone(temp[i]);
		if (end == 1)
			break;
	}
	free_zone(pinode->ind2);
}
