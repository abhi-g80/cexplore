#include "logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defaults.h"
#include "utils.h"

static const char *LEVEL_STRING[] = {FOREACH_LEVEL(GENERATE_STRING)};

extern int DEBUG_F;

/**
 * Log message to stdout for INFO and DEBUG and stderr for ERROR.
 * Will call perror() when logging ERROR message.
 */
void logger_f(enum LOG_LEVEL level, const char *file, int lineno, const char *fmt, ...) {
    char *timenow = get_time();

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
    }
    char fmt_s[MAX_BUFFER];

    sprintf(fmt_s, "%s [%-5s] %s:%-3d -> %s\n", timenow, level_string, file, lineno, buf);
    free(timenow);
    if (level == DEBUG && DEBUG_F != 1) {
        return;
    }
    fprintf(stream, "%s", fmt_s);

    if (level == ERROR) {
        perror(NULL);
    }
    fflush(stdout);
    fflush(stderr);
}
