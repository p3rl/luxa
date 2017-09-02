#ifndef HASH_H
#define HASH_H

#include <luxa/platform.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
* From https://tanjent.livejournal.com/756623.html
*/
static uint32_t lx_murmur_hash_32(const unsigned char *data, size_t size, uint32_t hash)
{
	const unsigned int m = 0x7fd652ad;
	const int r = 16;
	
	hash += 0xdeadbeef;

	while (size >= 4)
	{
		hash += *(uint32_t *)data;
		hash *= m;
		hash ^= hash >> r;

		data += 4;
		size -= 4;
	}

	switch (size)
	{
	case 3:
		hash += data[2] << 16;
	case 2:
		hash += data[1] << 8;
	case 1:
		hash += data[0];
		hash *= m;
		hash ^= hash >> r;
	};

	hash *= m;
	hash ^= hash >> 10;
	hash *= m;
	hash ^= hash >> 17;

	return hash;
}

#ifdef __cplusplus
}
#endif

#endif //HASH_H