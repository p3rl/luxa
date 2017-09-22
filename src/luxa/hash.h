#ifndef HASH_H
#define HASH_H

#include <luxa/platform.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
* From https://tanjent.livejournal.com/756623.html
*/
static uint32_t lx_murmur_hash_32(const void *key, size_t size, uint32_t hash)
{
	LX_ASSERT(key, "Invalid key");
	LX_ASSERT(size, "Invalid size");

	const unsigned int m = 0x7fd652ad;
	const int r = 16;
	
	hash += 0xdeadbeef;

	const unsigned char *data = (const unsigned char *)key;

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

/*
 * From https://stackoverflow.com/questions/5611188/how-to-use-murmurhash-64-in-objective-c
 */
static uint64_t lx_murmur_hash_64(const void *key, size_t len, unsigned int seed)
{
	LX_ASSERT(key, "Invalid key");
	LX_ASSERT(len, "Invalid key length");

	const unsigned int m = 0x5bd1e995;
	const int r = 24;

	unsigned int h1 = seed ^ (unsigned int)len;
	unsigned int h2 = 0;

	const unsigned int *data = (const unsigned int *)key;

	while (len >= 8)
	{
		unsigned int k1 = *data++;
		k1 *= m; k1 ^= k1 >> r; k1 *= m;
		h1 *= m; h1 ^= k1;
		len -= 4;

		unsigned int k2 = *data++;
		k2 *= m; k2 ^= k2 >> r; k2 *= m;
		h2 *= m; h2 ^= k2;
		len -= 4;
	}

	if (len >= 4)
	{
		unsigned int k1 = *data++;
		k1 *= m; k1 ^= k1 >> r; k1 *= m;
		h1 *= m; h1 ^= k1;
		len -= 4;
	}

	switch (len)
	{
	case 3: h2 ^= ((unsigned char*)data)[2] << 16;
	case 2: h2 ^= ((unsigned char*)data)[1] << 8;
	case 1: h2 ^= ((unsigned char*)data)[0];
		h2 *= m;
	};

	h1 ^= h2 >> 18; h1 *= m;
	h2 ^= h1 >> 22; h2 *= m;
	h1 ^= h2 >> 17; h1 *= m;
	h2 ^= h1 >> 19; h2 *= m;

	uint64_t h = h1;

	h = (h << 32) | h2;

	return h;
}

static inline size_t lx_string_hash64(const lx_any_t key)
{
	const char *s = (const char*)key;
	return (size_t)lx_murmur_hash_64(s, strlen(s), 0);
}

static inline size_t lx_id_hash64(const lx_any_t key)
{
    return (size_t)key;
}

#ifdef __cplusplus
}
#endif

#endif //HASH_H