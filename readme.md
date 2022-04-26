# High efficiency echo-server
team:
- bohdan hlovatskyi: https://github.com/bohdanhlovatskyi
- sasha tsepilova: https://github.com/sasha-tsepilova
- mykhailo pasichnyk: https://github.com/fox-flex
- yaroslav romanus: https://github.com/yarkoslav

## Prerequisites

- jmeter and bzt (```pip install bzt```) for testing
- c++17 or greater

### Compilation

```bat
mkdir build
cmake .. && make
./type_of_server [port]
```

### Testing

```bat
cd testing_util

bzt config.yaml
```
Note that config.yaml has following scenario:
```yaml
scenarios:
  with_script:
    script: tcp_samples.jmx
```
This *.jmx file can be generated directly via JMeter (the only way I found to test tcp loading via Taurus)

## Methods

boost::asio based:
    - syncronous sequential
    - asynchronous single-threaded
    - synchronous multithreade
    - asyncronous multithreaded

plain sockets:
    - simple syncrhonous
    - multiprocess
    - select based (simple threaded async)

## Todo:
- !!! First and foremost: extensive code-review before moving on
- Apache JMeter Distributed Testing (https://jmeter.apache.org/usermanual/jmeter_distributed_testing_step_by_step.html)
- Read on: **Thransaction Throughput vs Threads** metric 
- Other types of servers:
  - Coroutines based
  - epoll / io-select and so on - different ways of async io
  - multithreaded plain sockets realisations

