//
// blocking_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

using boost::asio::ip::tcp;

constexpr int max_length = 1024;

// TODO: socket is not thread safe , do this unique ptr?
typedef boost::shared_ptr<tcp::socket> socket_ptr;

void session(socket_ptr sock) {
    try {
        char data[max_length];

        boost::system::error_code error;
        size_t length = sock->read_some(boost::asio::buffer(data), error);

#ifdef DEBUG_INFO
        std::cout << "New session was created: [" << sock->remote_endpoint().address() << \
        ", " << sock->remote_endpoint().port() << "]" << std::endl;
#endif
        if (error && error != boost::asio::error::eof) {
            throw boost::system::system_error(error);
        }

        boost::asio::write(*sock, boost::asio::buffer(data, length));
        sock->shutdown(boost::asio::ip::tcp::socket::shutdown_send);
        sock->close();

    } catch (std::exception& e) {
        std::cerr << "Exception in thread: " << e.what() << "\n";
    }
}

void server(boost::asio::io_service& io_service, short port) {
    tcp::acceptor a(io_service, tcp::endpoint(tcp::v4(), port));

    for (;;) {
        socket_ptr sock(new tcp::socket(io_service));
        a.accept(*sock);
        boost::thread t(boost::bind(session, sock));
    }
}

int main(int argc, char* argv[]) {

    if (argc != 2)
    {
        std::cerr << "Usage: blocking_tcp_echo_server <port>\n";
        return 1;
    }

    boost::asio::io_service io_service;
    // TODO: port will be short, check overflow ?
    auto port = std::atoi(argv[1]);
    if (port == 0) {
        std::cerr << "Error: wrong port specified" << std::endl;
        return 2;
    }

    try {
        server(io_service, port);
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}