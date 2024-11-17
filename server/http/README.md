Webby
=====

A small webserver in C. Written to explore and understand various nuances of TCP and HTTP.


Build
-----

```
make clean && make
```

Test
----

Run the binary
```
export WEBBY_ROOT=/var/www/mywebsite
bin/webby
```
Use curl to send a GET request

```
$ curl localhost:9090 -D -
HTTP/1.1 200 OK
Date: Thu, 07 Nov 2024 22:10:39 GMT
Server: Webby
Content-Length: 68
Content-Type: text/html

<html><h1><center><b>It could be working!</b></center></h1></html>
```

Benchmark
---------
Using the [wrk](https://github.com/wg/wrk) program for benchmarking.

```
$ ./wrk -t 10 -c 500 -d 10s --latency http://localhost:9090
Running 10s test @ http://localhost:9090
  10 threads and 500 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    12.35ms    2.14ms  54.51ms   90.10%
    Req/Sec     4.06k     1.06k   32.27k    99.60%
  Latency Distribution
     50%   12.24ms
     75%   13.19ms
     90%   14.12ms
     99%   16.97ms
  403903 requests in 10.10s, 70.88MB read
  Socket errors: connect 0, read 403890, write 0, timeout 0
Requests/sec:  39989.13
Transfer/sec:      7.02MB
```
