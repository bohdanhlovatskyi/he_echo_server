//
// Created by yarmus on 4/26/22.
//
#include "common_sockets.hpp"
#include "servers.hpp"

// register events of fd to epfd
void AsyncEpoll::epoll_ctl_add(int epfd, int fd, uint32_t events) {
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        std::cerr << "We cannot add event to the epoll interest list";
        exit(1);
    }
}

void AsyncEpoll::init() {
    listener_fd = create_listener(false, port);

    epfd = epoll_create(1);
    if (epfd == -1) {
        std::cerr << "We cannot create epoll instance" << std::endl;
        exit(1);
    }

    epoll_ctl_add(epfd, listener_fd, EPOLLIN | EPOLLOUT | EPOLLET);
}

void AsyncEpoll::run() {
    std::vector<char> buf{};
    buf.reserve(buf_size);

    for (;;) {
        // If timeout equals –1, block until an event occurs for one of the file descriptors in
        // the interest list for epfd or until a signal is caught.
        auto nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);

        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == listener_fd) {
                // handle new connection
                auto socket_descriptor = handle_connection(listener_fd, false);

                // adding new socket to epoll interest list
                epoll_ctl_add(epfd, socket_descriptor, EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLHUP | EPOLLERR);
            } else if (events[i].events & EPOLLIN) {
                auto bytes_read = read_msg(events[i].data.fd, buf.data(), buf_size);
                if (bytes_read > 0) {
                    std::cout << "input msg: " << buf.data() << std::endl;
                    write_msg(events[i].data.fd, buf.data(), bytes_read);
                } else {
                    epoll_ctl(epfd, EPOLL_CTL_DEL,events[i].data.fd, NULL);
                    close(events[i].data.fd);
                }
            } else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                epoll_ctl(epfd, EPOLL_CTL_DEL,events[i].data.fd, NULL);
                close(events[i].data.fd);
            }
        }
    }
}
