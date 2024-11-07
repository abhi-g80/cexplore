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
bin/server
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
