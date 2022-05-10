# High efficiency echo-server
team:
- bohdan hlovatskyi: https://github.com/bohdanhlovatskyi
- sasha tsepilova: https://github.com/sasha-tsepilova
- mykhailo pasichnyk: https://github.com/fox-flex
- yaroslav romanus: https://github.com/yarkoslav

## TODO:
- debug select (Linux only stuff)
- read docs on fork and get why it uses so much sockets
- read more on internals of sockets - how to get their limit
- from the previous, manually estimate bounds on productivity and what can limit it
- clear out all the todos  
- what to do with coroutines? thread pools?
- what to do with Windows? 
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

