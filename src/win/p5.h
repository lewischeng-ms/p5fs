
#ifndef P5_H
#define P5_H

#include <sys/types.h>
#define MAX_FILE_NAME_LENGTH 62
#define MAX_OPEN_FILES 10
#define FS_VER	11
#define MAX_FILE_NUM 5000
#define I_NODE_BYTES 64
#define DIR_BYTES 64
#define DIR_PER_BLOCK 16
#define NR_DIRECT_ZONES 7
#define NR_IND1_ZONES  256
#define NR_IND2_ZONES  65536
#define INODES_PER_BLOCK 16
#define OK 0
#define IMAP_BLOCKS 1
#define ZMAP_BLOCKS 31
#define LOG_ZONE_SIZE 0
#define FIRSTDATAZONE 346
#define ZONES 249654
#define INODES_PER_BLOCK 16
#define FIRST_IMAP_BLOCK 1
#define FIRST_ZMAP_BLOCK 315
#define FIRST_INODE_BLOCK 2
#define BLOCKSIZE 1024
#define BIT_PER_BLOCK 8192

#define DIR_FILE 2
#define NORMAL_FILE 1

#define BIT1 0x80
#define BIT2 0x40
#define BIT3 0x20
#define BIT4 0x10
#define BIT5 0x08
#define BIT6 0x04
#define BIT7 0x02
#define BIT8 0x01

static char BIT[8] = {0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};

typedef unsigned u32_t;
typedef unsigned short u16_t;

typedef u32_t blockn_t;
typedef u16_t inodesn_t;

typedef struct {
  inodesn_t ninodes;		/* # usable inodes on the minor device */
  blockn_t imap_blocks;		/* # of blocks used by inode bit map */
  blockn_t zmap_blocks;		/* # of blocks used by zone bit map */
  blockn_t firstdatazone;	/* number of first data zone */
  u16_t log_zone_size;		/* log2 of blocks/zone */
  blockn_t zones;		/* number of zones */

  u16_t version;			/* file system version */
  inodesn_t inodes_per_block;
  blockn_t first_imap_block;
  blockn_t first_zmap_block;
  blockn_t first_inode_block;	/* number of first block with inodes */
  size_t dzmap_size;		/* # of data zone blocks */
} super_t;

typedef struct {		/* summary of inode */
  u16_t mode;
  u16_t links;
  u16_t uid;
  u16_t gid;
  u32_t size;

  time_t build_time;
  time_t alter_time;
  time_t sta_time;

  blockn_t direct[NR_DIRECT_ZONES];	/* block numbers for direct,
					 * ind, ... */
  blockn_t ind1;		/* single indirect block number */
  blockn_t ind2;		/* double indirect block number */
  blockn_t ind3;		/* triple indirect block number */
} inode_t;

typedef struct {
  inodesn_t inode_nr;
  char name[MAX_FILE_NAME_LENGTH];
} dir_t;

typedef struct {
	inodesn_t inode_nr;
	inode_t inode;
	blockn_t block_nr;
	char buf[BLOCKSIZE];
	int offset;
	char used;
}fp_t;


/*Global var */
fp_t fp[MAX_OPEN_FILES];
char zeros[BLOCKSIZE];
int last_free_zmap,last_free_zone;
char zmap_buf[BLOCKSIZE];

/* file API */
extern int my_open (const char * path);
extern int my_creat (const char * path);
extern int my_read (int fd, void * buf, int count);
extern int my_write (int fd, const void * buf, int count);
extern int my_close (int fd);

extern int my_remove (const char * path);
extern int my_rename (const char * old, const char * new);
extern int my_mkdir (const char * path);
extern int my_rmdir (const char * path);

extern void my_mkfs (void);

/* provided by the lower layer */

/* not used in any declaration, just a reminder that each block is 1KB */
/* and may be useful inside the code. */
typedef char block [BLOCKSIZE];

extern int dev_open ();
extern int read_block (int block_num, char * block);
extern int write_block (int block_num, char * block);

#endif /* P5_H */

