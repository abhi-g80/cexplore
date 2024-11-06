#include "../include/logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/utils.h"

#define MAX_BUFFER 2048

static const char *LEVEL_STRING[] = {FOREACH_LEVEL(GENERATE_STRING)};

void logger_f(enum LOG_LEVEL level, const char *file, int lineno, const char *fmt, ...) {
    const char *timenow = get_time();

    va_list args1;
    va_list args2;

    va_start(args1, fmt);
    va_copy(args2, args1);

    char buf[1 + vsnprintf(NULL, 0, fmt, args1)];

    va_end(args1);
    vsnprintf(buf, sizeof(buf), fmt, args2);
    va_end(args2);

    const char *level_string = LEVEL_STRING[level];
    FILE *stream = stdout;
    if (level == ERROR) {
        stream = stderr;
    } else if (level == INFO) {
        level_string = "INFO ";
    }
    char fmt_s[MAX_BUFFER];
    if (level == ERROR) {
        sprintf(fmt_s, "%s [%s] %s:%d -> %s\n", timenow, level_string, file, lineno, buf);
        perror(NULL);
    } else {
        sprintf(fmt_s, "%s [%s] %s:%d -> %s\n", timenow, level_string, file, lineno, buf);
    }
    fprintf(stream, "%s", fmt_s);
}
