//
// blocking_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "servers.hpp"

void BoostBlockingMultiThreaded::init() {
    assert(port != 0);
    ;
}

void BoostBlockingMultiThreaded::session_(
        boost::shared_ptr<boost::asio::ip::tcp::socket> sock, size_t buf_size) {
    std::vector<char> data;
    data.resize(buf_size);

    boost::system::error_code error;

    try {
        for (;;) {
            size_t length = sock->read_some(boost::asio::buffer(data), error);

            if (error && error != boost::asio::error::eof) {
                throw boost::system::system_error(error);
            } else if (error || length == 0) {
                break;
            }

            boost::asio::write(*sock, boost::asio::buffer(data, length));
        }

        sock->shutdown(boost::asio::ip::tcp::socket::shutdown_send);
        sock->close();

    } catch (std::exception& e) {
        // TODO: consider using future to get the exception from here
        std::cerr << "Exception in thread: " << e.what() << "\n";
    }
}



void BoostBlockingMultiThreaded::run() {
    using socket_ptr = boost::shared_ptr<boost::asio::ip::tcp::socket>;

    for (;;) {
        socket_ptr sock(new boost::asio::ip::tcp::socket(io_service));
        acc.accept(*sock);
        boost::thread t(boost::bind(session_, sock, buf_size));
    }
}

