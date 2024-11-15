#include "response.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>  // for free
#include <string.h>

#include "defaults.h"
#include "utils.h"

const char *http_proto_string(enum http_proto n) {
    switch (n) {
        case HttpProtoHTTP_0_9:
            return "HTTP/0.9";
        case HttpProtoHTTP_1_0:
            return "HTTP/1.0";
        case HttpProtoHTTP_1_1:
            return "HTTP/1.1";
        case HttpProtoHTTP_2_0:
            return "HTTP/2.0";
        default:
            return "";
    }
}

const char *http_status_string(enum http_status_code n) {
    switch (n) {
        case HttpStatusCodeContinue:
            return "Continue";
        case HttpStatusCodeOk:
            return "OK";
        case HttpStatusCodeAccepted:
            return "Accepted";
        case HttpStatusCodeMovedPermanently:
            return "Moved Permanently";
        case HttpStatusCodeFound:
            return "Found";
        case HttpStatusCodeBadRequest:
            return "Bad Request";
        case HttpStatusCodeForbidden:
            return "Forbidden";
        case HttpStatusCodeNotFound:
            return "Not Found";
        case HttpStatusCodeMethodNotAllowed:
            return "Method Not Allowed";
        case HttpStatusCodeImATeapot:
            return "I'm a Teapot";
        case HttpStatusCodeInternalServerError:
            return "Internal Server Error";
        case HttpStatusCodeNotImplemented:
            return "Not Implemented";
        case HttpStatusCodeBadGateway:
            return "Bad Gateway";
        default:
            return "";
    }
}

char *build_http_status(enum http_status_code n, enum http_proto p) { return "HTTP/1.1 200 OK"; }

/**
 * Send an HTTP response.
 *
 * Return the value from the send() function.
 */
int send_response(int fd, const char *http_status, char *content_type, void *body,
                  int content_length) {
    char response[MAX_RESPONSE_SIZE];

    char *server_date = get_server_date();

    int response_length = sprintf(response,
                                  "%s\r\n"
                                  "Date: %s\r\n"
                                  "Server: %s\r\n"
                                  "Content-Length: %d\r\n"
                                  "Content-Type: %s\r\n"
                                  "\r\n",
                                  http_status, server_date, APP_NAME, content_length, content_type);
    memcpy(response + response_length, body, content_length);
    free(server_date);
    return send(fd, response, response_length + content_length, 0);
}
