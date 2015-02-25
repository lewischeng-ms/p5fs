#include "p5.h"
#include "path.h"
#include "dz.h"

// Data zone iterator for each opened file.
dz_iterator_t dzis[MAX_OPEN_FILES];

static inline int get_min_fd()
{
	int fd;
	for (fd = 0; fd < MAX_OPEN_FILES; fd++)
		if (fp[fd].used == 0)
			return fd;
	return -1;
}

static int open_child(char * caller)
{
	int fd = get_min_fd();
	if (fd == -1)
	{
		printf("%s: the count of opened files reaches the max %d.\n", caller, MAX_OPEN_FILES);
		return -1;
	}

	fp[fd].used = 1;
	fp[fd].offset = 0;
	
	fp[fd].inode_nr = cinum;
	read_inode(cinum, &fp[fd].inode);
	dzi_make_iterator(&fp[fd].inode, &dzis[fd]);

	if (fp[fd].inode.size > 0)
	{
		dzi_move_to(&dzis[fd], 1);
		fp[fd].block_nr = dzis[fd].znum;
		read_zone(dzis[fd].znum, fp[fd].buf);
	}
	else
	{
		fp[fd].block_nr = 0;
	}

	return fd;
}

// Open a file and return its file descriptor.
// Return fd > 0 for success, -1 for failure(maybe not existed).
int my_open(const char * path)
{
	assert(path != NULL);

	int ret = last_dir(path);
	
	if (ret == 0)
	{
		// Do necessary checks.
		if (child.mode == DIR_FILE)
		{
			printf("my_open: %s is a dir.\n", name_buf);
			return -1;
		}

		return open_child("my_open");
	}
	else if (ret == -1)
	{ // last dir not found error.
		printf("my_open: file %s not found.\n", name_buf);
		return -1;
	}
	else if (ret == -2)
	{ // Some error occurred in path resolution.
		printf("my_open: path resolution error.\n");
		return -1;
	}

	return -1;
}

// Create a file and return its file descriptor.
// Return fd > 0 for success, -1 for failure(creation failed).
int my_creat(const char * path)
{
	assert(path != NULL);

	int ret = last_dir(path);
	if (ret == 0)
	{
		printf("my_creat: dir or file %s already exists.\n", name_buf);
		return -1;
	}
	else if (ret == -1)
	{ // last dir not found, so create it.
		// Fill 'new_dir'.
		dir_t new_dir;
		memset(&new_dir, 0, DIR_BYTES);
		strcpy(new_dir.name, name_buf);
		new_dir.inode_nr = alloc_inode(&child, NORMAL_FILE);
		cinum = new_dir.inode_nr; 
		if (new_dir.inode_nr == 0)
		{
			printf("my_creat: no space for new inode.\n");
			return -1;
		}

		// Add the entry to parent inode.
		if (add_dir_entry_to_parent(&new_dir) == -1)
		{
			free_inode(new_dir.inode_nr);
			printf("my_creat: no space for new dir entry.\n");
			return -1;
		}
		write_inode(pinum, &parent);

		return open_child("my_creat");
	}
	else if (ret == -2)
	{ // Some error occurred in path resolution.
		printf("my_creat: path resolution error.\n");
		return -1;
	}
	
	return -1;
}

