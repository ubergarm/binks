binks
===
A minimal concurrent HTTP 1.1 server in C.

## Goal
Implement a minimal HTTP 1.1 server in standard C building on top of:

* `libdill` - within thread concurrent coroutines and channels
* `dsock` - nifty composable protocol library
* `zmq` - intraprocess communication
* `picohttpparser` for fast HTTP 1.1 header parsing

## Status
Just getting started...

*NOTE*: Currently the test servers crash after ~32759 connections handled...

This seems consistent across both `debian:jessie` and `alpine:edge` base images.

## Building
```bash
docker build -t ubergarm/binks .
```

## Running
```bash
docker run --rm -it -v `pwd`:/app -p 5555:5555 ubergarm/binks /bin/sh
```

## Benchmarking
```bash
docker run --rm -it williamyeh/wrk -t4 -c400 -d30s "http://172.17.0.2:5555"
```

## Notes
1. `make check` can occasionally fail for both `libdill` and `dsock`
1. May need to `docker run` with [--security-opt seccomp=unconfined](http://stackoverflow.com/questions/35860527/warning-error-disabling-address-space-randomization-operation-not-permitted/35860616)
1. Running with `--net=host` may affect performance.
1. May need to [increase ephemeral ports](https://www.nginx.com/blog/overcoming-ephemeral-port-exhaustion-nginx-plus/).

## References
* [sustrik/libdill](https://github.com/sustrik/libdill)
* [sustrick/dsock](https://github.com/sustrik/dsock)
* [zmq](http://zguide.zeromq.org/page:all#Multithreading-with-ZeroMQ)
* [h2o/picohttpparser](https://github.com/h2o/picohttpparser)
* [h2o](http://blog.kazuhooku.com/2014/11/the-internals-h2o-or-how-to-write-fast.html)
* [motivation](https://github.com/sustrik/libmill/issues/161)
*
