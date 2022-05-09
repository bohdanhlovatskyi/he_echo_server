//
// Created by yarmus on 4/26/22.
//
#include <sys/socket.h>
#include <unistd.h>
#include <poll.h>
#include <linux/aio_abi.h>
#include <sys/syscall.h>
#include <string.h>
#include "common_sockets.hpp"

#include <iostream>
#include <array>

constexpr short SERVER_PORT = 9000;
constexpr short BUF_SIZE = 1024;
constexpr short MAX_EVENTS = 32;

inline long io_setup(unsigned nr, aio_context_t *ctxp)
{
    return syscall(__NR_io_setup, nr, ctxp);
}

inline long io_destroy(aio_context_t ctx)
{
 	return syscall(__NR_io_destroy, ctx);
}

inline long io_submit(aio_context_t ctx, long nr,  struct iocb **iocbpp)
{
  	return syscall(__NR_io_submit, ctx, nr, iocbpp);
}

inline long io_getevents(aio_context_t ctx, long min_nr, long max_nr,
  		struct io_event *events, struct timespec *timeout)
{
  	return syscall(__NR_io_getevents, ctx, min_nr, max_nr, events, timeout);
}

void create_iocb(iocb& cb, int sd);

int main() {
    std::array<char, BUF_SIZE> buf{{}};
    auto listener = create_listener(false, SERVER_PORT);

    auto err = listen(listener, SOMAXCONN);
    if (err < 0) {
        std::cerr << "Listener could not start to listen" << std::endl;
        exit(1);
    }

    aio_context_t ctx;
    ctx = 0;
    struct iocb cb_listener, cb_client;
    struct iocb *cbs_for_listener[1];
    struct iocb *cbs_for_listener_and_client[2];

    // I'm suspicious about first argument
    err = io_setup(MAX_EVENTS, &ctx);
    if (err < 0) {
        std::cerr << "Cannot setup AIO" << err << std::endl;
        exit(1);
    }

    // setup I/O control block
    create_iocb(cb_listener, listener);
    cbs_for_listener[0] = &cb_listener;

    // submitting listener
    err = io_submit(ctx, 1, cbs_for_listener);
    if (err < 0) {
        std::cerr << "Cannot submit listener" << std::endl;
        exit(1);
    }

    int ready;
    // I'm suspicious about size here
    struct io_event events[MAX_EVENTS] = {};
    for (;;) {
        ready = io_getevents(ctx, 1, MAX_EVENTS, events, nullptr);
        for (int i = 0; i < ready; ++i) {
            if (events[i].data == listener) {
                // handle new connection
                auto socket_descriptor = handle_connection(listener, false);

                // adding sockets to io_submit
                // listener
                create_iocb(cb_listener, listener);
                cbs_for_listener_and_client[0] = &cb_listener;

                // new client
                create_iocb(cb_client, socket_descriptor);
                cbs_for_listener_and_client[1] = &cb_client;

                // submitting listener and new client
                err = io_submit(ctx, 2, cbs_for_listener_and_client);
                if (err < 0) {
                    std::cerr << "Cannot submit listener and new client" << std::endl;
                    exit(1);
                }
            } else {
                auto bytes_read = recv(events[i].data, &buf, BUF_SIZE, 0);
                buf[bytes_read] = '\0';
                send(events[i].data, &buf, bytes_read, 0);
                close(events[i].data);
            }
        }
    }
    io_destroy(ctx);
}


void create_iocb(iocb& cb, int sd) {
    memset(&cb, 0, sizeof(cb));
    cb.aio_data = sd;
    cb.aio_fildes = sd;
    cb.aio_lio_opcode = IOCB_CMD_POLL;
    cb.aio_buf = POLLIN;
}