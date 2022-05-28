
/*
 * As there is no C++ socket API, we will stick to the C POSIX API
 */

#include "servers.hpp"
#include <mutex>

void BlockingMultiThreaded::init() {
    assert(port != 0);
    listener_fd = create_listener(true, Server::port);
}

void BlockingMultiThreaded::run() {

    if (listener_fd == -1) {
        throw std::runtime_error{"you need to init the server"};
    }

    for (;;) {
        auto sd = handle_connection(listener_fd, true);

        std::thread t([](auto sd, auto size, std::mutex& m){
            std::vector<char> buf;
            buf.reserve(size);

            for (;;) {
                auto bytes_read = read_msg(sd, buf.data(), size);
                if (bytes_read > 0) {
                    write_msg(sd, buf.data(), bytes_read);
                } else {
                    // closing socket when error
                    close(sd);
                    break;
                }
            }
        }, sd, buf_size, std::ref(m));

        t.detach();
    }
}
