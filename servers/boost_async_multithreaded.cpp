
#include "servers.hpp"

//
// async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

void BoostAsyncMultiThreaded::session::start() {
    socket_.async_read_some(
            boost::asio::buffer(data_, max_length),
            boost::bind(&session::handle_read,
                        this,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
}

void BoostAsyncMultiThreaded::session::handle_read(
        const boost::system::error_code& error,
        size_t bytes_transferred)
{
    if (!error) {
        boost::asio::async_write(socket_,
                                 boost::asio::buffer(data_, bytes_transferred),
                                 boost::bind(&session::handle_write, this,
                                             boost::asio::placeholders::error));
    } else {
        delete this;
    }
}

void BoostAsyncMultiThreaded::session::handle_write(const boost::system::error_code& error)
{
    if (!error) {
        socket_.async_read_some(
                boost::asio::buffer(data_, max_length),
                boost::bind(&session::handle_read,
                            this,
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));
    } else {
        socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_send);
        socket_.close();
        delete this;
    }
}

void BoostAsyncMultiThreaded::handle_accept(BoostAsyncMultiThreaded::session* new_session,
                               const boost::system::error_code& error)
{
    if (!error) {
        new_session->start();
        new_session = new session(io_service);
        acc.async_accept(new_session->socket(),
                         boost::bind(&BoostAsyncMultiThreaded::handle_accept, this, new_session,
                                     boost::asio::placeholders::error));
    } else {
        delete new_session;
    }
}


void BoostAsyncMultiThreaded::init() {
    assert(port != 0);
    assert(thread_pool_size != 0);

    for (auto i = 0; i < thread_pool_size; i++) {
        std::unique_ptr<std::thread> th(
                new std::thread([this](){
                    io_service.run();
                }));
        thread_pool_.push_back(std::move(th));
    }

    // N.B that io_service.run() will be called separately in main
}

void BoostAsyncMultiThreaded::run() {
    session* new_session = new BoostAsyncMultiThreaded::session(io_service);
    acc.async_accept(new_session->socket(),
                     boost::bind(&BoostAsyncMultiThreaded::handle_accept, this, new_session,
                                 boost::asio::placeholders::error));
}
