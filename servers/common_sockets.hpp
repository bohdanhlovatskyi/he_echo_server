//
// Created by yarmus on 4/27/22.
//

#ifndef ASYNC_MULTI_COMMON_SOCKETS_HPP
#define ASYNC_MULTI_COMMON_SOCKETS_HPP

#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <cassert>
#include <cerrno>

ssize_t read_msg(int sd, char* buf, ssize_t buf_size);

ssize_t write_msg(int sd, const char* buf, ssize_t msg_size);

int handle_connection(int listener, bool is_blocking);

int create_listener(bool is_blocking, int SERVER_PORT);

void clean_up(int sd);

#endif //ASYNC_MULTI_COMMON_SOCKETS_HPP
