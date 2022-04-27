//
// Created by yarmus on 4/27/22.
//
#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>
#include <fcntl.h>
#include "common_sockets.hpp"


int handle_connection(int listener, bool is_blocking) {
    struct sockaddr_storage ss;
    socklen_t slen = sizeof(ss);
    auto socket_descriptor = accept(listener, (struct sockaddr *) &ss, &slen);
    if (socket_descriptor < 0) {
        std::cerr << "Error accepting request" << std::endl;
        exit(1);
    }
    if (!is_blocking) {
        // making socket nonblocking
        fcntl(socket_descriptor, F_SETFL, O_NONBLOCK);
    }
    return socket_descriptor;
}


int create_listener(bool is_blocking, int SERVER_PORT) {
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    // convert into big endian byte order
    sin.sin_port = htons(SERVER_PORT);
    auto listener = socket(AF_INET, SOCK_STREAM, 0);
    auto err = fcntl(listener, F_SETFL, O_NONBLOCK);
    if (err < 0) {
        std::cerr << "Could not make the listener socket non-blocking" << std::endl;
        exit(1);
    }

    auto on = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    err = bind(listener, (struct sockaddr* ) &sin, sizeof(sin));
    if (err < 0) {
        std::cerr << "Could not bind a listener socket" << std::endl;
        exit(1);
    }

    // making listener nonblocking
    if (!is_blocking)
        fcntl(listener, F_SETFL, O_NONBLOCK);

    return listener;
}