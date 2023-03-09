/**
 * @file main.cc
 * @author Tang Jiapeng (tangjiapeng0215@gmail.com)
 * @brief
 * @version 0.1
 * @date 2023-03-09
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <thread>

#include "../include/Server.h"
#include "../include/Timer.h"

#define PORT 2352

int socketBindListen(int port) {
  if (port < 0 || port > 65535) {
    return -1;
  }
  int listen_fd = 0;
  if ((listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
    return -1;
  }
  int optval = 1;
  if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval,
                 sizeof(optval)) == -1) {
    close(listen_fd);
    return -1;
  }
  struct sockaddr_in server_addr;
  bzero((char *)&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons((unsigned short)port);
  if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) ==
      -1) {
    close(listen_fd);
    return -1;
  }
  if (listen(listen_fd, 2048) == -1) {
    close(listen_fd);
    return -1;
  }
  if (listen_fd == -1) {
    close(listen_fd);
    return -1;
  }
  return listen_fd;
}

int main(int argc, char *argv[]) {
  int port = PORT;
  int opt;
  const char *str = "p:";
  while ((opt = getopt(argc, argv, str)) != -1) {
    switch (opt) {
      case 'p': {
        port = atoi(optarg);
        break;
      }
      case 'v': {
        printf("skiplist-store version 1.0 \n");
        break;
      }
      default: {
        printf("Usage: %s [-p port] [-v] \n", argv[0]);
      }
    }
  }
  int listen_fd = socketBindListen(port);
  struct sockaddr_in client_addr;
  memset(&client_addr, 0, sizeof(struct sockaddr_in));
  socklen_t client_addr_len = sizeof(client_addr);
  int accept_fd = 0;
  Timer myTimer;
  myTimer.start(5, []() { SkipListWorker::mydb_.dumpFile(); });
  while ((accept_fd = accept(listen_fd, (struct sockaddr *)&client_addr,
                             &client_addr_len)) > 0) {
    std::thread workThread(SkipListWorker::threadFunc, accept_fd);
    workThread.detach();
  }
  myTimer.stop();
}