#define _GNU_SOURCE

#include "libhttp.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LIBHTTP_REQUEST_MAX_SIZE 8192

struct http_request *http_request_parse(int fd) {
    struct http_request *request = malloc(sizeof(struct http_request));
    if (request == NULL) {
        return NULL;
    }

    char *read_buffer = malloc(LIBHTTP_REQUEST_MAX_SIZE + 1);
    if (!read_buffer) {
        free(request);
        return NULL;
    }

    int bytes_read = read(fd, read_buffer, LIBHTTP_REQUEST_MAX_SIZE);
    read_buffer[bytes_read] = '\0'; /* Always null-terminate. */

    char *read_start, *read_end;
    size_t read_size;

    do {
        /* Read in the HTTP method: "[A-Z]*" */
        read_start = read_end = read_buffer;
        while (*read_end >= 'A' && *read_end <= 'Z') {
            read_end++;
        }
        read_size = read_end - read_start;
        if (read_size == 0) {
            break;
        }
        request->method = malloc(read_size + 1);
        memcpy(request->method, read_start, read_size);
        request->method[read_size] = '\0';

        /* Read in a space character. */
        read_start = read_end;
        if (*read_end != ' ') {
            break;
        }
        read_end++;

        /* Read in the path: "[^ \n]*" */
        read_start = read_end;
        while (*read_end != '\0' && *read_end != ' ' && *read_end != '\n') {
            read_end++;
        }
        read_size = read_end - read_start;
        if (read_size == 0) {
            break;
        }
        request->path = malloc(read_size + 1);
        memcpy(request->path, read_start, read_size);
        request->path[read_size] = '\0';

        /* Read in HTTP version and rest of request line: ".*" */
        read_start = read_end;
        while (*read_end != '\0' && *read_end != '\n') {
            read_end++;
        }
        if (*read_end != '\n') {
            break;
        }
        read_end++;

        free(read_buffer);
        return request;
    } while (false);

    /* An error occurred. */
    free(request);
    free(read_buffer);
    return NULL;
}

char *http_get_response_message(int status_code) {
    switch (status_code) {
    case 100:
        return "Continue";
    case 200:
        return "OK";
    case 301:
        return "Moved Permanently";
    case 302:
        return "Found";
    case 304:
        return "Not Modified";
    case 400:
        return "Bad Request";
    case 401:
        return "Unauthorized";
    case 403:
        return "Forbidden";
    case 404:
        return "Not Found";
    case 405:
        return "Method Not Allowed";
    default:
        return "Internal Server Error";
    }
}

bool http_start_response(int fd, int status_code) {
    return dprintf(fd, "HTTP/1.0 %d %s\r\n", status_code,
                   http_get_response_message(status_code)) >= 0;
}

bool http_send_header(int fd, char *key, char *value) {
    return dprintf(fd, "%s: %s\r\n", key, value) >= 0;
}

bool http_end_headers(int fd) {
    return dprintf(fd, "\r\n") >= 0;
}

bool http_send_string(int fd, char *data) {
    return http_send_data(fd, data, strlen(data));
}

bool http_send_data(int fd, char *data, size_t size) {
    ssize_t bytes_sent;
    while (size > 0) {
        bytes_sent = write(fd, data, size);
        if (bytes_sent < 0) {
            return false;
        }
        size -= bytes_sent;
        data += bytes_sent;
    }
    return true;
}

char *http_get_mime_type(char *file_name) {
    char *file_extension = strrchr(file_name, '.');
    if (file_extension == NULL) {
        return "text/plain";
    }

    if (strcmp(file_extension, ".html") == 0 ||
        strcmp(file_extension, ".htm") == 0) {
        return "text/html";
    } else if (strcmp(file_extension, ".jpg") == 0 ||
               strcmp(file_extension, ".jpeg") == 0) {
        return "image/jpeg";
    } else if (strcmp(file_extension, ".png") == 0) {
        return "image/png";
    } else if (strcmp(file_extension, ".css") == 0) {
        return "text/css";
    } else if (strcmp(file_extension, ".js") == 0) {
        return "application/javascript";
    } else if (strcmp(file_extension, ".pdf") == 0) {
        return "application/pdf";
    } else {
        return "text/plain";
    }
}
