#ifndef P5FS_PATH_H_
#define P5FS_PATH_H_

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "dz.h"

// inode for parent and child.
extern inodesn_t pinum;
extern inode_t parent;
extern inodesn_t cinum;
extern inode_t child;

extern char name_buf[MAX_FILE_NAME_LENGTH];

// Get the last dir of the path, its inode(if exists) and its parent dir's inode.
// Return 0 for found.
// Return -1 for last dir not found.
// Return -2 for error about middle dirs.
int last_dir(const char * path);

// Add a new entry to parent inode.
// Return 0 for success, -1 for failure.
int add_dir_entry_to_parent(dir_t * pdir);

// Remove the entry for child from parent.
// Return 0 for success, -1 for failure.
int remove_dir_entry_from_parent();

#endif // P5FS_PATH_H_
