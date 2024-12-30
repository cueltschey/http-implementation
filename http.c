#include "http.h"
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

void *client_handler(void *arg) {
  int client_fd = *((int *)arg);
  char *buffer = (char *)malloc(MAX_BUFFER_SIZE * sizeof(char));

  // receive request data from client and store into buffer
  ssize_t bytes_received = recv(client_fd, buffer, MAX_BUFFER_SIZE, 0);
  printf("Request: %s", buffer);
  return (void *)bytes_received;
}
