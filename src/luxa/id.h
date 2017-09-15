#ifndef ID_H
#define ID_H

#include <luxa/platform.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t lx_id32_t;
typedef uint64_t lx_id64_t;

lx_id32_t lx_id32(const char *s);

lx_id32_t lx_id32_nil();

bool lx_id32_is_nil(lx_id32_t id);

lx_id64_t lx_id64(const char *s);

lx_id64_t lx_id64_nil();

bool lx_id64_is_nil(lx_id64_t id);

#ifdef __cplusplus
}
#endif

#endif // ID_H