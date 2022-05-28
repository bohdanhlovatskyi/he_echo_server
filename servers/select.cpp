/*
 * As there is no C++ socket API, we will stick to the C POSIX API
 */

#include "common_sockets.hpp"
#include "servers.hpp"

ssize_t AsyncSelect::read_(int fd, fd_state* state) {
    assert(state != nullptr);
    auto bytes_read = read_msg(fd, state->buf.data(), buf_size);
    if (bytes_read > 0) {
        state->wr = true;
        state->bytes_read = bytes_read;
    } else {
        std::cerr << std::strerror(errno) << std::endl;
    }

    return bytes_read;
}

ssize_t AsyncSelect::write_(int fd, fd_state* state) {
    assert(state != nullptr);
    auto bytes_written = write_msg(fd, state->buf.data(), state->bytes_read);
    state->wr = false;
    state->bytes_read = 0;
    return bytes_written;
}

void AsyncSelect::init() {
    assert(port != 0);
    listener_fd = create_listener(false, Server::port);

    for (size_t i = 0; i < FD_SETSIZE; ++i) {
        state[i] = nullptr;
    }

    FD_ZERO(&read_set);
    FD_ZERO(&write_set);
    FD_ZERO(&ex_set);
}

void AsyncSelect::run() {

    if (listener_fd == -1) {
        throw std::runtime_error{"you need to init the server"};
    }

    for (;;) {
        auto max_fd = listener_fd;
        FD_ZERO(&read_set);
        FD_ZERO(&write_set);
        FD_ZERO(&ex_set);
        // add listener set to the set of sockets that are ready to read
        FD_SET(listener_fd, &read_set);

        for (size_t i = 0; i < FD_SETSIZE; ++i) {
            if (state[i]) {
                if (i > static_cast<size_t>(max_fd))
                    max_fd = i;
                if (state[i]->wr)
                    FD_SET(i, &write_set);
                else
                    FD_SET(i, &read_set);
            }
        }

        // null ptr - blocks
        auto err = select(max_fd + 1, &read_set, &write_set, &ex_set, NULL);
        if (err < 0) {
            throw std::runtime_error{std::strerror(errno)};
        }

        // new connection accepted
        if (FD_ISSET(listener_fd, &read_set)) {
            auto socket_descriptor = handle_connection(listener_fd, false);
            state[socket_descriptor] = new AsyncSelect::fd_state();
            state[socket_descriptor]->buf.reserve(buf_size);
        }

        for (size_t i = 0; i < static_cast<size_t>(max_fd) + 1; ++i) {
            if (i == static_cast<size_t>(listener_fd)) continue;

            if (FD_ISSET(i, &read_set)) {
                read_(i, state[i]);
            }

            if (FD_ISSET(i, &write_set)) {
                auto bytes_written = write_(i, state[i]);
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