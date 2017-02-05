binks
===
A minimal concurrent toy HTTP 1.1 server in C.

## Goal
Implement a minimal HTTP 1.1 server in standard C building on top of:

* `libdill` - within thread concurrent coroutines and channels
* `dsock` - nifty composable protocol library

## Building
```bash
docker build -t ubergarm/binks .
```

## Running
Run container:
```bash
docker run --rm -it -v `pwd`:/app -p 5555:5555 ubergarm/binks /bin/sh
```

From within container:
```bash
cd /app/src/dsock-servers
make
./server4 5555 20
```
## dsock servers

* `server1` - accept connections and immediately close them
* `server2` - same as previous but with coroutines and a channel
* `server3` - receive request header, respond 200 OK, then close connection
* `server4` - receive full header, send full response, then close connection
* `server5` - same as previous but keep single connection alive

## Benchmarking
Note that standard `wrk` doesn't support disabling keep-alive.
```bash
docker run --rm -it williamyeh/wrk -t2 -c20 -d30s http://172.17.0.2:5555
```

Nominal unoptimized results:
`server4 5555 3`:
```bash
docker run --rm -it williamyeh/wrk -t6 -c6 -d10s http://172.17.0.2:5555
Running 10s test @ http://172.17.0.2:5555
  6 threads and 6 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency   314.51us  384.64us  24.90ms   96.37%
    Req/Sec     1.40k     1.27k    4.85k    71.50%
  83694 requests in 10.04s, 10.54MB read
Requests/sec:   8335.29
Transfer/sec:      1.05MB
```

`server5 5555 30000`:
```bash
$ docker run --rm -it williamyeh/wrk -t6 -c30000 -d30s http://172.17.0.2:5555
Running 30s test @ http://172.17.0.2:5555
  6 threads and 30000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency   602.53ms  165.70ms   1.90s    85.99%
    Req/Sec     7.57k     8.33k   36.34k    83.38%
  845784 requests in 30.10s, 110.53MB read
  Socket errors: connect 1769, read 2, write 0, timeout 209
Requests/sec:  28099.85
Transfer/sec:      3.67MB
```
## Summary
I'm still learning how to use `libdill`+`dsock` and these examples
may crash, act screwy, and likely are note a good reference
implementation. Cheers!

## References
* [sustrik/libdill](https://github.com/sustrik/libdill)
* [sustrick/dsock](https://github.com/sustrik/dsock)
* [nanomsg](http://nanomsg.org/)
* [zmq](http://zguide.zeromq.org/page:all#Multithreading-with-ZeroMQ)
* [h2o/picohttpparser](https://github.com/h2o/picohttpparser)
* [h2o](http://blog.kazuhooku.com/2014/11/the-internals-h2o-or-how-to-write-fast.html)
* [channelcat/sanic](https://github.com/channelcat/sanic)
* [uvloop & libuv](https://magic.io/blog/uvloop-blazing-fast-python-networking/)
* [valyala/fasthttp](https://github.com/valyala/fasthttp)
* [smallnest-go-web-framework-benchmark](https://github.com/smallnest/go-web-framework-benchmark)
* [tsenart/vegeta](https://github.com/tsenart/vegeta)
* [motivation](https://github.com/sustrik/libmill/issues/161)
