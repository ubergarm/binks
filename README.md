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

#### Setup
* All test apps running in Docker containers / network bridge
* Configure server to use single worker thread (`h2o` uses 2x threads)
* Respond with 200 OK `application/json` `{"hello":"world"}`.
* Intel(R) Core(TM)2 Duo CPU P8600 @ 2.40GHz

#### Results
*Note*: `wrk` seems to show socket read errors which leads me to believe it is attempting to re-use TCP connections which assumes `KeepAlive`. This likely skewed the results of my first test, so I've removed those results for now and plan to use something other than `wrk` for benchmarking.

Using `wrk`:
```bash
docker run --rm -it williamyeh/wrk -t2 -c20 -d30s "http://172.17.0.2:5555"
```

Framework | Avg Latency `ms` | Stdev Latency `ms`| CPU % | Requests/sec | Notes
--- | --- | --- | --- | --- | ---
`binks / http-server` | x | x | x | x | `wrk` seems to show read errors if keep alive not implemented
`sanic / uvloop` | x | x | x | x | -
`h2o / mruby` | x | x | x | x | *h2o has a minimum of 1x receiver 1x worker threads*
`h2o / file` | x | x | x | x | *h2o has a minimum of 1x receiver 1x worker threads*
`fasthttp / golang:1.6` | x | x | x | x | `GOMAXPROCS=1` *still seems to use > 1 thread*

Using `weighttp`:
```bash
docker run uzyexe/weighttp -n 100000 -c 10 -t 2 http://172.17.0.2:5555
```

Using `ab`:
```bash
TODO
```

Using [vegeta](https://github.com/tsenart/vegeta/releases):
```bash
echo "GET http://localhost:5555/" | vegeta attack -keepalive 0 -cpus 2 -rate 10000 -duration=5s | tee results.bin | vegeta report
```

*TODO*

## Summary
These apples and oranges toy benchmarks shouldn't be used for anything serious. Also, since `h2o` uses a minimum of 2x threads, it should naturally return a better result. Running `wrk` and a server on the *same* machine will likely affect server performance. Finally, servers that support `Keep-Alive` will skew

More experimentation is needed at this point as there is more to the story than simply `resests/second` for trivial requests/responses.

## Conclusion
There are many frameworks available in a variety of languages for implementing web-services, however `libdill`+`dsock` show promise for implementing efficient lowish level network related services with [structured concurrency](http://250bpm.com/blog:71) patterns.

## TODO
- [x] Fake a simple request/response server with `libdill`+`dsock`.
- [x] Use `wrk` to get early benchmarks.
- [x] Fiddle with compiler optimizations or at least remove debugging.
- [x] See if `taskset -cp 0 <pid>` affects server performance
- [x] See how number of coroutines affects performance profile.
- [x] Benchmark `h2o` and a `go` based framework for comparison
- [ ] Address KeepAlive and/or choose a different tool than `wrk`
- [ ] Actually receive full request
- [ ] Actually parse request header/body.
- [ ] If single thread performance is good, add multi-threading.

## References
* [sustrik/libdill](https://github.com/sustrik/libdill)
* [sustrick/dsock](https://github.com/sustrik/dsock)
* [zmq](http://zguide.zeromq.org/page:all#Multithreading-with-ZeroMQ)
* [nanomsg](http://nanomsg.org/)
* [h2o/picohttpparser](https://github.com/h2o/picohttpparser)
* [h2o](http://blog.kazuhooku.com/2014/11/the-internals-h2o-or-how-to-write-fast.html)
* [channelcat/sanic](https://github.com/channelcat/sanic)
* [uvloop & libuv](https://magic.io/blog/uvloop-blazing-fast-python-networking/)
* [valyala/fasthttp](https://github.com/valyala/fasthttp)
* [smallnest-go-web-framework-benchmark](https://github.com/smallnest/go-web-framework-benchmark)
* [tsenart/vegeta](https://github.com/tsenart/vegeta)
* [motivation](https://github.com/sustrik/libmill/issues/161)
