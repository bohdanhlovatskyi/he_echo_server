#ifndef SERVERTS_
#define SERVERTS_

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include <iostream>
#include <vector>
#include <thread>
#include <cassert>
#include <cstring>

#include "common_sockets.hpp"

class Server {
protected:
    const ssize_t buf_size;
    const size_t port;
    int listener_fd;
public:
    inline explicit Server(size_t port, ssize_t buf_size):\
                        port{port}, buf_size{buf_size}, listener_fd{-1} {};

    virtual void run() = 0;
    virtual void init() = 0;

    inline virtual ~Server() noexcept {
        clean_up(listener_fd);
    }
};

class Syncronous: public Server {
public:
    using Server::Server;

    void init() override;
    void run() override;

    ~Syncronous() = default;
};

class BlockingMultiThreaded: public Server {
public:
    using Server::Server;

    void init() override;
    void run() override;

    ~BlockingMultiThreaded() = default;
};

class BlockingMultiProcess: public Server {
public:
    using Server::Server;

    void init() override;
    void run() override;

    ~BlockingMultiProcess() = default;
};

class AsyncSelect: public Server {
private:
    fd_set read_set, write_set, ex_set;

    struct fd_state {
        std::vector<char> buf;
        bool wr;
        ssize_t bytes_read;
        fd_state(): buf{}, wr{false}, bytes_read{0} {};
        ~fd_state() = default;
    };

    fd_state* state[FD_SETSIZE];

    ssize_t read_(int fd, fd_state* state);
    ssize_t write_(int fd, fd_state* state);

public:
    using Server::Server;

    void init() override;
    void run() override;

    ~AsyncSelect() = default;
};


#ifdef __linux__
#include <sys/epoll.h>

class AsyncEpoll: public Server {
private:
    constexpr static short MAX_EVENTS = 32;
    struct epoll_event events[MAX_EVENTS];
    int epfd;
    void epoll_ctl_add(int epfd, int fd, uint32_t events);

public:
    using Server::Server;

    void init() override;
    void run() override;

    ~AsyncEpoll() = default;
};

#endif // __linux__

#endif // SERVERTS_