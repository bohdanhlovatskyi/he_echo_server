//
// Created by yarmus on 4/26/22.
//

#include "servers.hpp"
#include "common_sockets.hpp"

void AsyncIOSubmit::create_iocb_(iocb& cb, int sd) {
    memset(&cb, 0, sizeof(cb));
    cb.aio_data = sd;
    cb.aio_fildes = sd;
    cb.aio_lio_opcode = IOCB_CMD_POLL;
    cb.aio_buf = POLLIN;
}

void AsyncIOSubmit::init() {
    listener_fd = create_listener(false, port);

    ctx = 0;
    // I'm suspicious about first argument
    auto err = io_setup(MAX_EVENTS, &ctx);
    if (err < 0) {
        throw std::runtime_error{"Cannot setup AIO"};
    }

    // setup I/O control block
    create_iocb_(cb_listener, listener_fd);
    cbs_for_listener[0] = &cb_listener;

    // submitting listener
    err = io_submit(ctx, 1, cbs_for_listener);
    if (err < 0) {
        throw std::runtime_error{"Cannot submit listener"};
    }
}

void AsyncIOSubmit::run() {
    std::vector<char> buf{};
    buf.reserve(buf_size);

    int ready;
    // I'm suspicious about size here
    struct io_event events[MAX_EVENTS] = {};

    for (;;) {
        ready = io_getevents(ctx, 1, MAX_EVENTS, events, nullptr);
        for (int i = 0; i < ready; ++i) {
            if (events[i].data == listener_fd) {
                // handle new connection
                auto socket_descriptor = handle_connection(listener_fd, false);

                // adding sockets to io_submit
                // listener
                create_iocb_(cb_listener, listener_fd);
                cbs_for_listener_and_client[0] = &cb_listener;

                // new client
                create_iocb_(cb_client, socket_descriptor);
                cbs_for_listener_and_client[1] = &cb_client;

                // submitting listener and new client
                auto err = io_submit(ctx, 2, cbs_for_listener_and_client);
                if (err < 0) {
                    std::cerr << "Cannot submit listener and new client" << std::endl;
                    exit(1);
                }
            } else {
                auto bytes_read = read_msg(events[i].data, buf.data(), buf_size);
                if (bytes_read > 0) {
                    std::cout << "input msg: " << buf.data() << std::endl;
                    write_msg(events[i].data, buf.data(), bytes_read);

                    // submitting the same client
                    create_iocb_(cb_client, events[i].data);
                    cbs_for_client[0] = &cb_client;
                    auto err = io_submit(ctx, 1, cbs_for_client);
                    if (err < 0) {
                        std::cerr << "Cannot submit listener and new client" << std::endl;
                        exit(1);
                    }
                } else {
                    close(events[i].data);
                }
            }
        }
    }
}
