#ifndef P5FS_ZONE_H_
#define P5FS_ZONE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bitmap.h"

// Initialize the zone map.
void init_zmap();

// Apply for an zone.
// Return the # of zone, 0 for failure.
u32_t alloc_zone();

// Free the zone at the specified number.
// The function won't be checked with zmap when not debugging.
void free_zone(u32_t num);

// Read the zone to device at the specified number.
// The function won't be checked with zmap when not debugging.
void read_zone(u32_t num, char * zbuf);

// Write the zone to device at the specified number.
// The function won't be checked with zmap when not debugging.
void write_zone(u32_t num, char * zbuf);

// Clear all zones.
void clear_zones();

// Count occupied zones.
u32_t count_occupied_zones();

// Count free zones.
u32_t count_free_zones();

#endif // P5FS_ZONE_H_
