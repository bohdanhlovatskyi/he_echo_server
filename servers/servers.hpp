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

#include <cstdlib>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/spawn.hpp>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/write.hpp>
#include <cstdio>

#include <utility>
#include <experimental/coroutine>

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

class BoostBlockingThreadPool: public Server {
    using atcp = boost::asio::ip::tcp;
    using tp_t = boost::asio::thread_pool;
private:
    atcp::acceptor acc;
    boost::asio::io_service& io_service;
    tp_t tp;

    static void session_(boost::shared_ptr<boost::asio::ip::tcp::socket> soc, size_t buf_size);
public:
    BoostBlockingThreadPool(size_t port, ssize_t buf_size, boost::asio::io_service& io): \
                            Server::Server(port, buf_size),
                            io_service{io},
                            acc{io, atcp::endpoint( atcp::v4(), port)},
                            tp{std::thread::hardware_concurrency()} {};

    void init() override;
    void run() override;

    ~BoostBlockingThreadPool() = default;
};


class BoostAsync: public Server {
    using atcp = boost::asio::ip::tcp;
private:
    atcp::acceptor acc;
    boost::asio::io_service& io_service;

    class session {
    private:
        atcp::socket socket_;
        enum { max_length = 1024 };
        char data_[max_length];

    public:
        session(boost::asio::io_service& io_service)
                : socket_(io_service) {}

        inline atcp::socket& socket() {
            return socket_;
        }

        void start();
        void handle_write(const boost::system::error_code& error);
        void handle_read(const boost::system::error_code& error, size_t bytes_transferred);
    };

    void handle_accept(session* new_session, const boost::system::error_code& error);

public:
    BoostAsync(size_t port, ssize_t buf_size, boost::asio::io_service& io): \
                        Server::Server(port, buf_size),
                        io_service{io},
                        acc{io, atcp::endpoint( atcp::v4(), port)} {};

    void init() override;
    void run() override;

    ~BoostAsync() = default;
};

using coro_handle = std::experimental::coroutine_handle<>;
class CoroBoost : public Server {
private:
    boost::asio::ip::tcp::acceptor acc;
    boost::asio::io_service& io_service;

    struct Task {
        struct promise_type {
            inline auto get_return_object() { return Task{}; }
            inline auto initial_suspend() noexcept { return std::experimental::suspend_never{}; }
            inline auto final_suspend() noexcept { return std::experimental::suspend_never{}; }
            inline void unhandled_exception() { std::terminate(); }
            inline void return_void() {}
        };
    };

    struct read_awaiter {

        inline bool await_ready() { return false; }

        void await_suspend(coro_handle h);
        auto await_resume();

        boost::asio::ip::tcp::socket& socket_;
        boost::asio::mutable_buffer buffer_;
        std::error_code ec_{};
        size_t bytes_read_{};
    };

    struct write_awaiter {
        inline bool await_ready() { return false; }

        void await_suspend(coro_handle h);
        auto await_resume();

        boost::asio::ip::tcp::socket& socket_;
        boost::asio::const_buffer buffer_;
        std::error_code ec_{};
    };

    void accept(boost::asio::io_service& io_service,
                size_t port, boost::asio::yield_context yield);

    Task handle_client(boost::asio::ip::tcp::socket socket) noexcept;

    inline auto async_write(boost::asio::ip::tcp::socket& socket,
                     boost::asio::const_buffer buffer) {
        return write_awaiter{socket, buffer};
    }

    inline auto async_read(boost::asio::ip::tcp::socket& socket, boost::asio::mutable_buffer buffer) {
        return read_awaiter{socket, buffer};
    }

public:
    CoroBoost(size_t port, ssize_t buf_size, boost::asio::io_service& io): \
                        Server::Server(port, buf_size),
                        io_service{io},
                        acc{io,
                            boost::asio::ip::tcp::endpoint( boost::asio::ip::tcp::v4(), port)}
    {
        ;
    };

    void init() override;
    void run() override;

    ~CoroBoost() = default;
};


class StackFullBoost : public Server {
    using atcp = boost::asio::ip::tcp;
private:
    atcp::acceptor acc;
    boost::asio::io_service& io_service;

    void accept(boost::asio::io_service& io_service,
                size_t port, boost::asio::yield_context yield);

    class session : public boost::enable_shared_from_this<session> {
    private:
        boost::asio::io_service::strand strand_;
        boost::asio::ip::tcp::socket socket_;

        void echo(boost::asio::yield_context yield);

    public:
        inline explicit session(boost::asio::io_service& io_service)
                : strand_(io_service),
                  socket_(io_service)
        {
            ;
        }

        inline boost::asio::ip::tcp::socket& socket() { return socket_; }

        void go();
    };
public:
    StackFullBoost(size_t port, ssize_t buf_size, boost::asio::io_service& io): \
                        Server::Server(port, buf_size),
                        io_service{io},
                        acc{io, atcp::endpoint( atcp::v4(), port)} {};

    void init() override;
    void run() override;

    ~StackFullBoost() = default;
};



class StacklessBoost : public Server {
    using atcp = boost::asio::ip::tcp;
private:
    boost::asio::io_context io_context{1};

    boost::asio::awaitable<void> echo(atcp::socket socket);

public:
    StacklessBoost(size_t port, ssize_t buf_size, boost::asio::io_service& io): \
                        Server::Server(port, buf_size) { (void) io; };

    boost::asio::awaitable<void> listener();

    void init() override;
    void run() override;

    ~StacklessBoost() = default;
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

class ThreadedAsyncEpoll: public Server {
private:
    oneapi::tbb::task_arena arena{
        oneapi::tbb::this_task_arena::max_concurrency()
    };

    constexpr static short MAX_EVENTS = 32;
    struct epoll_event events[MAX_EVENTS];
    int epfd;
    void epoll_ctl_add(int epfd, int fd, uint32_t events);

public:
    using Server::Server;

    void init() override;
    void run() override;

    ~ThreadedAsyncEpoll() = default;
};

class AsyncIOSubmit: public Server {
private:
    static constexpr short MAX_EVENTS = 32;

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

    aio_context_t ctx;
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