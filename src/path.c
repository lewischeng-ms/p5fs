#include "path.h"

// inode for parent and child.
inodesn_t pinum;
inode_t parent;
inodesn_t cinum;
inode_t child;

char name_buf[MAX_FILE_NAME_LENGTH];

// Iterator used to search dir.
dz_iterator_t dzi;

// The block owned by parent that contains child.
dir_t dir_entry_buf[DIR_PER_BLOCK];
blockn_t dir_entry_zone_num;
// 'dir_entry_index' indicates where child is in the entries.
u32_t dir_entry_index;
// 'dir_entry_count' indicates how many effective entries in the block.
u32_t dir_entry_count;

// Load zone #num to dir_entry_buf. 'size' indicates the acutal size
// data occupies.
static inline void load_dir_entry_buf(u32_t num, u32_t size)
{
	assert(num > 0);
	assert(size % DIR_BYTES == 0);
	dir_entry_count = size / DIR_BYTES;
	dir_entry_zone_num = num;
	read_zone(dir_entry_zone_num, (char *)dir_entry_buf);
}

// Search dir named 'buf' in the dir entry buf.
static inline void search_dir_entry()
{
	for (dir_entry_index = 0; dir_entry_index < dir_entry_count; dir_entry_index++)
	{
		if (strcmp(dir_entry_buf[dir_entry_index].name, name_buf) == 0)
			break;
	}
}

// Find dir_t named 'buf' in the parent inode, and store corresponding inode
// to child.
// Return 0 for found, -1 for not found.
static int find_dir_entry_in_parent()
{
	dzi_make_iterator(&parent, &dzi);
	if (dzi.count == 0)
		return -1;
	else
		dzi_move_to(&dzi, 1);
	while (1)
	{
		load_dir_entry_buf(dzi.znum, dzi.asize);
		search_dir_entry();
		if (dir_entry_index < dir_entry_count)
		{
			cinum = dir_entry_buf[dir_entry_index].inode_nr;
			read_inode(cinum, &child);
			return 0;
		}

		// Move to next zone.
		if (dzi.num < dzi.count)
			dzi_move_to(&dzi, dzi.num + 1);
		else
			break;
	}
	return -1;
}

// Get the last dir of the path, its inode(if exists) and its parent dir's inode.
// Return 0 for found.
// Return -1 for last dir not found.
// Return -2 for error about middle dirs.
int last_dir(const char * path)
{
	assert(path != NULL);

	// Process root.
	if (*path++ != '/')
		return -2;
	pinum = 1;
	read_inode(1, &parent);
	assert(parent.mode == DIR_FILE);

	while (1)
	{
		u32_t count = 0;
		while (*path != '\0' && *path != '/' && count < MAX_FILE_NAME_LENGTH - 1)
			name_buf[count++] = *path++;
		name_buf[count] = '\0';

		// Expect dir name to contain at least one charater.
		if (count == 0) return -2;

		// Find the dir named 'buf' in parent.
		int ret = find_dir_entry_in_parent();

		if (*path != '\0')
		{ // Expect moving to next component.
			if (ret == -1 || child.mode != DIR_FILE || *path++ != '/')
				return -2;
			parent = child;
			pinum = cinum;
		}
		else
		{ // It's the last dir.
			if (ret == -1)
				return -1;
			else
				return 0;
		}
	}

	// Won't be here for good.
	return -1;
}

// Add a new entry to parent inode.
// Return 0 for success, -1 for failure.
int add_dir_entry_to_parent(dir_t * pdir)
{
	assert(pdir != NULL);

	dzi_make_iterator(&parent, &dzi);
	if (dzi.count > 0)
	{
		dzi_move_to(&dzi, dzi.count);
		load_dir_entry_buf(dzi.znum, dzi.asize);
		if (dir_entry_count < DIR_PER_BLOCK)
		{ // The last zone is not full.
			dir_entry_buf[dir_entry_count] = *pdir;
			write_zone(dir_entry_zone_num, (char *)dir_entry_buf);
			parent.size += DIR_BYTES;

			return 0;
		}
	}

	// Need to create a new zone and write to it.
	dzi_move_to(&dzi, dzi.count + 1);
	if (dzi.place == IP_NIL) return -1;
	if (dzi.znum == 0)
	{
		u32_t new_zone = alloc_zone();
		if (new_zone == 0) return -1;
		dzi_redirect(&dzi, new_zone);
		load_dir_entry_buf(new_zone, BLOCKSIZE);
	}
	else
	{
		load_dir_entry_buf(dzi.znum, BLOCKSIZE);
	}
	dir_entry_buf[0] = *pdir;
	write_zone(dir_entry_zone_num, (char *)dir_entry_buf);
	parent.size += DIR_BYTES;

	return 0;
}

// Remove the entry for child from parent.
// Return 0 for success, -1 for failure.
int remove_dir_entry_from_parent()
{
	u32_t old_num = dir_entry_zone_num;
	u32_t old_cnt = dir_entry_count;
	u32_t old_idx = dir_entry_index;

	dzi_make_iterator(&parent, &dzi);
	if (dzi.count == 0) return -1;

	dzi_move_to(&dzi, dzi.count);
	if (old_num == dzi.znum)
	{ // Only one zone.
		if (old_idx == old_cnt - 1)
		{ // The last dir_t is the one to be overwrited.
			// Need to do nothing.
		}
		else
		{ // Use last entry to overwrite it.
			dir_entry_buf[old_idx] = dir_entry_buf[old_cnt - 1];
		}
	}
	else
	{ // The last dir_t is in another zone.
		load_dir_entry_buf(dzi.znum, dzi.asize);
		dir_t last_dir = dir_entry_buf[dir_entry_count - 1];
		load_dir_entry_buf(old_num, BLOCKSIZE);
		dir_entry_buf[old_idx] = last_dir;
	}

	write_zone(old_num, (char *)dir_entry_buf);
	parent.size -= DIR_BYTES;

	return 0;
}
