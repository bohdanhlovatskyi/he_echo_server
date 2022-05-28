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

    Server* server;

#ifdef TARGET_SYNCRONOUS
     server = new Syncronous{port, BUF_SIZE};
#elif TARGET_THREADED
     server = new BlockingMultiThreaded{port, BUF_SIZE};
#elif TARGET_MULTIPROCESS
     server = new BlockingMultiProcess{port, BUF_SIZE};
#elif TARGET_SELECT
     server = new AsyncSelect{port, BUF_SIZE};
#elif TARGET_BOOST_SYNCRONOUS
     server = new BoostSyncronous{port, BUF_SIZE, io};
#elif TARGET_BOOST_THREADED
     server = new BoostBlockingMultiThreaded{port, BUF_SIZE, io};
#elif TARGET_BOOST_THREAD_POOL
     server = new BoostBlockingThreadPool{port, BUF_SIZE, io};
#elif TARGET_BOOST_ASYNC
     server = new BoostAsync{port, BUF_SIZE, io};
#elif TARGET_CORO_BOOST
     server = new CoroBoost{port, BUF_SIZE, io};
#elif TARGET_STACKFUL
     server = new StackFullBoost{port, BUF_SIZE, io};
#elif TARGET_STACKLESS
    server = new StacklessBoost{port, BUF_SIZE, io};
#elif TARGET_EPOLL
#ifdef __linux__
    server = new AsyncEpoll{port, BUF_SIZE};
#endif
#elif TARGET_THREADED_EPOLL
#ifdef __linux__
    server = new ThreadedAsyncEpoll{port, BUF_SIZE};
#endif
#elif TARGET_IO_SUBMIT
#ifdef __linux__
    server = new AsyncIOSubmit{port, BUF_SIZE};
#endif
#else
    std::cerr << "No valid compile target defined!" << std::endl;
    return -1;
#endif

    try {
        server->init();
        server->run();
        io.run();
    } catch (std::runtime_error &e) {
        std::cerr << "Fatal: " << e.what() << std::endl;
        return 4;
    }

    return 0;
}