// Read bytes of 'count' to 'buf' from file 'fd'.
// Return bytes actually read for success, -1 for failure.
int my_read(int fd, void * buf, int count)
{
	assert(fd >= 0 && fd < MAX_OPEN_FILES);
	assert(buf != NULL);
	assert(count >= 0);
	
	// Check if the file's been opened.
	if (fp[fd].used == 0)
	{
		printf("my_read: read from an unopened file.\n");
		return -1;
	}

	// No need to read.
	if (count == 0) return 0;

	// Already at end.
	if (fp[fd].offset == fp[fd].inode.size) return -1;

	int acount = fp[fd].inode.size - fp[fd].offset;
	if (count < acount) acount = count;
	int remain = acount;

	fp[fd].inode.sta_time = time(NULL);
	
	// Copy the remaining of the current block to buf.
	int cpcnt;
	int zoff = fp[fd].offset % BLOCKSIZE;
	cpcnt = (acount < (BLOCKSIZE - zoff) ? acount : (BLOCKSIZE - zoff));
	memcpy(buf, fp[fd].buf + zoff, cpcnt);
	remain -= cpcnt;
	buf = (void *)((char *)buf + cpcnt);

	// Still data to read?
	while (remain > 0)
	{
		dzi_move_to(&dzis[fd], dzis[fd].num + 1);
		fp[fd].block_nr = dzis[fd].znum;
		read_zone(fp[fd].block_nr, fp[fd].buf);
		cpcnt = (remain < BLOCKSIZE ? remain : BLOCKSIZE);
		memcpy(buf, fp[fd].buf, cpcnt);
		remain -= cpcnt;
		buf = (void *)((char *)buf + cpcnt);
	}

	assert(remain == 0);
	fp[fd].offset += acount;

	return acount;
}

// Write bytes of 'count' from 'buf' to file 'fd'.
// Return bytes actually written for success, -1 for failure.
int my_write(int fd, const void * buf, int count)
{
	assert(fd >= 0 && fd < MAX_OPEN_FILES);
	assert(buf != NULL);
	assert(count >= 0);

	// Check if the file's been opened.
	if (fp[fd].used == 0)
	{
		printf("my_write: write to an unopened file.\n");
		return -1;
	}

	// No need to write.
	if (count == 0) return 0;

	time_t now = time(NULL);
	fp[fd].inode.alter_time = now;
	fp[fd].inode.sta_time = now;

	int remain = count;
	  
	int cpcnt;

	// Copy buf to the remaining of the current block.
	if (fp[fd].block_nr > 0)
	{
		u32_t zoff = fp[fd].offset % BLOCKSIZE;
		cpcnt = (count < (BLOCKSIZE - zoff) ? count : (BLOCKSIZE - zoff));
		memcpy(fp[fd].buf + zoff, buf, cpcnt);
		write_zone(fp[fd].block_nr, fp[fd].buf);
		remain -= cpcnt;
		buf = (void *)((char *)buf + cpcnt);
		fp[fd].offset += cpcnt;
	}
	
	// Still data to write?
	while (remain > 0)
	{
		assert(dzis[fd].num <= dzis[fd].count);
		
		if (dzis[fd].num == dzis[fd].count)
		{
			u32_t new_zone = alloc_zone();
			if (new_zone == 0)
			{
				printf("my_write: no space on disk.\n");
				return count - remain;
			}
			dzi_move_to(&dzis[fd], dzis[fd].count + 1);
			if (dzis[fd].place == IP_NIL)
			{
				free_zone(new_zone);
				printf("my_write: reach max size of a file.\n");
				return count - remain;
			}
			
			dzi_redirect(&dzis[fd], new_zone);
			dzis[fd].count++;
		}
		else
		{
			dzi_move_to(&dzis[fd], dzis[fd].num + 1);
		}
		
		fp[fd].block_nr = dzis[fd].znum;
		cpcnt = (remain < BLOCKSIZE ? remain : BLOCKSIZE);
		write_zone(fp[fd].block_nr, (char *)buf);
		buf = (void *)((char *)buf + cpcnt);
		remain -= cpcnt;
		fp[fd].offset += cpcnt;
	}

	read_zone(fp[fd].block_nr, fp[fd].buf);

	assert(remain == 0);
	fp[fd].inode.size += count;

	return count;
}

// Close a file descriptor.
// Return 0 for success, -1 for failure(closing failed).
int my_close(int fd)
{
	// Write back inode to update filesize, timestamp...
	write_inode(fp[fd].inode_nr, &fp[fd].inode);

	// Release the file descripter.
	if (fp[fd].used == 0)
		return -1;
	fp[fd].used = 0;
	return 0;
}
