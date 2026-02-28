#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

/**
 * Returns local server date time
 *
 * format example: 2024/04/22 23:59:59
 */
char *get_time() {
    char *buf = malloc(sizeof(char) * 256);
    time_t rawtime = time(NULL);

    struct tm *ptm = localtime(&rawtime);

    strftime(buf, 256, "%Y/%m/%d %H:%M:%S.%f", ptm);
    return buf;
}

/**
 * Returns local server date time with microseconds
 *
 * format example: 2024/04/22 23:59:59.123456
 * Caller must free the returned string.
 */
char *get_time_mics(void) {
    char *buf = malloc(256);
    if (!buf) return NULL;

    struct timeval tv;
    if (gettimeofday(&tv, NULL) != 0) {
        free(buf);
        return NULL;
    }

    time_t sec = tv.tv_sec;
    struct tm tm;
    localtime_r(&sec, &tm);

    /* Format seconds part then append microseconds */
    if (strftime(buf, 256, "%Y/%m/%d %H:%M:%S", &tm) == 0) {
        free(buf);
        return NULL;
    }

    /* Append .microseconds */
    size_t len = strlen(buf);
    snprintf(buf + len, 256 - len, ".%06ld", (long)tv.tv_usec);

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
