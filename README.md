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

#### Command
```bash
docker run --rm -it williamyeh/wrk -t2 -c200 -d30s "http://172.17.0.2:5555"
```

#### Results

Framework | Avg Latency `ms` | Stdev Latency `ms`| CPU % | Requests/sec | Notes
--- | --- | --- | --- | --- | ---
`binks / http-server` | 5.53 | 51.58 | 97% | 10153 | ?read socket error every request?
`sanic / uvloop` | 19.5 | 6.74 | 58% | 10161 | .
`h2o / mruby` | 8.38 | 3.32 | 100% | 23956 | *h2o has a minimum of 1x receiver 1x worker threads*
`h2o / file` | 4.29 | 3.74 | 100% | 43576 | *h2o has a minimum of 1x receiver 1x worker threads*
`fasthttp / golang:1.6` | 5.87 | 2.10| 33975 | `GOMAXPROCS=1` *still seems to use > 1 thread*

## Summary
These apples and oranges toy benchmarks shouldn't be used for anything serious. Also, since `h2o` uses a minimum of 2x threads, it should naturally return a better result. Finally, running `wrk` and a server on the *same* machine will likely affect server performance.

More experimentation is needed at this point, but this single threaded comparison of `binks` doesn't show much promise out of the gate on this hardware. A similar test on an Intel i7 showed proportionally better performance for `binks`. Also, this toy `binks` server doesn't even handle request properly showing read socket errors. Performance may improve assuming the requests are properly closed etc.

## Conclusion
One shouldn't draw any meaningful conclusions from this anecdotal toy benchmarking.

However, if one were to do so anyway, then:
1. If you want *max* speed server, try out `h2o`.
2. If you want a *very fast* web framework, build it in `golang` on `fasthttp`.
3. `libdill` and `dsock`, if used properly, show some promise.

## TODO
- [x] Fake a simple request/response server with `libdill`+`dsock`.
- [x] Use `wrk` to get early benchmarks.
- [x] Fiddle with compiler optimizations or at least remove debugging.
- [x] See if `taskset -cp 0 <pid>` affects server performance
- [ ] See how number of coroutines affects performance profile.
- [ ] Actually receive full request
- [ ] Actually parse request header/body.
- [ ] If single thread performance is good, add multi-threading.
- [x] Benchmark `h2o` and a `go` based framework for comparison

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
* [motivation](https://github.com/sustrik/libmill/issues/161)
