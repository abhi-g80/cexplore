#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <string.h>


/**
 * Returns local server date time
 *
 * format example: 2024/04/22 23:59:59
 */
char *get_time() {
    char *buf = malloc(sizeof(char) * 256);
    time_t rawtime = time(NULL);

    struct tm *ptm = localtime(&rawtime);

    strftime(buf, 256, "%Y/%m/%d %H:%M:%S", ptm);
    return buf;
}

/**
 * Returns local server date time in GMT
 *
 * format example: Wed, 06 Nov 2024 21:12:07 GMT
 */
char *get_server_date() {
    char *buf = malloc(sizeof(char) * 256);
    time_t rawtime = time(NULL);

    struct tm *ptm = gmtime(&rawtime);

    strftime(buf, 256, "%a, %d %b %Y %T %Z", ptm);
    return buf;
}

/**
 * File size for a FILE resource
 */
size_t get_file_size(FILE *resource) {
    fseek(resource, 0, SEEK_END);
    size_t size = ftell(resource);
    fseek(resource, 0, SEEK_SET);

    return size;
}

/**
 * Returns string pointing to s1+s2
 */
char *strconcat(const char *s1, const char *s2) {
    char *result = (char *)malloc(strlen(s1) + strlen(s2) + 1);

    strcpy(result, s1);
    strcat(result, s2);

    return result;
}

/**
 * Setup the root location of the website
 */
void setup_webby_root(char *w) {
    char *wbr = getenv("WEBBY_ROOT");
    if (wbr == NULL) {
        fprintf(stderr, "Please set WEBBY_ROOT environment variable\n");
        exit(EXIT_FAILURE);
    }
    strcpy(w, wbr);
}

