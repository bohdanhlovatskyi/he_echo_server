#ifndef SERVERTS_
#define SERVERTS_

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include <iostream>
#include <vector>
#include <thread>
#include <cassert>

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


#endif // SERVERTS_