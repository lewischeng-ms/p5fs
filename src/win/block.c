#include <stdio.h>
#include "p5.h"

/* only open the file once */
static FILE * dev_fp = NULL;
static int devsize = 0;

/* returns the device size (in blocks) if the operation is successful,
 * and -1 otherwise */
int dev_open()
{
	if (dev_fp == NULL)
	{
		dev_fp = fopen("simulated_device", "rb+");
		if (dev_fp == NULL)
		{
			perror("fopen");
			return -1;
		}
		fseek(dev_fp, 0, SEEK_END);
		devsize = ftell(dev_fp) / BLOCKSIZE;
	}
	return devsize;
}

/* returns 0 if the operation is successful, and -1 otherwise */
int read_block(int block_num, char * block)
{
	if (block_num >= devsize)
	{
		printf ("read block number requested %d, maximum %d\n", block_num, devsize - 1);
		return -1;
	}
	if (fseek(dev_fp, block_num * BLOCKSIZE,  SEEK_SET) != 0)
	{
		perror ("fseek");
		return -1;
	}
	if (fread(block, 1, BLOCKSIZE, dev_fp) != BLOCKSIZE)
	{
		perror ("fread");
		return -1;
	}
	return 0;
}

/* returns 0 if the operation is successful, and -1 otherwise */
int write_block(int block_num, char * block)
{
	if (block_num >= devsize)
	{
		printf("write block number requested %d, maximum %d\n", block_num, devsize - 1);
		return -1;
	}
	if (fseek(dev_fp, block_num * BLOCKSIZE,  SEEK_SET) != 0)
	{
		perror ("fseek");
		return -1;
	}
	if (fwrite(block, 1, BLOCKSIZE, dev_fp) != BLOCKSIZE)
	{
		perror("fwrite");
		return -1;
	}
	return 0;
}