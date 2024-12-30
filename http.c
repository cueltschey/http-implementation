#include "http.h"
#include <stddef.h>
#include <stdio.h>

void handle_error(int status_code, const char *err) {
  if (status_code < 0) {
    perror(err);
    exit(EXIT_FAILURE);
  }
}

const char *file_ext_from_filename(const char *filename) {
  const char *dot = strrchr(filename, '.');
  if (!dot || dot == filename)
    return "";
  return dot + 1;
}

mime_type get_mime_type(const char *file_ext) {
  if (strcasecmp(file_ext, "html") == 0 || strcasecmp(file_ext, "htm") == 0) {
    return MIME_TEXT_HTML;
  } else if (strcasecmp(file_ext, "txt") == 0) {
    return MIME_TEXT_PLAIN;
  } else if (strcasecmp(file_ext, "jpg") == 0 ||
             strcasecmp(file_ext, "jpeg") == 0) {
    return MIME_IMAGE_JPEG;
  } else if (strcasecmp(file_ext, "png") == 0) {
    return MIME_IMAGE_PNG;
  } else {
    return MIME_APPLICATION_OCTET;
  }
}

bool file_exists(const char *file_path, const char *root_dir) {
  DIR *dir = opendir(root_dir);
  if (dir == NULL) {
    perror("opendir");
    return NULL;
  }

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, file_path)) {
      closedir(dir);
      return true;
    }
  }

  closedir(dir);
  return false;
}

char *url_decode(const char *src) {
  size_t src_len = strlen(src);
  char *decoded = malloc(src_len + 1);
  size_t decoded_len = 0;

  // decode %2x to hex
  for (size_t i = 0; i < src_len; i++) {
    if (src[i] == '%' && i + 2 < src_len) {
      int hex_val;
      sscanf(src + i + 1, "%2x", &hex_val);
      decoded[decoded_len++] = hex_val;
      i += 2;
    } else {
      decoded[decoded_len++] = src[i];
    }
  }

  // add null terminator
  decoded[decoded_len] = '\0';
  return decoded;
}

void http_response_file(const char *file_name, const char *file_ext,
                        char *response, size_t *response_len) {

  // Set error 404, in case reading fails
  snprintf(response, MAX_BUFFER_SIZE,
           "HTTP/1.1 404 Not Found\r\n"
           "Content-Type: text/plain\r\n"
           "\r\n"
           "404 File Not Found: %s",
           file_name);

  *response_len = strlen(response);

  if (!file_exists(file_name, ".")) {
    perror("File not found");
    return;
  }

  int file_fd;
  if ((file_fd = open(file_name, O_RDONLY)) < 0) {
    perror("Open failed");
    return;
  }
  struct stat file_stat;
  fstat(file_fd, &file_stat);

  mime_type mime = get_mime_type(file_ext);
  char *header = (char *)malloc(MAX_BUFFER_SIZE * sizeof(char));
  snprintf(header, MAX_BUFFER_SIZE,
           "HTTP/1.1 200 OK\r\n"
           "Content-Type: %s\r\n"
           "\r\n",
           mime_type_txt[mime]);

  *response_len = 0;
  memcpy(response, header, strlen(header));
  *response_len += strlen(header);

  // copy file to response buffer
  ssize_t bytes_read;
  while ((bytes_read = read(file_fd, response + *response_len,
                            MAX_BUFFER_SIZE - *response_len)) > 0) {
    *response_len += bytes_read;
  }
  free(header);
  close(file_fd);
}

void *client_handler(void *arg) {
  int client_fd = *((int *)arg);
  char *buffer = (char *)malloc(MAX_BUFFER_SIZE * sizeof(char));
  ssize_t bytes_received = recv(client_fd, buffer, MAX_BUFFER_SIZE, 0);

  if (bytes_received <= 0) {
    perror("Recieve failed");
    return NULL;
  }

  // Regex for a GET request
  regex_t regex;
  regcomp(&regex, "^GET /([^ ]*) HTTP/1", REG_EXTENDED);
  regmatch_t matches[2];
  if (regexec(&regex, buffer, 2, matches, 0) != 0) {
    printf("Not a GET request ... ignoring\n");
    return NULL;
  }

  buffer[matches[1].rm_eo] = '\0';
  const char *url_encoded_file_name = buffer + matches[1].rm_so;
  char *file_name = url_decode(url_encoded_file_name);

  char file_ext[32];
  strcpy(file_ext, file_ext_from_filename(file_name));

  char *response = (char *)malloc(MAX_BUFFER_SIZE * 2 * sizeof(char));
  size_t response_len;
  http_response_file(file_name, file_ext, response, &response_len);

  // send HTTP response to client
  send(client_fd, response, response_len, 0);

  free(response);
  free(file_name);
  free(arg);
  free(buffer);

  regfree(&regex);
  close(client_fd);

  return NULL;
}
