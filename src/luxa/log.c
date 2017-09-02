#include <luxa/log.h>
#include <luxa/collections/array.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

typedef struct log_target
{
	lx_log_level_t log_level;
	lx_log_callback_t log;
	void *user_data;
} log_target_t;

static lx_allocator_t *_allocator;
static lx_array_t *_log_targets;
static lx_log_level_t _log_level;

void lx_initialize_log(lx_allocator_t *allocator, lx_log_level_t log_level)
{
	LX_ASSERT(_allocator == NULL, "Log already initialized");
	LX_ASSERT(allocator != NULL, "Invalid allocator");
	_allocator = allocator;
	_log_targets = lx_array_create(allocator, sizeof(log_target_t));
	_log_level = log_level;
}

void lx_shutdown_log()
{
	lx_array_destroy(_log_targets);
	_allocator = NULL;
	_log_targets = NULL;
}

void lx_register_log_target(lx_log_level_t log_level, lx_log_callback_t log_callback, void *user_data)
{
	LX_ASSERT(log_callback, "Invalid log callback function");
	log_target_t log_target = { .log_level = log_level, .log = log_callback, .user_data = user_data };
	lx_array_push_back(_log_targets, &log_target);
}

void lx_log(lx_log_level_t log_level, const char *tag, const char *format, ...)
{
	va_list list;
	va_start(list, format);
	char message[1024];
	vsprintf_s(message, 1024, format, list);
	va_end(list);

	time_t current_time;
	time(&current_time);
	struct tm* tm_info;
	tm_info = localtime(&current_time);
	char time_tag[64];
	strftime(time_tag, 64, "%H:%M:%S", tm_info);

	char log[2048];
	sprintf_s(log, 2048, "[%s][%s]: %s\n", time_tag, tag ? tag : "General", message);

	for (unsigned i = 0; i < lx_array_size(_log_targets); ++i) {
		log_target_t *log_target = (log_target_t*)lx_array_at(_log_targets, i);
		if (_log_level >= log_target->log_level)
			log_target->log(log_level, log, log_target->user_data);
	}
}