#include <iostream>
#include <vector>

#include "servers.hpp"

constexpr ssize_t BUF_SIZE = 1024;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: start_server <port> <method>\n";
        return 1;
    }

    auto port = static_cast<size_t>(std::atoi(argv[1]));
    if (port == 0) {
        std::cerr << "Error: wrong port specified" << std::endl;
        return 2;
    }

    boost::asio::io_service io;
#ifdef TARGET_SYNCRONOUS
    auto server = Syncronous{port, BUF_SIZE};
#elif TARGET_THREADED
    auto server = BlockingMultiThreaded{port, BUF_SIZE};
#elif TARGET_MULTIPROCESS
    auto server = BlockingMultiProcess{port, BUF_SIZE};
#elif TARGET_SELECT
    auto server = AsyncSelect{port, BUF_SIZE};
#elif TARGET_BOOST_SYNCRONOUS
    auto server = BoostSyncronous{port, BUF_SIZE, io};
#elif TARGET_BOOST_THREADED
    auto server = BoostBlockingMultiThreaded{port, BUF_SIZE, io};
#elif TARGET_BOOST_THREAD_POOL
    auto server = BoostBlockingThreadPool{port, BUF_SIZE, io};
#elif TARGET_BOOST_ASYNC
    auto server = BoostAsync{port, BUF_SIZE, io};
#elif TARGET_CORO_BOOST
    auto server = CoroBoost{port, BUF_SIZE, io};
#elif TARGET_STACKFUL
    auto server = StackFullBoost{port, BUF_SIZE, io};
#elif TARGET_STACKLESS
    auto server = StacklessBoost{port, BUF_SIZE, io};
#elif TARGET_EPOLL
#ifdef __linux__
    auto server = AsyncEpoll{port, BUF_SIZE});
#endif
#elif TARGET_IO_SUBMIT
#ifdef __linux__
    auto server = AsyncIOSubmit{port, BUF_SIZE});
#endif
#else
    std::cerr << "No valid compile target defined!" << std::endl;
    return -1;
#endif

    try {
        server.init();
        server.run();
        io.run();
    } catch (std::runtime_error &e) {
        std::cerr << "Fatal: " << e.what() << std::endl;
        return 4;
    }

    return 0;
}