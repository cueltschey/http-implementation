
#include <arpa/inet.h>
#include <dirent.h>
#include <error.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MIME_COUNT 7
#define MAX_BUFFER_SIZE 104857600

typedef enum {
  MIME_TEXT_HTML,
  MIME_TEXT_PLAIN,
  MIME_IMAGE_JPEG,
  MIME_IMAGE_PNG,
  MIME_APPLICATION_JSON,
  MIME_APPLICATION_XML,
  MIME_APPLICATION_OCTET,
} mime_type;

void handle_error(int status_code, const char *err);

mime_type get_mime_type(const char *file_ext);

const char *get_mime_txt(mime_type t);

const char *file_ext_from_filename(const char *filename);

bool file_exists(const char *file_path, const char *root_dir);

void http_response_file(const char *file_name, const char *file_ext,
                        char *response, size_t *response_len);

void *client_handler(void *arg);
