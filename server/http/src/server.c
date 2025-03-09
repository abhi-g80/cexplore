#include <arpa/inet.h>
#include <error.h>
#include <fcntl.h>
#include <getopt.h>
#include <netinet/ip.h>  // contains socket.h via in.h
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>  // for exit
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "defaults.h"
#include "logger.h"
#include "requests.h"
#include "response.h"

int DEBUG_F = 0;
char WEBBY_ROOT[MAX_BUFFER];

/*
 * Set the fd as non-blocking but keep the existing options
 */
void setnonblocking(int fd) { fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK); }

/**
 * Simple signal handler.
 * Log an message and exit with EXIT_SUCCESS.
 */
void termination_handler(int signum) {
    log_info("Shutting down the server");
    exit(EXIT_SUCCESS);
}

/**
 * Signal handler which should handle both SIGINT and SIGTERM.
 */
void setup_signal_handler() {
    struct sigaction new_action, old_action;

    new_action.sa_handler = termination_handler;
    sigemptyset(&new_action.sa_mask);

    new_action.sa_flags = 0;

    sigaction(SIGINT, NULL, &old_action);
    sigaction(SIGTERM, NULL, &old_action);

    if (old_action.sa_handler != SIG_IGN) {
        sigaction(SIGINT, &new_action, NULL);
        sigaction(SIGTERM, &new_action, NULL);
    }
}

/**
 * Setup the root location of the website
 */
void setup_webby_root() {
    char *wbr = getenv("WEBBY_ROOT");
    if (wbr == NULL) {
        fprintf(stderr, "Please set WEBBY_ROOT environment variable\n");
        exit(EXIT_FAILURE);
    }
    strcpy(WEBBY_ROOT, wbr);
}

/**
 * Create a socket, bind it on INADDR_ANY with the given port and
 * start listening. Return the created socket fd.
 */
