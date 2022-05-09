
/*
 * As there is no C++ socket API, we will stick to the C POSIX API
 */
#include <sys/socket.h>
#include <unistd.h>
#include <sys/select.h>

#include <iostream>
#include <array>
#include <cerrno>
#include <cstring>

#include "common_sockets.hpp"
constexpr short SERVER_PORT = 9000;
constexpr short BUF_SIZE = 1024;

fd_set read_set, write_set, ex_set;

struct fd_state {
    std::array<char, BUF_SIZE> buf;
    bool wr;
    ssize_t bytes_read;
    fd_state(): buf{}, wr{false}, bytes_read{0} {};
    ~fd_state() = default;
};

ssize_t read(int fd, fd_state* state) {
    auto bytes_read = read_msg(fd, state->buf.data(), BUF_SIZE);
    if (bytes_read > 0) {
        std::cout << "input msg: " << state->buf.data() << std::endl;
        state->wr = true;
        state->bytes_read = bytes_read;
    }
    return bytes_read;
}

ssize_t write(int fd, fd_state* state) {
    auto bytes_written = write_msg(fd, state->buf.data(), state->bytes_read);
    state->wr = false;
    state->bytes_read = 0;
    return bytes_written;
}


int main(int argc, char* argv[]) {
    auto listener = create_listener(false, SERVER_PORT);

    // for each of the sockets, remember its state: buffers,
    // state of the communication that is currently going on
    fd_state* state[FD_SETSIZE];
    for (size_t i = 0; i < FD_SETSIZE; ++i)
        state[i] = nullptr;

    fd_set read_set, write_set, ex_set;
    FD_ZERO(&read_set);
    FD_ZERO(&write_set);
    FD_ZERO(&ex_set);

    for (;;) {
        auto max_fd = listener;
        FD_ZERO(&read_set);
        FD_ZERO(&write_set);
        FD_ZERO(&ex_set);
        // add listener set to the set of sockets that are ready to read
        FD_SET(listener, &read_set);

        for (size_t i = 0; i < FD_SETSIZE; ++i) {
            if (state[i]) {
                if (i > max_fd)
                    max_fd = i;
                if (state[i]->wr)
                    FD_SET(i, &write_set);
                else
                    FD_SET(i, &read_set);
            }
        }

        // null ptr - blocks
        auto err = select(max_fd + 1, &read_set, &write_set, &ex_set, NULL);//syscall not like winda( Fara stattya. Stromno KQueL
        if (err < 0) {
            std::cerr << "select err: " << std::strerror(errno) << std::endl;
            exit(1);
        }

        // new connection accepted
        if (FD_ISSET(listener, &read_set)) {
            auto socket_descriptor = handle_connection(listener, false);
            state[socket_descriptor] = new fd_state();
        }

        for (size_t i = 0; i < max_fd + 1; ++i) {
            if (i == listener) continue;

            if (FD_ISSET(i, &read_set))
                read(i, state[i]);

            if (FD_ISSET(i, &write_set)) {
                auto bytes_written = write(i, state[i]);
                // close the connection when error
                if (bytes_written < 0) {
                    state[i] = nullptr;
                    close(i);
                    continue;
                }
            }
        }
    }
}