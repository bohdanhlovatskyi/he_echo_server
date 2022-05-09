/*
 * As there is no C++ socket API, we will stick to the C POSIX API
 */

#include <array>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "servers.hpp"

void Syncronous::init() {
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    // convert into big endian byte order
    sin.sin_port = htons(port);

    listener_fd = socket(AF_INET, SOCK_STREAM, 0);
    int on = 1;
    setsockopt(listener_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    auto err = bind(listener_fd, (struct sockaddr* ) &sin, sizeof(sin));
    if (err < 0) {
        throw std::runtime_error{"Could not bind a listener socket"};
    }

    err = listen(listener_fd, SOMAXCONN);
    if (err < 0) {
        throw std::runtime_error{"Listener could not start to listen"};
    }
}

void Syncronous::run() {
    for (;;) {
        struct sockaddr_storage ss;
        socklen_t slen = sizeof(ss);
        auto socket_descriptor = accept(listener_fd, (struct sockaddr*)&ss, &slen);
        if (socket_descriptor < 0) {
            throw std::runtime_error{"Error accepting request"};
        }

        int bytes_read = 1;
        // [TODO]: consider sort of while loop
        // MSG_DONTWAIT - not standard verbose, use fcntl()
        bytes_read = recv(socket_descriptor, &buf, BUF_SIZE, 0);
        if (bytes_read > 0) {
            send(socket_descriptor, buf.cbegin(), bytes_read, 0);
        }

        // [TODO]: works on kernel descriptors, thus affcets
        // all the descriptors of the process (???????)
        auto err = shutdown(socket_descriptor, SHUT_RDWR);
        if (err < 0) {
            std::cerr << "Error shutting down a socket" << std::endl;
        }

        close(socket_descriptor);
    }
}
