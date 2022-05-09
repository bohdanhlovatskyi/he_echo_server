#ifndef SERVERTS_
#define SERVERTS_

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include <iostream>
#include <array>

class Syncronous {
private:
    const size_t port;
    constexpr static short BUF_SIZE = 1024;

    int listener_fd;
    std::array<char, BUF_SIZE> buf{{}};

public:
    explicit Syncronous(size_t port);

    void run();
};


#endif // SERVERTS_