#ifndef LOG_H
#define LOG_H

#include <luxa/memory/allocator.h>

typedef enum lx_log_level
{
	LX_LOG_LEVEL_OFF,
	LX_LOG_LEVEL_ERROR,
	LX_LOG_LEVEL_WARNING,
	LX_LOG_LEVEL_INFO,
	LX_LOG_LEVEL_DEBUG,
	LX_LOG_LEVEL_TRACE,
} lx_log_level_t;

typedef void (*lx_log_callback_t)(time_t time, lx_log_level_t log_level, const char *tag, const char *message, void *user_data);

#ifdef __cplusplus
extern "C" {
#endif

#define LX_LOG(log_level, tag, message, ...) lx_log(log_level, tag, message, ##__VA_ARGS__)
#define LX_LOG_DEBUG(tag, message, ...) LX_LOG(LX_LOG_LEVEL_DEBUG, tag, message, ##__VA_ARGS__)
#define LX_LOG_INFO(tag, message, ...) LX_LOG(LX_LOG_LEVEL_INFO, tag, message, ##__VA_ARGS__)
#define LX_LOG_WARNING(tag, message, ...) LX_LOG(LX_LOG_LEVEL_WARNING, tag, message, ##__VA_ARGS__)
#define LX_LOG_ERROR(tag, message, ...) LX_LOG(LX_LOG_LEVEL_ERROR, tag, message, ##__VA_ARGS__)

void lx_initialize_log(lx_allocator_t *allocator, lx_log_level_t log_level);

void lx_shutdown_log();

void lx_register_log_target(lx_log_level_t log_level, lx_log_callback_t log_callback, void *user_data);

void lx_log(lx_log_level_t level, const char* tag, const char *format, ...);

const char *lx_log_level_to_c_str(lx_log_level_t log_level);

#ifdef __cplusplus
}
#endif

#endif // LOG_H
