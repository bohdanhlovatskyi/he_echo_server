#include <winsock2.h>
#include <windows.h>

#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>

#include <iostream>
#include <array>

#pragma comment(lib, "Ws2_32.lib")

constexpr short SERVER_PORT = 8888;
constexpr short BUF_SIZE = 1024;

fd_set read_set, write_set, ex_set;

struct fd_state {
    char buf[BUF_SIZE];
    bool wr;

    fd_state(): buf{}, wr{false} {};
    ~fd_state() = default;
};

int read(SOCKET client, fd_state* state) {
    recv(client, state->buf, sizeof(state->buf), 0);
    state->wr = true;
    std::cout<<"write try \n";

    return 0;
}

int write(SOCKET client, fd_state* state) {
    send(client, state->buf, sizeof(state->buf), 0);
    state->wr = false;

    return 0;
}

int main(int argc, char* argv[]){
    WSADATA WSAData;

    int iResult;
    iResult = WSAStartup(MAKEWORD(2, 2), &WSAData);
    if (iResult != 0) {
        std::cerr<<"WSAStartup failed: "<< iResult<<"\n";
        return 1;
    }

    SOCKET listener = socket(AF_INET, SOCK_STREAM, 0);
    if(listener == INVALID_SOCKET){
        std::cerr <<"socket function failed with error: "<< WSAGetLastError()<<"\n";
        WSACleanup();
        return 1;
    }


    SOCKADDR_IN service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = inet_addr("127.0.0.1");
    service.sin_port = htons(SERVER_PORT);
    char on = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    auto err = bind(listener, (SOCKADDR*)&service, sizeof(service));
    if(err == SOCKET_ERROR){
        std::cerr<<"bind failed with error "<< WSAGetLastError();
        closesocket(listener);
        WSACleanup();
        return 1;
    }

    listen(listener, 0);
    fd_state* state[FD_SETSIZE];
    for (auto & i : state)
        i = nullptr;
    FD_ZERO(&read_set);
    FD_ZERO(&write_set);
    FD_ZERO(&ex_set);

    for (;;) {
        auto max_fd = listener;
        FD_ZERO(&read_set);
        FD_ZERO(&write_set);
        FD_ZERO(&ex_set);

        // add listener set to the set of sockets that are ready to read
        FD_SET(listener, &read_set);

        for (size_t i = 0; i < FD_SETSIZE; ++i) {
            if (state[i] == nullptr) {
                continue;
            }

            max_fd = i > max_fd ? i : max_fd;
            FD_SET(i, &read_set);
            if (state[i]->wr) {
                FD_SET(i, &write_set);
            }
        }

        // null ptr - blocks
        err = select(max_fd + 1, &read_set, &write_set, &ex_set, NULL);//syscall not like winda( Fara stattya. Stromno KQueL
        if (err < 0) {
            std::cerr << "select err: " << WSAGetLastError()<< std::endl;
            exit(1);
        }

        // new connection accepted
        if (FD_ISSET(listener, &read_set)) {

            SOCKET client;
            SOCKADDR_IN client_adr;
            int client_adr_size = sizeof(client_adr);
            auto socket_descriptor = accept(listener, (SOCKADDR*)&client_adr, &client_adr_size);
            if (socket_descriptor == INVALID_SOCKET) {
                std::cerr << "Error accepting request" << std::endl;
                closesocket(listener);
                WSACleanup();
                exit(1);
            } else {
                // note that the socket descriptor is already in the reading set
                //fcntl(socket_descriptor, F_SETFL, O_NONBLOCK);
                state[socket_descriptor] = new fd_state();
            }
        }

        for (size_t i = 0; i < max_fd + 1; ++i) {
            if (i == listener) continue;

            if (FD_ISSET(i, &read_set))
                read(i, state[i]);

            if (FD_ISSET(i, &write_set)) {
                std::cout<<"write try \n";
                write(i, state[i]);
                state[i] = nullptr;
                // close the connection
                closesocket(listener);
                WSACleanup();
            }
        }

    }
}