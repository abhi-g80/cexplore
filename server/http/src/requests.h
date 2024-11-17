#ifndef REQUESTS_H
#define REQUESTS_H

#define MAX_URI 4096
#define MAX_METHOD 8
#define MAX_PROTO 16

struct http_request_info {
    int fd;  // conn fd

    char uri[MAX_URI];
    char method[MAX_METHOD];
    char proto[MAX_PROTO];
};

#endif /* REQUESTS_H */
