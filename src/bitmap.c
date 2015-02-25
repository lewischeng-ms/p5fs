#include "bitmap.h"

// Whether to end iterate.
int bitmap_iterate_end;

// Block buffer for current operation.
static u8_t block_buf[BLOCKSIZE];

// Create a bitmap using the specified first block # and # of blocks.
void create_bitmap(bitmap_t * pbitmap, blockn_t first_block, blockn_t block_count)
{
	assert(pbitmap != NULL);
	assert(first_block >= 0 && block_count >= 0);
	
	pbitmap->first_block = first_block;
	pbitmap->block_count = block_count;
	pbitmap->final_block = first_block + block_count - 1;
	pbitmap->bit_count = block_count * BIT_PER_BLOCK;
}

// Get a bit from the specified index.
int get_bit(bitmap_t * pbitmap, u32_t index)
{
	assert(pbitmap != NULL);
	assert(index >= 0 && index < pbitmap->bit_count);
	
	blockn_t current_block = index / BIT_PER_BLOCK + pbitmap->first_block;
	cached_read_block(current_block, block_buf);

	int block_offset = index % BIT_PER_BLOCK;
	int char_index = block_offset / 8;
	int char_offset = block_offset % 8;

	return (block_buf[char_index] & (1 << char_offset)) ? 1 : 0;
}

// Set a bit to the specified index.
void set_bit(bitmap_t * pbitmap, u32_t index, int bit)
{
	assert(pbitmap != NULL);
	assert(bit == 0 || bit == 1);
	assert(index >= 0 && index < pbitmap->bit_count);
	
	blockn_t current_block = index / BIT_PER_BLOCK + pbitmap->first_block;
	cached_read_block(current_block, block_buf);

	int block_offset = index % BIT_PER_BLOCK;
	int char_index = block_offset / 8;
	int char_offset = block_offset % 8;

	if (bit)
		block_buf[char_index] |= (1 << char_offset);
	else
		block_buf[char_index] &= ~(1 << char_offset);

	cached_write_block(current_block, block_buf);
}

// Clear all bits of the bitmap to false.
void clear_bits(bitmap_t * pbitmap)
{
	assert(pbitmap != NULL);
	blockn_t i;
	for (i = pbitmap->first_block; i <= pbitmap->final_block; i++)
		cached_write_block(i, zeros);
}

// Iterate over the bits in the bitmap.
void iterate_over_bits(bitmap_t * pbitmap, u32_t start_index, u32_t end_index, void (*proc)(int, u32_t))
{
	assert(pbitmap != NULL);
	assert(start_index >= 0);
	assert(start_index < end_index);
	assert(end_index <= pbitmap->bit_count);

	bitmap_iterate_end = 0;
	u32_t idx = start_index;
	do {
		blockn_t current_block = idx / BIT_PER_BLOCK + pbitmap->first_block;
		cached_read_block(current_block, block_buf);

		u32_t block_offset = idx % BIT_PER_BLOCK;
		u32_t char_index = block_offset / 8;
		u32_t char_offset = block_offset % 8;

		(*proc)(((block_buf[char_index] & (1 << char_offset)) ? 1 : 0), idx);
	} while (++idx < end_index && !bitmap_iterate_end);
}

// Print a single bit.
static inline void print_bit(int bit, u32_t index)
{
	printf("%d", bit);
}

// Print bits[start_index..end_index] of the bitmap.
void print_bits(bitmap_t * pbitmap, u32_t start_index, u32_t end_index)
{
	iterate_over_bits(pbitmap, start_index, end_index, print_bit);
}

// Count of ones.
static u32_t one_count;

// Increment when the bit is one.
static inline void increment_on_one(int bit, u32_t index)
{
	if (bit) one_count++;
}

// Count ones in the bitmap.
u32_t count_ones(bitmap_t * pbitmap, u32_t start_index, u32_t end_index)
{
	assert(pbitmap != NULL);

	one_count = 0;
	iterate_over_bits(pbitmap, start_index, end_index, increment_on_one);
	return one_count;
}

// Count of zeros.
static u32_t zero_count;

// Increment when the bit is zero.
static inline void increment_on_zero(int bit, u32_t index)
{
	if (!bit) zero_count++;
}

// Count zeros in the bitmap.
u32_t count_zeros(bitmap_t * pbitmap, u32_t start_index, u32_t end_index)
{
	assert(pbitmap != NULL);

	zero_count = 0;
	iterate_over_bits(pbitmap, start_index, end_index, increment_on_zero);
	return zero_count;
}

// The index of the first zero bit.
static first_zero_index;

// Iterate ends when bit is zero.
static inline void end_on_zero(int bit, u32_t index)
{
	if (!bit) {
		first_zero_index = index;
		bitmap_iterate_end = 1;
	}
}

// Find the first zero.
u32_t find_first_zero(bitmap_t * pbitmap, u32_t start_index, u32_t end_index)
{
	assert(pbitmap != NULL);

	first_zero_index = pbitmap->bit_count;
	iterate_over_bits(pbitmap, start_index, end_index, end_on_zero);
	return first_zero_index;
}
