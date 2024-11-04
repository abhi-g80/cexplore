#include <stdarg.h>

#ifndef LOGGER_H
#define LOGGER_H

#define FOREACH_LEVEL(LEVEL) \
    LEVEL(INFO)              \
    LEVEL(DEBUG)             \
    LEVEL(ERROR)

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

enum LOG_LEVEL { FOREACH_LEVEL(GENERATE_ENUM) };

void logger_f(enum LOG_LEVEL level, const char *file, int lineno, const char *fmt, ...);

#endif /* LOG_H */
