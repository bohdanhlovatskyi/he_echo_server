#include <iostream>

#include "servers.hpp"

int main(int argc, char* argv[]) {

    if (argc != 3) {
        std::cerr << "Usage: start_server <port> <method>\n";
        return 1;
    }

    auto port = static_cast<size_t>(std::atoi(argv[1]));
    if (port == 0) {
        std::cerr << "Error: wrong port specified" << std::endl;
        return 2;
    }

    auto method = std::atoi(argv[2]);
    if (method == 1) {
        Syncronous s{port};
        s.run();
    } else {
        std::cerr << "Wrong method specialized" << std::endl;
        exit(3);
    }

    return 0;
}