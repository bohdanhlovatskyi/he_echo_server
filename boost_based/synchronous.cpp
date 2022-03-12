#include <iostream>
#include <boost/asio.hpp>

constexpr int max_length = 1024;

int main(int argc, char* argv[]) {

    if (argc != 2) {
        std::cerr << "Usage: synchronous <port>\n";
        return 1;
    }

    auto port = std::atoi(argv[1]);
    if (port == 0) {
        std::cerr << "Error: wrong port specified" << std::endl;
        return 2;
    }

    boost::asio::io_service io_service;

    boost::asio::ip::tcp::acceptor acc(
            io_service,
            boost::asio::ip::tcp::endpoint( boost::asio::ip::tcp::v4(), port ) );

    char data[max_length];

    for (;;) {
        boost::asio::ip::tcp::socket socket(io_service);
        acc.accept( socket );

        boost::system::error_code error;
        auto length = socket.read_some(boost::asio::buffer(data), error);
        if (error || length == 0) {
            continue;
        }

        boost::asio::write(socket, boost::asio::buffer(data),
                           boost::asio::transfer_all(), error);

        socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send);
        socket.close();
    }
}
