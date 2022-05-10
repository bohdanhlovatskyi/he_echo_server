# High efficiency echo-server
team:
- bohdan hlovatskyi: https://github.com/bohdanhlovatskyi
- sasha tsepilova: https://github.com/sasha-tsepilova
- mykhailo pasichnyk: https://github.com/fox-flex
- yaroslav romanus: https://github.com/yarkoslav

## TODO:
- debug io_submit and select (Linux only stuff)
- read docs on fork and get why it uses so much sockets
- read more on internals of sockets - how to get their limit
- from the previous, manually estimate bounds on productivity and what can limit it
- what to do with coroutines? thread pools? 
- discuss prev and testing overally

## Prerequisites

- c++17 or greater

### Compilation

```shell
mkdir build
cmake .. && make
./start_server [port] [number_of_implementation]
```

### Testing

TODO

## Methods

