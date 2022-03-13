
/*
 * As there is no C++ socket API, we will stick to the C POSIX API
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>

#include <iostream>
#include <array>
#include <cerrno>

constexpr short SERVER_PORT = 9000;
constexpr short BUF_SIZE = 1024;

fd_set read_set, write_set, ex_set;

struct fd_state {
    std::array<char, BUF_SIZE> buf;
    bool wr;

    fd_state(): buf{}, wr{false} {};
    ~fd_state() = default;
};

int read(int fd, fd_state* state) {
    recv(fd, &state->buf, sizeof(state->buf), 0);
    state->wr = true;

    return 0;
}

int write(int fd, fd_state* state) {
    send(fd, &state->buf, sizeof(state->buf), 0);
    state->wr = false;

    return 0;
}


int main(int argc, char* argv[]) {
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

    err = listen(listener, SOMAXCONN);
    if (err < 0) {
        std::cerr << "Listener could not start to listen" << std::endl;
        exit(1);
    }

    // for each of the sockets, remember its state: buffers,
    // state of the communication that is currently going on
    fd_state* state[FD_SETSIZE];
    for (size_t i = 0; i < FD_SETSIZE; ++i)
        state[i] = nullptr;
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
            if (state[i] == nullptr) {
                continue;;
            }

            max_fd = i > max_fd ? i : max_fd;
            FD_SET(i, &read_set);
            if (state[i]->wr) {
                FD_SET(i, &write_set);
            }
        }

        // null ptr - blocks
        err = select(max_fd + 1, &read_set, &write_set, &ex_set, NULL);
        if (err < 0) {
            std::cerr << "select err: " << std::strerror(errno) << std::endl;
            exit(1);
        }

        // new connection accepted
        if (FD_ISSET(listener, &read_set)) {
            struct sockaddr_storage ss;
            socklen_t slen = sizeof(ss);
            auto socket_descriptor = accept(listener, (struct sockaddr*)&ss, &slen);
            if (socket_descriptor < 0) {
                std::cerr << "Error accepting request" << std::endl;
                exit(1);
            } else {
                // note that the socket descriptor is already in the reading set
                fcntl(socket_descriptor, F_SETFL, O_NONBLOCK);
                state[socket_descriptor] = new fd_state();
            }
        }

        for (size_t i = 0; i < max_fd + 1; ++i) {
            if (i == listener) continue;

            if (FD_ISSET(i, &read_set))
                read(i, state[i]);

            if (FD_ISSET(i, &write_set)) {
                write(i, state[i]);
                state[i] = nullptr;
                // close the connection
                shutdown(i, SHUT_RD);
                close(i);
            }
        }

    }
}