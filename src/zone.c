#include "zone.h"

extern super_t super_block;

// Zone bitmap.
static bitmap_t zmap;

// The number of a zone which was most recently freed.
static u32_t last_free_zone_num = 0;

// Initialize the zone map.
void init_zmap()
{
	create_bitmap(&zmap, super_block.first_zmap_block, super_block.zmap_blocks);
}

// Apply for an zone.
// Return the # of zone, 0 for failure.
u32_t alloc_zone()
{
	u32_t index;
	u32_t num;

	// No last freed zone so look for first fit.
	if (last_free_zone_num == 0)
	{
		index = find_first_zero(&zmap, 0, zmap.bit_count);
		if (index == zmap.bit_count)
		{
			printf("the data zone is full!\n");
			return 0;
		}
	}
	else
	{
		index = last_free_zone_num - 1;
		last_free_zone_num = 0;
	}

	set_bit(&zmap, index, 1);
	num = index + 1;

	// Clear all data on the zone.
	write_zone(num, zeros);

	return num;
}

// Free the zone at the specified number.
// The function won't be checked with zmap when not debugging.
void free_zone(u32_t num)
{
	assert(num >= 1 && num <= zmap.bit_count);

	u32_t index = num - 1;
	assert(get_bit(&zmap, index) == 1);

	set_bit(&zmap, index, 0);
	last_free_zone_num = num;
}

// Read the zone to device at the specified number.
// The function won't be checked with zmap when not debugging.
void read_zone(u32_t num, char * zbuf)
{
	assert(zbuf != NULL);
	assert(num >= 1 && num <= zmap.bit_count);

	u32_t index = num - 1;
	assert(get_bit(&zmap, index) == 1);

	cached_read_block(super_block.firstdatazone + index, zbuf);
}

// Write the zone to device at the specified number.
// The function won't be checked with zmap when not debugging.
void write_zone(u32_t num, char * zbuf)
{
	assert(zbuf != NULL);
	assert(num >= 1 && num <= zmap.bit_count);

	u32_t index = num - 1;
	assert(get_bit(&zmap, index) == 1);

	cached_write_block(super_block.firstdatazone + index, zbuf);
}

// Count occupied zones.
u32_t count_occupied_zones()
{
	return count_ones(&zmap, 0, super_block.zones);
}

// Count free zones.
u32_t count_free_zones()
{
	return count_zeros(&zmap, 0, super_block.zones);
}

// Clear all zones.
void clear_zones()
{
	clear_bits(&zmap);
}
