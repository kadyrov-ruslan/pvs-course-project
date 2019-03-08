#include "pattern.h"

#include <stdio.h>
#include <string.h>

int pattern_init()
{
    char* _patterns[PT_END];
    _patterns[PT_QUIT] = "^[Qq][Uu][Ii][Tt][\\r\\n]?$";
    _patterns[PT_RSET] = "^[Rr][Ss][Ee][Tt][\\r\\n]?$";
    _patterns[PT_VRFY] = "^[Vv][Rr][Ff][Yy] (.+?)[\\r\\n]?$";
    _patterns[PT_HELO] = "^[Hh][Ee][Ll][Oo] (.+?)[\\r\\n]?$";
    _patterns[PT_EHLO] = "^[Ee][Hh][Ll][Oo] (.+?)[\\r\\n]?$";
    _patterns[PT_MAIL] = "^[Mm][Aa][Ii][Ll]\\s[Ff][Rr][Oo][Mm]:\\s?[<]?(.+?)[>]?[\\r\\n]?$";
    _patterns[PT_RCPT] = "^[Rr][Cc][Pp][Tt]\\s[Tt][Oo]:\\s?[<]?(.+?)[>]?[\\r\\n]?$";
    _patterns[PT_DATA] = "^[Dd][Aa][Tt][Aa][\\r\\n]?$";
    _patterns[PT_DATA_END] = "^[\\r\\n]?[.][\\r\\n]?$";
    _patterns[PT_DATA_RECV] = "(.*)";
    _patterns[PT_EMAIL] = "^(.+?)@(.+?)$";

    pcre* re;
    int err_offset;
    const char* err;
    for (int i = PT_START + 1; i < PT_END; ++i)
    {
        re = pcre_compile(_patterns[i], 0, &err, &err_offset, NULL);
        if (!re)
        {
            fprintf(stderr, "PCRE compilation for %d failed at offset %d :%s\n", i, err_offset, err);
            return -1;
        }
        else
            patterns.re[i] = re;
    }

    return 0;
}

int pattern_compute(pattern_type type, const char* buf, const char** content)
{
    int count, ovector[32];
    pcre *re = patterns.re[type];
    if ((count = pcre_exec(re, NULL, buf, (int) strlen(buf), 0, 0, ovector, sizeof(ovector) / sizeof(ovector[0]))) < 0)
        return -1;

    pcre_get_substring(buf, ovector, count, 1, content);
    return 0;
}

const char **pattern_email(const char* buf)
{
    int count, ovector[32];
    pcre* re = patterns.re[PT_EMAIL];
    if ((count = pcre_exec(re, NULL, buf, (int) strlen(buf), 0, 0, ovector, sizeof(ovector) / sizeof(ovector[0]))) < 0)
        return NULL;

    const char **res = malloc(2 * sizeof(char*));
    pcre_get_substring(buf, ovector, count, 1, &res[0]);
    pcre_get_substring(buf, ovector, count, 2, &res[1]);
    return res;
}