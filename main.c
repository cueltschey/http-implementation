#include "http.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  int port;
  if (argc < 2) {
    perror("Usage: main <port>");
  }
  port = atol(argv[1]);
  int server_fd;
  struct sockaddr_in server_addr;

  // create server socket
  handle_error((server_fd = socket(AF_INET, SOCK_STREAM, 0)),
               "Socket creation failed");

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  handle_error(
      (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr))),
      "Bind failed");

  handle_error(listen(server_fd, 10), "Listen failed");

  printf("Listening on port: %d\n", port);

  while (1) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int *client_fd = malloc(sizeof(int));

    if ((*client_fd = accept(server_fd, (struct sockaddr *)&client_addr,
                             &client_addr_len)) < 0) {
      perror("accept failed");
      continue;
    }

    pthread_t request_thread;
    pthread_create(&request_thread, NULL, client_handler, (void *)client_fd);
    pthread_detach(request_thread);
  }

  return EXIT_SUCCESS;
}
