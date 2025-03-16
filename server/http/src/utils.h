#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

char *get_time();

char *get_server_date();

size_t get_file_size(FILE *);

char *strconcat(const char *, const char *);

void setup_webby_root(char *);

#endif /* LOG_H */
