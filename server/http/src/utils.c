#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
