/*
 * A simple HTTP library.
 *
 * Usage example:
 *
 *     // Returns NULL if an error was encountered.
 *     struct http_request *request = http_request_parse(fd);
 *
 *     ...
 *
 *     http_start_response(fd, 200);
 *     http_send_header(fd, "Content-type", http_get_mime_type("index.html"));
 *     http_send_header(fd, "Server", "httpserver/1.0");
 *     http_end_headers(fd);
 *     http_send_string(fd, "<html><body><a href='/'>Home</a></body></html>");
 *
 *     close(fd);
 */

#ifndef LIBHTTP_H
#define LIBHTTP_H

#include <stddef.h>
#include <stdbool.h>

/*
 * Functions for parsing an HTTP request.
 */
struct http_request {
    char *method;
    char *path;
};

struct http_request *http_request_parse(int fd);

/*
 * Functions for sending an HTTP response.
 * Each one returns true on success, and false on failure.
 */
bool http_start_response(int fd, int status_code);
bool http_send_header(int fd, char *key, char *value);
bool http_end_headers(int fd);
bool http_send_string(int fd, char *data);
bool http_send_data(int fd, char *data, size_t size);

/*
 * Helper function: gets the Content-Type based on a file name.
 */
char *http_get_mime_type(char *file_name);

#endif
