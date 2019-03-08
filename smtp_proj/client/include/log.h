#ifndef _LOG_H_
#define _LOG_H_

#include "../../common/include/dir_utils.h"
#include "../../common/include/string_utils.h"
#include "map.h"
#include <sys/ipc.h> 
#include <sys/msg.h> 

typedef enum log_level_type
{
	ERROR,
	INFO,
	WARN,
	DEBUG,
	LOG_LVL_LAST
} log_level;

int start_logger(char *log_filename_base);
int stop_logger(void);
int send_log_message(log_level log_lvl, char *message);
int save_log(char *message, char *logs_dir);

extern log_level cur_lvl;

#define LOG(lvl, format_, ...) {					\
	int total_size = 1024;						\
	char *prefix;							\
	if (lvl <= cur_lvl) {							\
		switch (lvl) {							\
		case INFO:							\
			prefix = "INFO";						\
			break;							\
		case WARN:							\
			prefix = "WARN";						\
			break;							\
		case ERROR:							\
			prefix = "ERROR";						\
			break;							\
		case DEBUG:							\
			prefix = "DEBUG";						\
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

#define log_i(format, ...) LOG(INFO, format, __VA_ARGS__);
#define log_d(format, ...) LOG(DEBUG, format, __VA_ARGS__);
#define log_w(format, ...) LOG(WARN, format, __VA_ARGS__);
#define log_e(format, ...) LOG(ERROR, format, __VA_ARGS__);

#endif
