#include <arpa/inet.h>
#include <error.h>
#include <netinet/ip.h>  // contains socket.h via in.h
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "logger.h"

#define DEFAULT_PORT 9090
#define MAX_BUFFER 2048
#define APP_NAME "Webby"
#define APP_VERSION "0.1.0"

#define log_info(...) logger_f(INFO, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) logger_f(DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) logger_f(ERROR, __FILE__, __LINE__, __VA_ARGS__)

char resp[] =
    "HTTP/1.1 200 OK\r\n"
    "Server: Webby\r\n"
    "Content-type: text/html\r\n\r\n"
    "<html><b>It may be working!</b></html>\r\n";

int main(int argc, char *argv[]) {
    log_info("Starting %s v%s", APP_NAME, APP_VERSION);

    // Socket creation

    // Use IPv4 (AF_INET) domain with sequenced, reliable 2-way
    // type (SOCK_STREAM) and default protocol 0 (IPPROTO_TCP)
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sockfd == -1) {
        log_error("Error creating socket");
        perror(NULL);
        return 1;
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
        perror(NULL);
        return 1;
    }
    log_info("Successfully bound socket to addr: %d", INADDR_ANY);

    // Listen to socket in passive mode
    if (listen(sockfd, SOMAXCONN) != 0) {
        log_error("Error setting listen mode");
        perror(NULL);
        return 1;
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
            perror(NULL);
            continue;
        }

        int sockname =
            getsockname(connfd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_len);
        if (sockname < 0) {
            log_error("Error reading client addr");
            perror(NULL);
            continue;
        }
        log_info("Accepted new incoming connection from: %s", inet_ntoa(client_addr.sin_addr));

        ssize_t r = read(connfd, read_buffer, MAX_BUFFER);
        if (r < 0) {
            log_error("Error reading from sock");
            perror(NULL);
            continue;
        }
        log_info("Read bytes: %ld", r);

        char method[MAX_BUFFER], uri[MAX_BUFFER], proto[MAX_BUFFER];

        sscanf(read_buffer, "%s %s %s", method, uri, proto);
        log_info("Request: method: %s uri: %s proto: %s", method, uri, proto);

        int w = write(connfd, resp, strlen(resp));
        if (w < 0) {
            log_error("Error writing response");
            perror(NULL);
            continue;
        }

        // Close connfd
        if (close(connfd) != 0) {
            log_error("Error closing connection");
            perror(NULL);
            if (fsync(connfd) != 0) {
                log_error("Error in flushing data");
                perror(NULL);
            }
            continue;
        }
    }
    return 0;
}
