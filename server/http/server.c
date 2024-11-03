#include <stdio.h>
#include <error.h>
#include <netinet/ip.h> // contains socket.h via in.h
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#define DEFAULT_PORT 9090
#define MAX_BUFFER 2048

char resp[] = "HTTP/1.1 200 OK\r\n"
"Server: Webby\r\n"
"Content-type: text/html\r\n\r\n"
"<html><b>It may be working!</b></html>\r\n";

int main(int argc, char *argv[])
{
    printf("Webby\n");

    // Socket creation

    // Use IPv4 (AF_INET) domain with sequenced, reliable 2-way
    // type (SOCK_STREAM) and default protocol 0 (IPPROTO_TCP)
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    if(sockfd == -1) {
        perror("Error creating socket");
        return 1;
    }
    printf("Successfully created socket: sockfd: %d \n", sockfd);

    // Binding socket to an addr

    struct sockaddr_in host_addr;
    int host_addrlen = sizeof(host_addr);
    
    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(DEFAULT_PORT);
    host_addr.sin_addr.s_addr = htonl(INADDR_ANY); // INADDR_ANY refers 0.0.0.0
                                                   // for accepting any incoming message
    if(bind(sockfd, (struct sockaddr *)&host_addr, host_addrlen) != 0) {
        perror("Error binding socket");
        return 1;
    }
    printf("Successfully bound socket to addr: %d \n", INADDR_ANY);

    // Listen to socket in passive mode
    if(listen(sockfd, SOMAXCONN) != 0) {
        perror("Error setting listen mode");
        return 1;
    }

    printf("Server now listening on socket for incoming connections\n");

    char read_buffer[MAX_BUFFER];

    struct sockaddr_in client_addr;
    ssize_t client_addr_len = sizeof(client_addr);

    while(1) {
        // Accept incoming connection
        int connfd = accept(sockfd, (struct sockaddr *)&host_addr, (socklen_t *)&host_addrlen);

        if(connfd == -1) {
            perror("Error accepting incoming connection");
            continue;
        }

        int sockname = getsockname(connfd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_len);
        if(sockname < 0) {
            perror("Error reading client addr");
            continue;
        }
        printf("Accepted new incoming connection from: %s\n", inet_ntoa(client_addr.sin_addr));

        ssize_t r = read(connfd, read_buffer, MAX_BUFFER); 
        if(r < 0) {
            perror("Error reading from sock");
            continue;
        }
        printf("Read bytes: %ld\n", r);

        int w = write(connfd, resp, strlen(resp));
        if(w < 0) {
            perror("Error writing response");
            continue;
        }

        // Close connfd
        if(close(connfd) != 0) {
            perror("Error closing connection");
            if(fsync(connfd) != 0) {
                perror("Error in flushing data");
            }
            continue;
        }
    }
    return 0;
}
