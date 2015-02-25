#ifndef P5FS_DZ_H_
#define P5FS_DZ_H_

#include <assert.h>
#include "p5.h"
#include "zone.h"

#define U32_PER_BLOCK (BLOCKSIZE / 4)

enum ITR_PLACES {
	IP_DIRECT,
	IP_IND1,
	IP_IND2,
	IP_NIL
};

typedef struct {
	// The inode which owns the data zone.
	inode_t * pinode;

	// Which place the zone is indexed.
	// Direct zones in inode? Ind1? Ind2?
	int place;

	// Number of zone which is between 1 and filesize / zonesize.
	u32_t num;

	// Total count of zones.
	u32_t count;

	// The index of zone in direct zones or ind1 zone.
	u32_t index;

	// The index of zone in ind2 zone.
	u32_t index2;

	// The current zone #.
	blockn_t znum;

	// Available bytes in the current zone.
	u32_t asize;

	// Ind1 zone buffer.
	u32_t zbuf1[U32_PER_BLOCK];
	blockn_t znum1;

	// Ind2 zone buffer.
	u32_t zbuf2[U32_PER_BLOCK];
	blockn_t znum2;
} dz_iterator_t;

// Load iterator with info from inode.
void dzi_make_iterator(inode_t * pinode, dz_iterator_t * dzi);

// Move the iterator to the zone at the specified number.
// num can be 1 to count + 1.
// Return 0 for success and -1 for failure.
int dzi_move_to(dz_iterator_t * pdzi, u32_t num);

// Modify the iterator pointing to a new zone
// and modify the inode or its referencing data zone.
// This function will write back related zone(s) if modified.
void dzi_redirect(dz_iterator_t * pdzi, u32_t new_num);

// Clear all data zones owned by the inode.
void clear_data_zones(inode_t * pinode);

#endif // P5FS_DZ_H_
