#ifndef P5FS_CACHE_H_
#define P5FS_CACHE_H_

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "p5.h"

int cached_read_block(int block_num, char * block);
int cached_write_block(int block_num, char * block);

#endif // P5FS_CACHE_H_
