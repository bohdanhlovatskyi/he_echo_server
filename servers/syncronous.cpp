/*
 * As there is no C++ socket API, we will stick to the C POSIX API
 */

#include "servers.hpp"

void Syncronous::init() {
    listener_fd = create_listener(true, Server::port);
}

void Syncronous::run() {
    if (listener_fd == -1) {
        throw std::runtime_error{"you need to init the server"};
    }

    std::vector<char> buf;
    buf.reserve(buf_size);

    for (;;) {
        auto sd = handle_connection(listener_fd, true);
        for (;;) {
            auto bytes_read = read_msg(sd, buf.data(), buf_size);
            if (bytes_read > 0) {
                std::cout << "input msg: " << buf.data() << std::endl;
                write_msg(sd, buf.data(), bytes_read);
            } else {
                // closing socket when error
                close(sd);
                break;
            }
        }
    }
}
