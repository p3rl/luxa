#include <luxa/id.h>
#include <luxa/hash.h>
#include <string.h>

lx_id32_t lx_id32(const char *s)
{
	LX_ASSERT(s, "Invalid id string");
	return lx_murmur_hash_32(s, strlen(s), 0);
}

lx_id32_t lx_id32_nil()
{
	return 0u;
}

bool lx_id32_is_nil(lx_id32_t id)
{
	return id == 0;
}

lx_id64_t lx_id64(const char *s)
{
	LX_ASSERT(s, "Invalid id string");
	return lx_murmur_hash_64(s, strlen(s), 0);
}

lx_id64_t lx_id64_nil()
{
	return 0u;
}

bool lx_id64_is_nil(lx_id64_t id)
{
	return id == 0u;
}
