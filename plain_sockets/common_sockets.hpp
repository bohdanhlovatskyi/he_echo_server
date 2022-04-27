//
// Created by yarmus on 4/27/22.
//

#ifndef ASYNC_MULTI_COMMON_SOCKETS_HPP
#define ASYNC_MULTI_COMMON_SOCKETS_HPP

int handle_connection(int listener, bool is_blocking);

int create_listener(bool is_blocking, int SERVER_PORT);

#endif //ASYNC_MULTI_COMMON_SOCKETS_HPP
