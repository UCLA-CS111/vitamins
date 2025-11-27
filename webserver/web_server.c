#define _GNU_SOURCE
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "libhttp.h"

static bool respond_bad_request(int connection_fd) {
    return http_start_response(connection_fd, 400) &&
           http_send_header(connection_fd, "Content-Type", "text/html") &&
           http_end_headers(connection_fd) &&
           http_send_string(connection_fd,
                            "<center><h1>400 Bad Request</h1><hr></center>");
}

static bool respond_forbidden(int connection_fd) {
    return http_start_response(connection_fd, 403) &&
           http_send_header(connection_fd, "Content-Type", "text/html") &&
           http_end_headers(connection_fd) &&
           http_send_string(connection_fd,
                            "<center><h1>403 Forbidden</h1><hr></center>");
}

static bool respond_not_found(int connection_fd) {
    return http_start_response(connection_fd, 404) &&
           http_send_header(connection_fd, "Content-Type", "text/html") &&
           http_end_headers(connection_fd) &&
           http_send_string(connection_fd,
                            "<center><h1>404 Not Found</h1><hr></center>");
}

static void handle_request(int connection_fd, const char *website_directory) {
    struct http_request *request = http_request_parse(connection_fd);

    /* Request path must start with "/". */
    if (request == NULL || request->path[0] != '/') {
        respond_bad_request(connection_fd);
        free(request);
        return;
    }

    /* Basic security mechanism: no requests may contain ".." in the path. */
    if (strstr(request->path, "..") != NULL) {
        respond_forbidden(connection_fd);
        free(request);
        return;
    }

    /* In the starter code, just serve response for "file not found." */
    respond_not_found(connection_fd);
}

#ifdef WEB_SERVER_PLAIN

static void handle_connection(int server_socket, int connection_socket,
                              const char *website_directory) {
}

#endif

#ifdef WEB_SERVER_PROCESSES

static void handle_connection(int server_socket, int connection_socket,
                              const char *website_directory) {
}

#endif

#ifdef WEB_SERVER_THREADS

static void handle_connection(int server_socket, int connection_socket,
                              const char *website_directory) {
}

#endif

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: %s <port> <website_directory>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int port = atoi(argv[1]);
    if (port == 0 || port > 65535) {
        fprintf(stderr, "invalid port\n");
        return EXIT_FAILURE;
    }

    int server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }

    int socket_option = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &socket_option,
                   sizeof(socket_option)) != 0) {
        perror("setsockopt(SO_REUSEADDR)");
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons((uint16_t) port);
    if (bind(server_socket, (struct sockaddr *) &server_address,
             sizeof(server_address)) != 0) {
        perror("bind");
        return EXIT_FAILURE;
    }

    for (;;) {
        struct sockaddr_in client_address;
        socklen_t client_address_length = sizeof(client_address);
        int connection_socket =
            accept(server_socket, (struct sockaddr *) &client_address,
                   &client_address_length);
        if (connection_socket < 0) {
            perror("accept");
            return EXIT_FAILURE;
        }

        char address_string_buffer[16];

        if (inet_ntop(AF_INET, &client_address.sin_addr,
                      &address_string_buffer[0],
                      sizeof(address_string_buffer)) == NULL) {
            perror("inet_ntop");
            strcpy(&address_string_buffer[0], "<unknown>");
        }

        printf("Accepted connection from %s:%d\n", &address_string_buffer[0],
               (int) ntohs(client_address.sin_port));

        handle_connection(server_socket, connection_socket, argv[2]);
    }

    return EXIT_SUCCESS;
}
