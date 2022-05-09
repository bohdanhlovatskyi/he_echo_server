/*
 * As there is no C++ socket API, we will stick to the C POSIX API
 */

#include "common_sockets.hpp"
#include "servers.hpp"

void BlockingMultiProcess::init() {
    assert(port != 0);
    listener_fd = create_listener(true, Server::port);
}

void BlockingMultiProcess::run() {

    if (listener_fd == -1) {
        throw std::runtime_error{"you need to init the server"};
    }

    for (;;) {
        auto sd = handle_connection(listener_fd, true);

        if (fork() == 0) {
            close(listener_fd);

            std::vector<char> buf;
            buf.reserve(buf_size);

            for (;;) {
                auto bytes_read = read_msg(sd, buf.data(), buf_size);
                if (bytes_read > 0) {
                    write_msg(sd, buf.data(), buf_size);
                } else {
                    break;
                }
            }

            exit(0);
        } else {
            // Note that we can also send error message using this socket descriptor to the client
            close(sd);
            // [TODO] : check where socket is allocated(we two times close it)
        }
    }
}
