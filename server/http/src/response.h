#ifndef RESPONSES_H
#define RESPONSES_H

#include "requests.h"

int send_response(int, const char *, char *, void *, int);

enum http_method { GET, HEAD, POST, PUT, DELETE, CONNECT, OPTIONS, TRACE, PATCH };

enum http_content_type {
    HttpContentType_TextHtml = 1,
    HttpContentType_TextPlain = 2,
};

enum http_proto {
    HttpProtoHTTP_0_9 = 9,
    HttpProtoHTTP_1_0 = 10,
    HttpProtoHTTP_1_1 = 11,
    HttpProtoHTTP_2_0 = 20
};

enum http_status_code {
    HttpStatusCodeContinue = 100,
    HttpStatusCodeSwitchingProtocols = 101,
    HttpStatusCodeEarlyHints = 103,
    HttpStatusCodeOk = 200,
    HttpStatusCodeCreated = 201,
    HttpStatusCodeAccepted = 202,
    HttpStatusCodeNonAuthoritativeInformation = 203,
    HttpStatusCodeNoContent = 204,
    HttpStatusCodeResetContent = 205,
    HttpStatusCodePartialContent = 206,
    HttpStatusCodeMultipleChoices = 300,
    HttpStatusCodeMovedPermanently = 301,
    HttpStatusCodeFound = 302,
    HttpStatusCodeNotModified = 304,
    HttpStatusCodeBadRequest = 400,
    HttpStatusCodeUnauthorized = 401,
    HttpStatusCodeForbidden = 403,
    HttpStatusCodeNotFound = 404,
    HttpStatusCodeMethodNotAllowed = 405,
    HttpStatusCodeRequestTimeout = 408,
    HttpStatusCodeImATeapot = 418,
    HttpStatusCodeInternalServerError = 500,
    HttpStatusCodeNotImplemented = 501,
    HttpStatusCodeBadGateway = 502,
    HttpStatusCodeServiceUnvailable = 503,
    HttpStatusCodeGatewayTimeout = 504,
    HttpStatusCodeHttpVersionNotSupported = 505
};

char *build_http_status(enum http_proto, enum http_status_code);

int send_html_response(int, struct http_request_info *);
/* int send_text_response(int, char*, struct http_request_info *); */
int send_text_response(int, struct http_request_info *);
#endif /* RESPONSES_H */
