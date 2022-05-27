
#include <experimental/coroutine>

#include <boost/asio.hpp>

#include <utility>
#include <vector>
#include <thread>
#include <iostream>

#include <experimental/coroutine>

struct Task {
    struct promise_type {
        auto get_return_object() { return Task{}; }
        auto initial_suspend() noexcept { return std::experimental::suspend_never{}; }
        auto final_suspend() noexcept { return std::experimental::suspend_never{}; }
        void unhandled_exception() { std::terminate(); }
        void return_void() {}
    };
};

using boost::asio::ip::tcp;

using std::experimental::coroutine_handle;

//////////////////////////////////////////////////////////////////////

// Asynchronous operations

struct ReadSomeAwaiter {
    bool await_ready() {
        return false;
    }

    void await_suspend(coroutine_handle<> h) {
        socket_.async_read_some(buffer_,
                                [this, h](auto ec, auto bytes_read) mutable {
                                    ec_ = ec;
                                    bytes_read_ = bytes_read;
                                    h.resume();
                                });
    }

    auto await_resume() {
        if (ec_) {
            // throw std::system_error(ec_);
            std::cerr << ec_ << std::endl;
        }
        return bytes_read_;
    }

    // Arguments
    tcp::socket& socket_;
    boost::asio::mutable_buffer buffer_;

    std::error_code ec_;
    size_t bytes_read_;
};

auto AsyncReadSome(tcp::socket& socket, boost::asio::mutable_buffer buffer) {
    return ReadSomeAwaiter{socket, buffer};
}

struct WriteAwaiter {
    bool await_ready() {
        return false;
    }

    void await_suspend(coroutine_handle<> h) {
        boost::asio::async_write(socket_, buffer_,
                          [this, h](auto ec, size_t bytes) mutable {
                              ec_ = ec;
                              h.resume();
                          });
    }

    void await_resume() {
        if (ec_) {
//            throw std::system_error(ec_);
            std::cerr << ec_ << std::endl;
        }
    }

    // Arguments
    tcp::socket& socket_;
    boost::asio::const_buffer buffer_;

    std::error_code ec_;
};

auto AsyncWrite(tcp::socket& socket, boost::asio::const_buffer buffer) {
    return WriteAwaiter{socket, buffer};
}

//////////////////////////////////////////////////////////////////////

// Client Handler

Task HandleClient(tcp::socket socket) noexcept {
    static const size_t kBufferSize = 1024;

    char data[kBufferSize];

    while (true) {
        size_t bytes_read = co_await AsyncReadSome(socket, boost::asio::buffer(data));
        co_await AsyncWrite(socket, {data, bytes_read});
    }
}

//////////////////////////////////////////////////////////////////////

class Server {
public:
    Server(boost::asio::io_context& io_context, uint16_t port)
            : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
    }

    void Start() {
        AcceptClient();
    }

private:
    void AcceptClient() {
        acceptor_.async_accept(
                [this](std::error_code error_code, tcp::socket client_socket) {
                    if (!error_code) {
                        HandleClient(std::move(client_socket));
                    }
                    AcceptClient();
                });
    }

private:
    tcp::acceptor acceptor_;
};

////////////////////////////////////////////////////////////////////////////////

void Run(boost::asio::io_context& io_context, size_t threads) {
    std::vector<std::thread> workers;

    for (size_t i = 1; i < threads; ++i) {
        workers.emplace_back([&io_context]() {
            io_context.run();
        });
    }
    io_context.run();

    for (auto& t : workers) {
        t.join();
    }
}

void ServeForever(uint16_t port) {
    boost::asio::io_context io_context;

    Server server(io_context, port);
    server.Start();

    Run(io_context, /*threads=*/5);
}

int main(void ) {
    ServeForever(9000);
    return 0;
}
