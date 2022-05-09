//
// Created by yarmus on 4/26/22.
//
#include <sys/socket.h>
#include <unistd.h>
#include <sys/epoll.h>
#include "common_sockets.hpp"
#include <iostream>
#include <array>

constexpr short SERVER_PORT = 9000;
constexpr short BUF_SIZE = 1024;
constexpr short MAX_EVENTS = 32;

// register events of fd to epfd
void epoll_ctl_add(int epfd, int fd, uint32_t events);

int main() {
    std::array<char, BUF_SIZE> buf{{}};
    auto listener = create_listener(false, SERVER_PORT);

    int epfd = epoll_create(1);
    if (epfd == -1) {
        std::cerr << "We cannot create epoll instance" << std::endl;
        exit(1);
    }

    epoll_ctl_add(epfd, listener, EPOLLIN | EPOLLOUT | EPOLLET);

    struct epoll_event events[MAX_EVENTS];
    int nfds;
    for (;;) {
        // If timeout equals –1, block until an event occurs for one of the file descriptors in
        // the interest list for epfd or until a signal is caught.
        nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);

        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == listener) {
                // handle new connection
                auto socket_descriptor = handle_connection(listener, false);

                // adding new socket to epoll interest list
                epoll_ctl_add(epfd, socket_descriptor, EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLHUP | EPOLLERR);
            } else if (events[i].events & EPOLLIN) {
                auto bytes_read = read_msg(events[i].data.fd, buf.data(), BUF_SIZE);
                if (bytes_read > 0) {
                    std::cout << "input msg: " << buf.data() << std::endl;
                    write_msg(events[i].data.fd, buf.data(), bytes_read);
                }
            } else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                epoll_ctl(epfd, EPOLL_CTL_DEL,events[i].data.fd, NULL);
                close(events[i].data.fd);
            }
        }
    }
}

void epoll_ctl_add(int epfd, int fd, uint32_t events) {
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        std::cerr << "We cannot add event to the epoll interest list";
        exit(1);
    }
}