#ifndef _LOG_H_
#define _LOG_H_

typedef struct log_options_t
{
	const char *path;
} log_options_t;

typedef enum log_level
{
	ERROR,
	INFO,
	WARN,
	DEBUG
} log_level;

extern log_level cur_level;

int logger_start(log_options_t *opts);
int logger_stop(void);
int log_message(log_level level, const char *message);

#endif // _LOG_H_