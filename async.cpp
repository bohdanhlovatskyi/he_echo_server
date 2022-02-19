//
// async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class session
{
public:
    session(boost::asio::io_service& io_service)
            : socket_(io_service) {}

    tcp::socket& socket() {
        return socket_;
    }

    void start() {

#ifdef DEBUG_INFO
        std::cout << "New session was created: [" << this->socket().remote_endpoint().address() << \
            ", " << this->socket().remote_endpoint().port() << "]" << std::endl;
#endif

        socket_.async_read_some(boost::asio::buffer(data_, max_length),
                                boost::bind(&session::handle_read, this,
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred));
    }

    void handle_read(const boost::system::error_code& error,
                     size_t bytes_transferred)
    {
        if (!error) {
            boost::asio::async_write(socket_,
                                     boost::asio::buffer(data_, bytes_transferred),
                                     boost::bind(&session::handle_write, this,
                                                 boost::asio::placeholders::error));
        } else {
#ifdef DEBUG_INFO
            std::cout << "Session was killed..." << std::endl;
#endif
            delete this;
        }
    }

    void handle_write(const boost::system::error_code& error)
    {
        if (!error) {
            socket_.async_read_some(boost::asio::buffer(data_, max_length),
                                    boost::bind(&session::handle_read, this,
                                                boost::asio::placeholders::error,
                                                boost::asio::placeholders::bytes_transferred));
#ifdef DEBUG_INFO
            std::cout << "Session was continued..." << std::endl;
#endif
        } else {
            delete this;
        }
    }

private:
    tcp::socket socket_;
    enum { max_length = 1024 };
    char data_[max_length];
};


class server {
public:
    server(boost::asio::io_service& io_service, short port)
            : io_service_(io_service),
              acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
    {
        session* new_session = new session(io_service_);
        acceptor_.async_accept(new_session->socket(),
                               boost::bind(&server::handle_accept, this, new_session,
                                           boost::asio::placeholders::error));
    }

    void handle_accept(session* new_session,
                       const boost::system::error_code& error)
    {
        if (!error) {
            new_session->start();
            new_session = new session(io_service_);
            acceptor_.async_accept(new_session->socket(),
                                   boost::bind(&server::handle_accept, this, new_session,
                                               boost::asio::placeholders::error));
        } else {
            delete new_session;
        }
    }

private:
    boost::asio::io_service& io_service_;
    tcp::acceptor acceptor_;
};

int main(int argc, char* argv[]) {

    if (argc != 2) {
        std::cerr << "Usage: async_tcp_echo_server <port>\n";
        return 1;
    }

    auto port = std::atoi(argv[1]);
    if (port == 0) {
        std::cerr << "Error: wrong port specified" << std::endl;
        return 2;
    }

    try {
        boost::asio::io_service io_service;
        server s(io_service, port);
        io_service.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}