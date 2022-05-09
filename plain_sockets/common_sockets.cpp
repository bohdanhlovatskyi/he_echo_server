//
// Created by yarmus on 4/27/22.
//

#include "common_sockets.hpp"

ssize_t read_msg(int sd, char* buf, ssize_t buf_size) {
    ssize_t read_data;
    read_data = read(sd, buf, buf_size);
    if (read_data < 0)
        return read_data;
    if (read_data >= 0) {
        buf[read_data] = '\0';
    }
    return read_data;
}

ssize_t write_msg(int sd, const char* buf, ssize_t msg_size) {
    ssize_t already_written = 0;
    ssize_t written_now;

    while(already_written < msg_size) {
        written_now = write(sd, buf + already_written, msg_size - already_written);
        if (written_now < 0)
            // correspond to error in writing
            return written_now;
        already_written += written_now;
    }
    return already_written;
}

int handle_connection(int listener, bool is_blocking) {
    assert(listener != 0 || listener != -1);

    struct sockaddr_storage ss;
    socklen_t slen = sizeof(ss);
    auto socket_descriptor = accept(listener, (struct sockaddr *) &ss, &slen);
    if (socket_descriptor < 0) {
        std::cerr << std::strerror(errno) << std::endl;
        throw std::runtime_error{"listener : handle connection : Error accepting request"};
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

    auto on = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    auto err = bind(listener, (struct sockaddr* ) &sin, sizeof(sin));
    if (err < 0) {
        throw std::runtime_error{"Could not bind a listener socket"};
    }

    // making listener nonblocking
    if (!is_blocking) {
        auto err = fcntl(listener, F_SETFL, O_NONBLOCK);
        if (err < 0) {
            throw std::runtime_error{"Could not make the listener socket non-blocking"};
        }
    }

    err = listen(listener, SOMAXCONN);
    if (err < 0) {
        throw std::runtime_error{"Listener could not start to listen"};
    }

    return listener;
}

void clean_up(int sd) {
    auto err = shutdown(sd, SHUT_RDWR);
    if (err < 0) {
        std::cerr << "Error shutting down a socket" << std::endl;
    }

    close(sd);
}