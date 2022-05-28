#include "servers.hpp"


void StackFullBoost::session::go() {
    boost::asio::spawn(strand_,
                       boost::bind(&session::echo,
                                   shared_from_this(), _1));
}

void StackFullBoost::session::echo(boost::asio::yield_context yield) {
    try {
        char data[128];
        for (;;)
        {
            std::size_t n = socket_.async_read_some(boost::asio::buffer(data), yield);
            boost::asio::async_write(socket_, boost::asio::buffer(data, n), yield);
        }
    } catch (std::exception& e)
    {
        socket_.close();
    }
}

void StackFullBoost::accept(boost::asio::io_service& io_service,
                            size_t port, boost::asio::yield_context yield) {
    (void) port;

    boost::system::error_code ec;
    boost::shared_ptr<session> new_session(new session(io_service));
    acc.async_accept(new_session->socket(), yield[ec]);
    if (!ec) new_session->go();
}

void StackFullBoost::init() {
    assert(port != 0);
}

void StackFullBoost::run() {
    boost::asio::spawn(io_service,
        [&](boost::asio::yield_context yield){ accept(io_service, port, yield);} );
}