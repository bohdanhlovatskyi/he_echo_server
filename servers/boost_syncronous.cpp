
#include "servers.hpp"

void BoostSyncronous::init() {
    assert(port != 0);
    ;
}

void BoostSyncronous::run() {
    std::vector<char> data;
    data.resize(buf_size);

    for (;;) {
        boost::asio::ip::tcp::socket socket(io_service);
        acc.accept( socket );

        boost::system::error_code error;

        for (;;) {
            auto length = socket.read_some(boost::asio::buffer(data), error);
            // TODO: error handling is awkward here
            if (error || length == 0) {
                std::cerr << error.message() << std::endl;
                break;
            }

            boost::asio::write(socket, boost::asio::buffer(data),
                               boost::asio::transfer_all(), error);
        }

        socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send);
        socket.close();
    }
}