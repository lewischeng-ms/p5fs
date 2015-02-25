#ifndef P5FS_BITMAP_H_
#define P5FS_BITMAP_H_

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "p5.h"

typedef unsigned char u8_t;

typedef struct {
	blockn_t first_block;
	blockn_t block_count;
	blockn_t final_block;
	u32_t bit_count;
} bitmap_t;

// Whether to end iterate.
extern int bitmap_iterate_end;

// Create a bitmap using the specified first block # and # of blocks.
void create_bitmap(bitmap_t * pbitmap, blockn_t first_block, blockn_t block_count);

// Get a bit from the specified index.
int get_bit(bitmap_t * pbitmap, u32_t index);

// Set a bit to the specified index.
void set_bit(bitmap_t * pbitmap, u32_t index, int bit);

// Iterate over the bits in the bitmap.
void iterate_over_bits(bitmap_t * pbitmap, u32_t start_index, u32_t end_index, void (*proc)(int, u32_t));

// Print bits[start_index..end_index] of the bitmap.
void print_bits(bitmap_t * pbitmap, u32_t start_index, u32_t end_index);

// Clear all bits of the bitmap to false.
void clear_bits(bitmap_t * pbitmap);

// Count ones in the bitmap.
u32_t count_ones(bitmap_t * pbitmap, u32_t start_index, u32_t end_index);

// Count zeros in the bitmap.
u32_t count_zeros(bitmap_t * pbitmap, u32_t start_index, u32_t end_index);

// Find the first zero.
u32_t find_first_zero(bitmap_t * pbitmap, u32_t start_index, u32_t end_index);

#endif // P5FS_BITMAP_H_
