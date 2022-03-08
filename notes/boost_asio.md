
## IO
**boost::asio::ip::tcp::socket::read_some**
- only garanteed to read at least one byte
- alternative: receive method, even better: asio::read()
- write methods' api looks in the same manner
- asio::read() - blocking function, will block while exaclt n bytes are read or an error has occured

- async write:
The application is run by a single thread, in the context of which the application's main() entry point function is called. Note that Boost.Asio may create additional threads for some internal operations, but it guarantees that no application code is executed in the context of those threads.

Boost.Asio captures the thread of execution and uses it to call the callback functions associated with asynchronous operations when they get completed.

- ios.run() blocks then while there is at least one pending async op
- buffer should be available whole time while callback is not called
- by analogy, ther eis async::write() which guarantees that all the bytes will be written
- callbacks should have signature specified in docs

## Canceling async operations
- cancellation is cool
- ios.run() in other thread :
```c++
  std::thread worker_thread([&ios](){
         try {
           ios.run();
         }
         catch (system::system_error &e) {
           std::cout << "Error occured!"
           << " Error code = " << e.code()
           << ". Message: " << e.what();
} });
```
- then in this thread we may use:
```c++
sock->cancel(), which will canccel all the async ops binded to the socket binded to the ios 
```
- we call the socket object's cancel() method to cancel the initiated connection operation. At this point, if the operation has not yet finished,
it will be canceled and the corresponding callback will be invoked

## Socket shutdown
- shutdown means stop of transmission: results in a special service message being sent to the receiver, informing the receiver that the message is over and the sender will not send anything else using the current connection.
- Closing a socket assumes returning the socket and all the other resources associated with it back to the operating system.

```c++
asio::write(socket, asio::buffer(buf))
socket.shutdown(asio::socket_base::shutdown_send) // we won't send any new info
```
- no need to explicitly close the socket as the destructor will do this automatically

## Implementing server applications

### Synchronous server
- push-style communication
- pseudo parallelism
- synchronous servers are exposed to bad network connection, as they may cause hangs through web
- synchronous ops cancellation: polling on stop conditions 
- dummpy request to make acc.Accept() method unblock and then terminate server 
- nonblocking sockets - reactive
- synchronous servers are vulnerable to such hangings

### Synchronous parallel TCP server
- it seems that the book is written poorly, and the whole problem is about graceful shutdown ? 

### Asynchronous TCP server
- Spawn one or more threads of control and add them to the pool of threads that run the Boost.Asio event loop.
 
