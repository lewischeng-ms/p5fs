#ifndef P5FS_INODE_H_
#define P5FS_INODE_H_

#include <string.h>
#include <stdio.h>
#include <time.h>
#include "bitmap.h"

#define INODE_INIT_UID 0 // Change this to geteuid() in *nix.
#define INODE_INIT_GID 0 // Change this to getegid() in *nix.

// Initialize the inode map.
void init_imap();

// Apply for an inode for a new file or directory.
// Return the # of inode, 0 for failure.
inodesn_t alloc_inode(inode_t * pinode, u16_t mode);

// Free the inode at the specified number.
// The function won't be checked with imap when not debugging.
void free_inode(inodesn_t num);

// Read the inode to device at the specified number.
// The function won't be checked with imap when not debugging.
void read_inode(inodesn_t num, inode_t * pinode);

// Write the inode to device at the specified number.
// The function won't be checked with imap when not debugging.
void write_inode(inodesn_t num, inode_t * pinode);

// Clear all inodes.
void clear_inodes();

// Count occupied inodes.
inodesn_t count_occupied_inodes();

// Count free inodes.
inodesn_t count_free_inodes();

#endif // P5FS_INODE_H_
