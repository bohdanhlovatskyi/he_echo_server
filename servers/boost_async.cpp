
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

void BoostAsync::session::start() {
    socket_.async_read_some(
        boost::asio::buffer(data_, max_length),
        boost::bind(&session::handle_read,
        this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
}

void BoostAsync::session::handle_read(
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

void BoostAsync::session::handle_write(const boost::system::error_code& error)
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

void BoostAsync::handle_accept(BoostAsync::session* new_session,
                                    const boost::system::error_code& error)
{
    if (!error) {
        new_session->start();
        new_session = new session(io_service);
        acc.async_accept(new_session->socket(),
                               boost::bind(&BoostAsync::handle_accept, this, new_session,
                                           boost::asio::placeholders::error));
    } else {
        delete new_session;
    }
}


void BoostAsync::init() {
    assert(port != 0);
    ;
}

void BoostAsync::run() {
    session* new_session = new BoostAsync::session(io_service);
    acc.async_accept(new_session->socket(),
                     boost::bind(&BoostAsync::handle_accept, this, new_session,
                                 boost::asio::placeholders::error));
}
