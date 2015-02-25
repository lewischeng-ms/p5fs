#include "inode.h"
#include "path.h"

// Make a new directory.
// Return 0 for success, -1 for failure(maybe existed).
int my_mkdir(const char * path)
{
	assert(path != NULL);

	int ret = last_dir(path);
	if (ret == 0)
	{
		printf("my_mkdir: dir or file %s already exists.\n", name_buf);
		return -1;
	}
	else if (ret == -1)
	{ // last dir not found, so create it.
		// Fill 'new_dir'.
		dir_t new_dir;
		memset(&new_dir, 0, DIR_BYTES);
		strcpy(new_dir.name, name_buf);
		new_dir.inode_nr = alloc_inode(&child, DIR_FILE);
		cinum = new_dir.inode_nr;
		if (new_dir.inode_nr == 0)
		{
			printf("my_mkdir: no space for new inode.\n");
			return -1;
		}
		
		// Add the entry to parent inode.
		if (add_dir_entry_to_parent(&new_dir) == -1)
		{
			free_inode(new_dir.inode_nr);
			printf("my_mkdir: no space for new dir entry.\n");
			return -1;
		}
		write_inode(pinum, &parent);

		return 0;
	}
	else if (ret == -2)
	{ // Some error occurred in path resolution.
		printf("my_mkdir: path resolution error.\n");
		return -1;
	}
	
	return -1;
}

// Remove an existed directory.
// Return 0 for success, -1 for failure(maybe not existed).
int my_rmdir(const char * path)
{
	assert(path != NULL);

	int ret = last_dir(path);
	
	if (ret == 0)
	{
		// Do necessary checks.
		if (child.mode != DIR_FILE)
		{
			printf("my_rmdir: %s is not a dir.\n", name_buf);
			return -1;
		}
		if (child.size > 0)
		{
			printf("my_rmdir: dir %s is not empty %d.\n", name_buf);
			return -1;
		}

		// Remove the entry for child inode from parent inode.
		ret = remove_dir_entry_from_parent();
		assert(ret == 0);
		write_inode(pinum, &parent);

		// Clear all contents in child and remove it.
		clear_data_zones(&child);
		free_inode(cinum);

		return 0;
	}
	else if (ret == -1)
	{ // last dir not found error.
		printf("my_rmdir: dir %s not found.\n", name_buf);
		return -1;
	}
	else if (ret == -2)
	{ // Some error occurred in path resolution.
		printf("my_rmdir: path resolution error.\n");
		return -1;
	}

	return -1;
}

// Remove an existed file.
// Return 0 for success, -1 for failure(maybe not existed).
int my_remove(const char * path)
{
	assert(path != NULL);

	int ret = last_dir(path);
	if (ret == 0)
	{
		// Do necessary checks.
		if (child.mode == DIR_FILE)
		{
			printf("my_remove: %s is a dir.\n", name_buf);
			return -1;
		}

		// Remove the entry for child inode from parent inode.
		ret = remove_dir_entry_from_parent();
		assert(ret == 0);
		write_inode(pinum, &parent);

		// Clear all contents in child and remove it.
		clear_data_zones(&child);
		free_inode(cinum);
		
		return 0;
	}
	else if (ret == -1)
	{ // last dir not found error.
		printf("my_remove: file %s not found.\n", name_buf);
		return -1;
	}
	else if (ret == -2)
	{ // Some error occurred in path resolution.
		printf("my_remove: path resolution error.\n");
		return -1;
	}

	return -1;
}

// Rename a file or dir.
// Return 0 for success, -1 for failure(maybe not existed).
int my_rename(const char * old, const char * new)
{
	assert(old != NULL && new != NULL);

	int ret = last_dir(old);
	if (ret == 0)
	{
		// Backup the inode of which the file to be moved.
		inodesn_t bak = cinum;

		// Remove the entry for child inode from parent inode.
		ret = remove_dir_entry_from_parent();
		assert(ret == 0);
		write_inode(pinum, &parent);

		// Add it to 'new'.
		ret = last_dir(new);
		if (ret == 0)
		{
			printf("my_rename: dir or file %s already exists.\n", name_buf);
			return -1;
		}
		else if (ret == -1)
		{
			// Fill 'new_dir'.
			dir_t new_dir;
			memset(&new_dir, 0, DIR_BYTES);
			strcpy(new_dir.name, name_buf);
			new_dir.inode_nr = bak;

			// Add the entry to parent inode.
			if (add_dir_entry_to_parent(&new_dir) == -1)
			{
				printf("my_rename: no space for new dir entry.\n");
				return -1;
			}
			write_inode(pinum, &parent);

			return 0;
		}
		else if (ret == -2)
		{ // Some error occurred in path resolution.
			printf("my_rename: path resolution error.\n");
			return -1;
		}

		return -1;
	}
	else if (ret == -1)
	{ // last dir not found error.
		printf("my_rename: dir %s not found.\n", name_buf);
		return -1;
	}
	else if (ret == -2)
	{ // Some error occurred in path resolution.
		printf("my_rename: path resolution error.\n");
		return -1;
	}
	
	return -1;
}
