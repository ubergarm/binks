binks
===
A minimal concurrent toy HTTP 1.1 server in C.

## Goal
Implement a minimal HTTP 1.1 server in standard C building on top of:

* `libdill` - within thread concurrent coroutines and channels
* `dsock` - nifty composable protocol library
* `zmq` or `nanomsg` - intraprocess communication
* `picohttpparser` for fast HTTP 1.1 header parsing

## Building
```bash
docker build -t ubergarm/binks .
```

## Running
```bash
docker run --rm -it -v `pwd`:/app -p 5555:5555 ubergarm/binks /bin/sh
```

```bash
cd src
make clean && make && ./dsock-n-server 5555 10
```

## Benchmarking
*NOTE* This is still a toy server that isn't properly parsing requests.

```bash
docker run --rm -it williamyeh/wrk -t2 -c200 -d30s "http://172.17.0.2:5555"
```
#### dsock-n-server
```
Binks: Goin' Fast @ http://0.0.0.0:5555
INFO: Spinning up 10 workers per 1 thread(s)...

  2 threads and 200 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     8.04ms   82.04ms   1.67s    98.41%
    Req/Sec    28.71k     1.13k   29.92k    88.17%
  1714134 requests in 30.06s, 143.86MB read
  Socket errors: connect 0, read 1714131, write 0, timeout 7
Requests/sec:  57016.44
Transfer/sec:      4.79MB
```

### [ubergarm/sanic-alpine](https://github.com/ubergarm/sanic-alpine)
```
Goin' Fast @ http://0.0.0.0:8000

  2 threads and 200 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     5.88ms  819.88us  37.54ms   93.47%
    Req/Sec    17.11k   670.00    18.18k    68.00%
  1021887 requests in 30.10s, 132.54MB read
Requests/sec:  33954.29
Transfer/sec:      4.40MB
```
## Summary
This apples and oranges toy comparison is to suss out if a `libdill`+`dsock` based web framework has potential out-perform other existing solutions in terms of requests/second etc.

More experimentation is needed at this point, but this single threaded comparison of `Binks` vs `sanic` holds *some* promise IMO.

## TODO
- [x] Fake a simple request/response server with `libdill`+`dsock`.
- [x] Use `wrk` to get early benchmarks.
- [ ] Fiddle with compiler optimizations or at least remove debugging.
- [ ] See how number of coroutines affects performance profile.
- [ ] Actually receive full request
- [ ] Actually parse request header/body.
- [ ] If single thread performance is good, add multi-threading.
- [ ] Benchmark `h2o` and a `go` based framework for comparison

## References
* [sustrik/libdill](https://github.com/sustrik/libdill)
* [sustrick/dsock](https://github.com/sustrik/dsock)
* [zmq](http://zguide.zeromq.org/page:all#Multithreading-with-ZeroMQ)
* [nanomsg](http://nanomsg.org/)
* [h2o/picohttpparser](https://github.com/h2o/picohttpparser)
* [h2o](http://blog.kazuhooku.com/2014/11/the-internals-h2o-or-how-to-write-fast.html)
* [channelcat/sanic](https://github.com/channelcat/sanic)
* [uvloop & libuv](https://magic.io/blog/uvloop-blazing-fast-python-networking/)
* [motivation](https://github.com/sustrik/libmill/issues/161)
