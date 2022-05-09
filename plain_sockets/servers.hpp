#ifndef SERVERTS_
#define SERVERTS_

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include <iostream>
#include <array>

class Server {
protected:
    constexpr static short BUF_SIZE = 1024;
    const size_t port;
public:
    inline explicit Server(size_t port): port{port} {};

    virtual void run() = 0;
    virtual void init() = 0;
};

class Syncronous: public Server {
private:
    int listener_fd;
    std::array<char, Server::BUF_SIZE> buf{{}};
public:
    using Server::Server;

    void init() override;
    void run() override;
};


#endif // SERVERTS_