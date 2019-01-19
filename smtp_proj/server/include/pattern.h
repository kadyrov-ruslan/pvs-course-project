#ifndef _PATTERN_H_
#define _PATTERN_H_

#include <pcre.h>

typedef enum {
    PT_START,

    PT_QUIT,
    PT_RSET,
    PT_VRFY,
    PT_HELO,
    PT_EHLO,
    PT_MAIL,
    PT_RCPT,
    PT_DATA,
    PT_DATA_END,
    PT_DATA_RECV,
    PT_EMAIL,

    PT_END
} pattern_type;

typedef struct
{
    pcre *re[PT_END];
} patterns_t;

extern patterns_t patterns;

int pattern_init();

int pattern_compute(pattern_type type, const char *buf, const char **content);

const char **pattern_email(const char* email);

#endif // _PATTERN_H_
