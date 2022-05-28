#include "servers.hpp"

struct read_awaiter {

    inline bool await_ready() { return false; }

    void await_suspend(coro_handle h);
    auto await_resume();

    boost::asio::ip::tcp::socket& socket_;
    boost::asio::mutable_buffer buffer_;
    std::error_code ec_;
    size_t bytes_read_;
};

void CoroBoost::read_awaiter::await_suspend(coro_handle h) {
    socket_.async_read_some(buffer_,
                            [this, h](auto ec, auto bytes_read) mutable {
                                ec_ = ec;
                                bytes_read_ = bytes_read;
                                h.resume();
                            });
}

auto CoroBoost::read_awaiter::await_resume() {
    if (ec_) {
        // throw std::system_error(ec_);
        std::cerr << ec_ << std::endl;
    }
    return bytes_read_;
}

void CoroBoost::write_awaiter::await_suspend(coro_handle h) {
    boost::asio::async_write(socket_, buffer_,
                             [this, h](auto ec, size_t bytes) mutable {
                                 (void) bytes;
                                 ec_ = ec;
                                 h.resume();
                             });
}

auto CoroBoost::write_awaiter::await_resume() {
    if (ec_) {
//            throw std::system_error(ec_);
        std::cerr << ec_ << std::endl;
    }
}

CoroBoost::Task CoroBoost::handle_client(boost::asio::ip::tcp::socket socket) noexcept {
    static const size_t kBufferSize = 1024;

    char data[kBufferSize];

    while (true) {
        size_t bytes_read = co_await async_read(socket, boost::asio::buffer(data));
        co_await async_write(socket, {data, bytes_read});
    }
}

void CoroBoost::accept() {
    acc.async_accept(
            [this](std::error_code error_code, boost::asio::ip::tcp::socket client_socket) {
                if (!error_code) {
                    handle_client(std::move(client_socket));
                }
                accept();
            });
}

void CoroBoost::init() {
    ;
}

void CoroBoost::run() {
    accept();
}