int setup_socket(int port) {
    // Socket creation

    // Use IPv4 (AF_INET) domain with sequenced, reliable 2-way
    // type (SOCK_STREAM) and default protocol 0 (IPPROTO_TCP)
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sockfd == -1) {
        log_error("Error creating socket");
        exit(EXIT_FAILURE);
    }
    log_debug("Successfully created socket: sockfd: %d", sockfd);

    int opt = 1;  // For setting sock options

    // Get rid of the "Address already in use" error when binding socket
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        log_error("Could not set socket options");
        exit(EXIT_FAILURE);
    }

    // Binding socket to an addr
    struct sockaddr_in host_addr;
    int host_addrlen = sizeof(host_addr);

    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(port);
    host_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // INADDR_ANY refers 0.0.0.0
                                                    // for accepting any incoming message
    if (bind(sockfd, (struct sockaddr *)&host_addr, host_addrlen) != 0) {
        log_error("Error binding socket");
        exit(EXIT_FAILURE);
    }
    log_debug("Successfully bound socket to addr: %d", INADDR_ANY);

    // Listen to socket in passive mode
    if (listen(sockfd, SOMAXCONN) != 0) {
        log_error("Error setting listen mode");
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

/**
 * Naive implementation for reading all headers
 *
 * @read_buffer: buffer to read from
 * @start      : index to start reading from
 * @stop       : max index to read until
 */
void read_all_headers(char *read_buffer, int start, int stop) {
    char buf[MAX_BUFFER];
    char k[MAX_BUFFER], v[MAX_BUFFER];
    int key_found = 0;

    for (int i = start, j = 0; i < stop; i++) {
        if ((read_buffer[i - 1] == '\r') && (read_buffer[i] == '\n')) {
            memset(v, '\0', sizeof(char) * MAX_BUFFER);
            strcpy(v, buf);
            log_debug("Header: %s => %s", k, v);
            memset(buf, '\0', sizeof(char) * MAX_BUFFER);
            key_found = 0, j = 0;
            if ((read_buffer[i] == '\n') && (read_buffer[i + 1] == '\r')) {
                break;
            }
            continue;
        }
        buf[j++] = read_buffer[i];
        if ((buf[j - 1] == ':') && !key_found) {
            memset(k, '\0', sizeof(char) * MAX_BUFFER);
            strcpy(k, buf);
            key_found = 1, j = 0, i++;
            memset(buf, '\0', sizeof(char) * MAX_BUFFER);
        }
    }
}

int handle_client(int connfd) {
    char read_buffer[MAX_BUFFER] = {0};
    char content_type[30];
    ssize_t r = read(connfd, read_buffer, MAX_BUFFER);
    if (r < 0) {
        log_error("Error reading from sock");
        return 1;
    }
    log_debug("Read bytes: %ld", r);

    struct http_request_info hri;
    hri.fd = connfd;
    sscanf(read_buffer, "%s %s %s", hri.method, hri.uri, hri.proto);

    if (DEBUG_F) {
        int total_len = strlen(hri.method) + strlen(hri.uri) + strlen(hri.proto);
        read_all_headers(read_buffer, total_len + 4, r);
    }

    log_debug("Request info: method: %s uri: %s proto: %s", hri.method, hri.uri, hri.proto);
    if (strcmp(hri.method, "GET") == 0) {
        // handle get request
        memset(content_type, 0, sizeof(content_type));
        if (strstr(hri.uri, ".html") != NULL) {
            memcpy(content_type, "text/html", strlen("text/html"));
        } else {
            memcpy(content_type, "text/plain; charset=utf-8", strlen("text/plain; charset=utf-8"));
        }
        log_debug("Setting Content-type: %s", content_type);
        int w = send_text_response(connfd, content_type, &hri);
        if (w < 0) {
            log_error("Error sending html response: %d", w);
            return 1;
        }
    }
    return 0;
}

// Function to display the help message
void usage(char *bin) {
    printf("Usage: %s [options]\n\n", bin);
    printf("Webby - A small webserver\n\n");
    printf("Options:\n");
    printf("  -h, --help\t\tdisplay this help message\n");
    printf("  -d, --debug\t\tprint debug logs\n");
    printf("  -v, --version\t\tprint version number and exit\n");
    printf("  -p, --port <port>\tset the port number (default: 9090)\n");
}
// Print version
void version() { printf("%s v%s\n", APP_NAME, APP_VERSION); }

int main(int argc, char *argv[]) {
    int c;
    uint16_t port = DEFAULT_PORT;

    // clang-format off
    static struct option long_options[] = {
        {"help",    no_argument,       0, 'h'},
        {"debug",   no_argument,       0, 'd'},
        {"version", no_argument,       0, 'v'},
        {"port",    required_argument, 0, 'p'},
        {0,         0,                 0,  0 }
    };
    // clang-format on

    while (1) {
        int option_index = 0;

        c = getopt_long(argc, argv, "hvdp:0", long_options, &option_index);

        if (c == -1) break;

        switch (c) {
            case 'h':
                usage(argv[0]);
                exit(EXIT_SUCCESS);
            case 'v':
                version();
                exit(EXIT_SUCCESS);
            case 'd':
                DEBUG_F = 1;
                break;
            case 'p':
                port = strtol(optarg, NULL, 10);
                break;
            case '?':
                usage(argv[0]);
                exit(EXIT_FAILURE);
            default:
                printf("?? getopt returned char code: 0%o ??\n", c);
        }
    }

    setup_webby_root();
    setup_signal_handler();

    log_info("Starting %s v%s", APP_NAME, APP_VERSION);

    if (port == DEFAULT_PORT) log_info("Using default port: %d", port);

    int sockfd = setup_socket(port);

    struct epoll_event ev, events[MAX_EVENTS];
    int nfds;
    int epollfd = epoll_create1(0);
    if (epollfd == -1) {
        log_error("Could not create epoll fd");
        exit(EXIT_FAILURE);
    }
    ev.events = EPOLLIN;
    ev.data.fd = sockfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &ev) == -1) {
        log_error("epoll_ctl: listen sock: sockfd");
        exit(EXIT_FAILURE);
    }

    log_info("Server now listening for incoming connections on port: %d", port);

    struct sockaddr_in client_addr, host_addr;
    ssize_t client_addrlen = sizeof(client_addr);
    ssize_t host_addrlen = sizeof(host_addr);

    for (;;) {
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            log_error("epoll_wait: nfds:");
            exit(EXIT_FAILURE);
        }
        for (int n = 0; n < nfds; n++) {
            if (events[n].data.fd == sockfd) {
                int connfd =
                    accept(sockfd, (struct sockaddr *)&host_addr, (socklen_t *)&host_addrlen);
                if (connfd == -1) {
                    log_error("Error accepting incoming connection");
                    continue;
                }
                setnonblocking(connfd);
                ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
                ev.data.fd = connfd;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &ev) == -1) {
                    log_error("epoll_ctl: connfd");
                    exit(EXIT_FAILURE);
                }
            } else {
                int connfd = events[n].data.fd;
                int sockname = getsockname(connfd, (struct sockaddr *)&client_addr,
                                           (socklen_t *)&client_addrlen);
                if (sockname < 0) {
                    log_error("Error reading client addr");
                    return 1;
                }
                log_debug("Accepted new incoming connection from: %s",
                          inet_ntoa(client_addr.sin_addr));

                if (handle_client(connfd) != 0) {
                    log_error("Error handling client");
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
        }
    }

    exit(EXIT_SUCCESS);
}
