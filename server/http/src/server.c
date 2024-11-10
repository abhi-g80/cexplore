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
#include "utils.h"

/**
 * HTTP responses
 */
const char *http_status_ok = "HTTP/1.1 200 OK";
const char *http_status_not_found = "HTTP/1.1 404 NOT FOUND";

int DEBUG_F = 0;

/**
 * Example response html
 */
char *example_html_response =
    "<html><h1><center><b>It could be working!</b></center></h1></html>\r\n";

/**
 * Send an HTTP response.
 *
 * Return the value from the send() function.
 */
int send_response(int fd, const char *header, char *content_type, void *body, int content_length) {
    char response[MAX_RESPONSE_SIZE];

    char *server_date = get_server_date();

    int response_length = sprintf(response,
                                  "%s\r\n"
                                  "Date: %s\r\n"
                                  "Server: %s\r\n"
                                  "Content-Length: %d\r\n"
                                  "Content-Type: %s\r\n"
                                  "\r\n",
                                  header, server_date, APP_NAME, content_length, content_type);
    memcpy(response + response_length, body, content_length);
    free(server_date);
    return send(fd, response, response_length + content_length, 0);
}

void setnonblocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;

    fcntl(fd, F_SETFL, new_option);
}

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

int handle_client(int connfd) {
    char read_buffer[MAX_BUFFER] = {0};
    ssize_t r = read(connfd, read_buffer, MAX_BUFFER);
    if (r < 0) {
        log_error("Error reading from sock");
        return 1;
    }
    log_debug("Read bytes: %ld", r);

    char method[MAX_BUFFER], uri[MAX_BUFFER], proto[MAX_BUFFER];

    sscanf(read_buffer, "%s %s %s", method, uri, proto);

    log_debug("Request: method: %s uri: %s proto: %s", method, uri, proto);

    int w = send_response(connfd, http_status_ok, "text/html", example_html_response,
                          strlen(example_html_response));
    if (w < 0) {
        log_error("Error sending response");
        return 1;
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
