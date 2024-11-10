#ifndef LOGGER_H
#define LOGGER_H
#include <stdarg.h>

#define FOREACH_LEVEL(LEVEL) \
    LEVEL(INFO )             \
    LEVEL(DEBUG)             \
    LEVEL(ERROR)

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

enum LOG_LEVEL { FOREACH_LEVEL(GENERATE_ENUM) };

void logger_f(enum LOG_LEVEL level, const char *file, int lineno, const char *fmt, ...);

extern int DEBUG_F;

#define log_info(...) logger_f(INFO, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) logger_f(DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) logger_f(ERROR, __FILE__, __LINE__, __VA_ARGS__)

#endif /* LOG_H */
