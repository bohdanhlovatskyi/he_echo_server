#include "servers.hpp"

boost::asio::awaitable<void> StacklessBoost::listener() {
    auto executor = co_await boost::asio::this_coro::executor;

    boost::asio::ip::tcp::acceptor acceptor{executor,
     {boost::asio::ip::tcp::v4(), static_cast<unsigned short>(port)}};

    for (;;)
    {
        boost::asio::ip::tcp::socket socket = co_await acceptor.async_accept(
                boost::asio::use_awaitable);

        co_spawn(executor,
                 [socket = std::move(socket), this]() mutable
                 {
                     return echo(std::move(socket));
                 },
                 boost::asio::detached);
    }
}

boost::asio::awaitable<void>
StacklessBoost::echo(boost::asio::ip::tcp::socket socket) {

    try
    {
        char data[1024];
        for (;;)
        {
            std::size_t n = co_await socket.async_read_some(
                    boost::asio::buffer(data), boost::asio::use_awaitable
                    );

            co_await async_write(
                    socket, boost::asio::buffer(data, n), boost::asio::use_awaitable
                    );
        }
    }
    catch (std::exception& e)
    {
        std::printf("echo Exception: %s\n", e.what());
    }
}

void StacklessBoost::init() {
    ;
}

void StacklessBoost::run() {
    boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
    signals.async_wait([&](auto, auto){ io_context.stop(); });
    boost::asio::co_spawn(io_context, [this](){ return listener(); }, boost::asio::detached);

    io_context.run();
}