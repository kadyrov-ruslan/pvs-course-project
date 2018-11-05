#ifndef _LOG_H_
#define _LOG_H_

#include <unistd.h>

typedef enum log_level_type
{
	ERROR,
	INFO,
	WARN,
	DEBUG,
	LOG_LVL_LAST
} log_level;

typedef enum
{
	LS_CONTINUE,
	LS_STOP
} logger_signal;

int start_logger(const char *log_filename_base);

int stop_logger(void);

int send_log_message(log_level log_lvl, char *message);

extern log_level cur_lvl;

#define LOG(lvl, format_, ...) {					\
	int total_size = 1024;						\
	char *prefix;							\
	if (lvl <= cur_lvl) {							\
		switch (lvl) {							\
		case INFO:							\
			prefix = "I";						\
			break;							\
		case WARN:							\
			prefix = "W";						\
			break;							\
		case ERROR:							\
			prefix = "E";						\
			break;							\
		case DEBUG:							\
			prefix = "D";						\
			break;							\
		default:							\
			abort();						\
		};								\
		char *msg = malloc(sizeof(char) * total_size);			\
		snprintf(msg, total_size, "%s "format_"\n", prefix, __VA_ARGS__);\
		send_log_message(lvl, msg);					\
		free(msg);							\
	}									\
}

#define slog_i(format, ...) LOG(INFO, format, __VA_ARGS__);
#define slog_d(format, ...) LOG(DEBUG, format, __VA_ARGS__);
#define slog_w(format, ...) LOG(WARN, format, __VA_ARGS__);
#define slog_e(format, ...) LOG(ERROR, format, __VA_ARGS__);

#endif
