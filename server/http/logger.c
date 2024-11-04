#include "logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_BUFFER 2048

static const char *LEVEL_STRING[] = {FOREACH_LEVEL(GENERATE_STRING)};

const char *get_time() {
    char *buf = malloc(sizeof(char) * 256);
    time_t rawtime = time(NULL);

    struct tm *ptm = localtime(&rawtime);

    strftime(buf, 256, "%Y/%m/%d %H:%M:%S", ptm);
    return buf;
}

void logger_f(enum LOG_LEVEL level, const char *file, int lineno, const char *fmt, ...) {
    const char *timenow = get_time();
    const char *level_string = LEVEL_STRING[level];
    FILE *stream = stdout;

    va_list args1;
    va_start(args1, fmt);
    va_list args2;
    va_copy(args2, args1);
    char buf[1 + vsnprintf(NULL, 0, fmt, args1)];
    va_end(args1);
    vsnprintf(buf, sizeof(buf), fmt, args2);
    va_end(args2);

    if (level == ERROR) {
        stream = stderr;
    }
    if (level == INFO) {
        level_string = "INFO ";
    }
    char fmt_s[MAX_BUFFER];
    if (level == ERROR) {
        sprintf(fmt_s, "%s [%s] %s:%d -> %s\n", timenow, level_string, file, lineno, buf);
    } else {
        sprintf(fmt_s, "%s [%s] %s:%d -> %s\n", timenow, level_string, file, lineno, buf);
    }
    fprintf(stream, "%s", fmt_s);

    /* if(level == ERROR) { */
    /*     fprintf(stderr, "%s [ERROR] %s:%d -> %s\n", timenow, file, lineno,
     * buf); */
    /* } else { */
    /*     fprintf(stderr, "%s [%s] %s:%d -> %s\n", timenow, LEVEL_STRING[level],
     * file, lineno, buf); */
    /* } */
}
