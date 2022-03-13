
/*
 * As there is no C++ socket API, we will stick to the C POSIX API
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include <iostream>
#include <array>

constexpr short SERVER_PORT = 9000;
constexpr short BUF_SIZE = 1024;

int main(int argc, char* argv[]) {
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    // convert into big endian byte order
    sin.sin_port = htons(SERVER_PORT);

    auto listener = socket(AF_INET, SOCK_STREAM, 0);
    auto on = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    auto err = bind(listener, (struct sockaddr* ) &sin, sizeof(sin));
    if (err < 0) {
        std::cerr << "Could not bind a listener socket" << std::endl;
        exit(1);
    }

    err = listen(listener, SOMAXCONN);
    if (err < 0) {
        std::cerr << "Listener could not start to listen" << std::endl;
        exit(1);
    }

    std::array<char, BUF_SIZE> buf{{}};
    for (;;) {
        struct sockaddr_storage ss;
        socklen_t slen = sizeof(ss);
        auto socket_descriptor = accept(listener, (struct sockaddr*)&ss, &slen);
        if (socket_descriptor < 0) {
            std::cerr << "Error accepting request" << std::endl;
            exit(1);
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
        err = shutdown(socket_descriptor, SHUT_RDWR);
        if (err < 0) {
            std::cerr << "Error shutting down a socket" << std::endl;
        }

        close(socket_descriptor);
    }
}