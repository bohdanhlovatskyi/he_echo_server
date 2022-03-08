#include <iostream>

int main(int argc, char* argv[]) {

    if (argc != 3) {
        std::cerr << "Usage: start_server <port> <method>\n";
        return 1;
    }

    auto port = std::atoi(argv[1]);
    if (port == 0) {
        std::cerr << "Error: wrong port specified" << std::endl;
        return 2;
    }

    auto method = std::atoi(argv[2]);
    if (method == 0) {
        std::cerr << "Error: wrong method classifier" << std::endl;
        return 3;
    }

    boost::asio::io_service io_service;

    try {
        boost::asio::io_service io_service;
        server s(io_service, port);
        io_service.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}