#include <arpa/inet.h>
#include <error.h>
#include <netinet/ip.h>  // contains socket.h via in.h
#include <stdio.h>
#include <stdlib.h>  // for exit
#include <string.h>
#include <unistd.h>

#include "../include/defaults.h"
#include "../include/logger.h"
#include "../include/utils.h"

/**
 * HTTP responses
 */
const char *http_status_ok = "HTTP/1.1 200 OK";
const char *http_status_not_found = "HTTP/1.1 404 NOT FOUND";

/**
 * Example response html
 */
char *example_html_response =
    "<html><h1><center><b>It could be working!</b></center></h1></html>\r\n";

/**
 * Send an HTTP response
 *
 * header:       "HTTP/1.1 404 NOT FOUND" or "HTTP/1.1 200 OK", etc.
 * content_type: "text/plain", etc.
 * body:         the data to send.
 *
 * Return the value from the send() function.
 */
int send_response(int fd, const char *header, char *content_type, void *body, int content_length) {
    char response[MAX_RESPONSE_SIZE];

    const char *server_date = get_server_date();

    int response_length = sprintf(response,
                                  "%s\r\n"
                                  "Date: %s\r\n"
                                  "Server: %s\r\n"
                                  "Content-Length: %d\r\n"
                                  "Content-Type: %s\r\n"
                                  "\r\n",
                                  header, server_date, APP_NAME, content_length, content_type);
    memcpy(response + response_length, body, content_length);
    return send(fd, response, response_length + content_length, 0);
}

// Function to display the help message
void usage() {
    printf("Usage: %s [options]\n\n", APP_NAME);
    printf("Options:\n");
    printf("  -h, --help\t\tDisplay this help message\n");
    printf("  -d, --debug\t\tEnable debug mode\n");
    printf("  -v, --version\tPrint version number and exit\n");
    printf("  -p, --port <port>\tSet the port number (default: 9090)\n");
}

int main(int argc, char *argv[]) {
    log_info("Starting %s v%s", APP_NAME, APP_VERSION);

    // Socket creation

    // Use IPv4 (AF_INET) domain with sequenced, reliable 2-way
    // type (SOCK_STREAM) and default protocol 0 (IPPROTO_TCP)
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sockfd == -1) {
        log_error("Error creating socket");
        exit(EXIT_FAILURE);
    }
    log_info("Successfully created socket: sockfd: %d", sockfd);

    // Binding socket to an addr
    struct sockaddr_in host_addr;
    int host_addrlen = sizeof(host_addr);

    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(DEFAULT_PORT);
    host_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // INADDR_ANY refers 0.0.0.0
                                                    // for accepting any incoming message
    if (bind(sockfd, (struct sockaddr *)&host_addr, host_addrlen) != 0) {
        log_error("Error binding socket");
        exit(EXIT_FAILURE);
    }
    log_info("Successfully bound socket to addr: %d", INADDR_ANY);

    // Listen to socket in passive mode
    if (listen(sockfd, SOMAXCONN) != 0) {
        log_error("Error setting listen mode");
        exit(EXIT_FAILURE);
    }

    log_info("Server now listening on socket for incoming connections");

    char read_buffer[MAX_BUFFER];

    struct sockaddr_in client_addr;
    ssize_t client_addr_len = sizeof(client_addr);

    while (1) {
        // Accept incoming connection
        int connfd = accept(sockfd, (struct sockaddr *)&host_addr, (socklen_t *)&host_addrlen);

        if (connfd == -1) {
            log_error("Error accepting incoming connection");
            continue;
        }

        int sockname =
            getsockname(connfd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_len);
        if (sockname < 0) {
            log_error("Error reading client addr");
            continue;
        }
        log_info("Accepted new incoming connection from: %s", inet_ntoa(client_addr.sin_addr));

        ssize_t r = read(connfd, read_buffer, MAX_BUFFER);
        if (r < 0) {
            log_error("Error reading from sock");
            continue;
        }
        log_info("Read bytes: %ld", r);

        char method[MAX_BUFFER], uri[MAX_BUFFER], proto[MAX_BUFFER];

        sscanf(read_buffer, "%s %s %s", method, uri, proto);
        log_info("Request: method: %s uri: %s proto: %s", method, uri, proto);

        int w = send_response(connfd, http_status_ok, "text/html", example_html_response,
                              strlen(example_html_response));
        if (w < 0) {
            log_error("Error sending response");
            continue;
        }

        // Close connfd
        if (close(connfd) != 0) {
            log_error("Error closing connection");
            if (fsync(connfd) != 0) {
                log_error("Error in flushing data");
            }
            continue;
        }
    }
    exit(EXIT_SUCCESS);
}
