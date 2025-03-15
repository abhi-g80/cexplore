#include "response.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>  // for free
#include <string.h>

#include "defaults.h"
#include "logger.h"
#include "utils.h"

char not_found_response[] =
    "<title>Webby - 404</title>"
    "<html><body><h1>404 Not found</h1>"
    "<h3><i>Sorry, the requested resource wasn't found</h3><hr>"
    "<p style='font-family:\"Lucida Console\", monospace'>Powered by Webby</p>"
    "</body></html>";

/**
 * Example response html
 */
char example_html_response[] =
    "<html><h1><center><b>It could be working!</b></center></h1></html>\r\n";

extern char WEBBY_ROOT[MAX_BUFFER];

char *http_content_type_string(enum http_content_type n) {
    switch (n) {
        case HttpContentType_TextHtml:
            return "text/html";
        default:
            return "text/plain; charset=utf-8";
    }
}

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

/**
 * Return the HTTP header string based on the proto and status code
 *
 * Example: HTTP/1.1 200 OK
 */
char *build_http_status(enum http_proto p, enum http_status_code n) {
    char *s = (char *)malloc(sizeof(char) * MAX_BUFFER);
    sprintf(s, "%s %d %s", http_proto_string(p), n, http_status_string(n));
    return s;
}

char *add_http_header(char *header, char *key, size_t keylen, char *value, size_t valuelen) {
    char *h = (char *)malloc(sizeof(char)*(keylen+valuelen));
    int res_len = sprintf(h, "%s%s: %s\r\n", header, key, value);
    return h;
}

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



size_t get_file_size(FILE *resource) {
    fseek(resource, 0, SEEK_END);
    size_t size = ftell(resource);
    fseek(resource, 0, SEEK_SET);

    return size;
}

char *strconcat(const char *s1, const char *s2) {
    char *result = (char *)malloc(strlen(s1) + strlen(s2) + 1);

    strcpy(result, s1);
    strcat(result, s2);

    return result;
}

/*
 * Read a file content from res and send that over fd with content type set to type.
 * It is assumed that the file exists and hence the HTTP status is set to OK. The check
 * for file presence is a responsibility of the caller.
 */
int send_file_content(int fd, FILE *res, enum http_content_type type) {
    char content[MAX_RESPONSE_SIZE];
    size_t file_size = get_file_size(res);

    log_debug("file size: %d bytes", file_size);

    char *ret_http_status = build_http_status(HttpProtoHTTP_1_1, HttpStatusCodeOk);

    size_t b = fread(content, 1, MAX_RESPONSE_SIZE, res);
    log_debug("Read from file: %d bytes", b);

    int w = send_response(fd, ret_http_status, http_content_type_string(type), content, file_size);
    free(ret_http_status);
    return w;
}



/**
 * Send html response (chunked transfer not possible)
 */
int send_html_response(int fd, struct http_request_info *hri) {
    char *path = strconcat(WEBBY_ROOT, hri->uri);
    FILE *res = fopen(path, "r");
    free(path);

    char *ret_http_status = NULL;

    if (res == NULL) {
        ret_http_status = build_http_status(HttpProtoHTTP_1_1, HttpStatusCodeNotFound);
        int w =
            send_response(fd, ret_http_status, http_content_type_string(HttpContentType_TextHtml),
                          not_found_response, strlen(not_found_response));

        free(ret_http_status);
        return w;
    }

    int w = send_file_content(fd, res, HttpContentType_TextHtml);
    fclose(res);
    return w;
}

/**
 * Send text file (chunked transfer not possible)
 */
int send_text_response(int fd, struct http_request_info *hri) {
    if (strcmp(hri->uri, "/") == 0) {
        // default
        char *ret_http_status = build_http_status(HttpProtoHTTP_1_1, HttpStatusCodeOk);
        int w =
            send_response(fd, ret_http_status, http_content_type_string(HttpContentType_TextHtml),
                          example_html_response, strlen(example_html_response));
        free(ret_http_status);
        return w;
    }
    char *path = strconcat(WEBBY_ROOT, hri->uri);
    FILE *res = fopen(path, "r");
    free(path);

    char *ret_http_status = NULL;

    if (res == NULL) {
        ret_http_status = build_http_status(HttpProtoHTTP_1_1, HttpStatusCodeNotFound);
        int w =
            send_response(fd, ret_http_status, http_content_type_string(HttpContentType_TextHtml),
                          not_found_response, strlen(not_found_response));
        free(ret_http_status);
        return w;
    }

    int w = send_file_content(fd, res, HttpContentType_TextPlain);
    fclose(res);
    return w;
}
