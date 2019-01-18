#ifndef _LOG_H_
#define _LOG_H_

typedef struct log_options_t
{
    const char *path;
} log_options_t;

typedef enum log_level
{
    INFO,
    WARN,
    ERROR,
    DEBUG
} log_level;

extern log_level cur_level;

int logger_start(log_options_t *opts);
int logger_stop(void);
int log_message(log_level level, const char *message);

#define LOG(lvl, format_, ...) {                    \
    int size = 255;                                 \
    char *prefix;                                   \
    if (lvl < cur_level) {                          \
        switch(lvl) {                               \
        case INFO:                                  \
            prefix = "INFO:";                       \
            break;                                  \
        case WARN:                                  \
            prefix = "WARN:";                       \
            break;                                  \
        case ERROR:                                 \
            prefix = "ERROR:";                      \
            break;                                  \
        case DEBUG:                                 \
            prefix = "DEBUG:";                      \
            break;                                  \
        default:                                    \
            abort();                                \
        };                                          \
        char *msg = malloc(size * sizeof(char));    \
        snprintf(msg, size, "%s " format_"\n", prefix, __VA_ARGS__);\
        log_message(lvl, msg);                      \
        free(msg);                                  \
    }                                               \
}

#define log_i(format, ...) LOG(INFO, format, __VA_ARGS__);
#define log_w(format, ...) LOG(WARN, format, __VA_ARGS__);
#define log_e(format, ...) LOG(ERROR, format, __VA_ARGS__);
#define log_d(format, ...) LOG(DEBUG, format, __VA_ARGS__);

#endif // _LOG_H_