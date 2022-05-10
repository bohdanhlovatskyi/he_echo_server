#ifndef SERVERTS_
#define SERVERTS_

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include <boost/asio.hpp>

#include <iostream>
#include <vector>
#include <thread>
#include <cassert>
#include <cstring>

// TODO: get rid of them here and in the boost threaded
#include <cstdlib>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

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

class BoostSyncronous: public Server {
    using atcp = boost::asio::ip::tcp;
private:
    atcp::acceptor acc;
    boost::asio::io_service& io_service;
public:
    inline BoostSyncronous(size_t port, ssize_t buf_size, boost::asio::io_service& io): \
                    Server::Server(port, buf_size),
                    io_service{io},
                    acc{io, atcp::endpoint( atcp::v4(), port)} {};

    void init() override;
    void run() override;

    ~BoostSyncronous() = default;
};


class BlockingMultiThreaded: public Server {
public:
    using Server::Server;

    void init() override;
    void run() override;

    ~BlockingMultiThreaded() = default;
};

class BoostBlockingMultiThreaded: public Server {
    using atcp = boost::asio::ip::tcp;
private:
    atcp::acceptor acc;
    boost::asio::io_service& io_service;

    static void session_(boost::shared_ptr<boost::asio::ip::tcp::socket> soc, size_t buf_size);
public:
    BoostBlockingMultiThreaded(size_t port, ssize_t buf_size, boost::asio::io_service& io): \
                        Server::Server(port, buf_size),
                        io_service{io},
                        acc{io, atcp::endpoint( atcp::v4(), port)} {};

    void init() override;
    void run() override;

    ~BoostBlockingMultiThreaded() = default;
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

#include <unistd.h>
#include <poll.h>
#include <linux/aio_abi.h>
#include <sys/syscall.h>

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


class AsyncIOSubmit: public Server {
private:
    constexpr short MAX_EVENTS = 32;

    inline static long io_setup(unsigned nr, aio_context_t *ctxp) {
        return syscall(__NR_io_setup, nr, ctxp);
    }

    inline static long io_destroy(aio_context_t ctx) {
        return syscall(__NR_io_destroy, ctx);
    }

    inline static long io_submit(aio_context_t ctx, long nr,  struct iocb **iocbpp) {
        return syscall(__NR_io_submit, ctx, nr, iocbpp);
    }

    inline static long io_getevents(aio_context_t ctx, long min_nr, long max_nr,
            struct io_event *events, struct timespec *timeout) {
        return syscall(__NR_io_getevents, ctx, min_nr, max_nr, events, timeout);
    }

    void create_iocb_(iocb& cb, int sd);

    aio_context_t ctx = 0;
    struct iocb cb_listener, cb_client;
    struct iocb *cbs_for_listener[1];
    struct iocb *cbs_for_client[1];
    struct iocb *cbs_for_listener_and_client[2];

public:
    using Server::Server;

    void init() override;
    void run() override;

    // additional clean up for this one is needed
    inline ~AsyncIOSubmit() {
        io_destroy(ctx);
    };
};

#endif // __linux__

#endif // SERVERTS_