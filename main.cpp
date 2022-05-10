#include <iostream>
#include <vector>

#include "servers.hpp"

constexpr ssize_t BUF_SIZE = 1024;

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

    std::vector<Server *> servers = {new Syncronous{port, BUF_SIZE},
                                     new BlockingMultiThreaded{port, BUF_SIZE},
                                     new BlockingMultiProcess{port, BUF_SIZE},
                                     new AsyncSelect{port, BUF_SIZE},
                                     };

    // TODO: atoi could return 0
    auto method = std::atoi(argv[2]);
    if (method < 0 || static_cast<size_t>(method) >= servers.size()) {
        std::cerr << "Fatal: Wrong method specified" << std::endl;
        return 3;
    }

    try {
        servers[method]->init();
    } catch (std::runtime_error e) {
        std::cerr << "Fatal: could not init server" << e.what() << std::endl;
        return 4;
    }

    try {
        servers[method]->run();
    } catch (std::runtime_error e) {
        std::cerr << "Fatal: " << e.what() << std::endl;
        return 4;
    }

    return 0;
}