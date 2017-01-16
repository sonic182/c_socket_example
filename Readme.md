# Simple socket handle in C++

It is C++ just because I use queue library for threading usage. Feel free to implement a queue in C and to use C.

## Requirements

* g++

## Usage

* Compile with `make`
* Run with `./server <port>`
* test with curl like `curl -X GET localhost:<port>`

Sample output:
```
user@user-pc:~/path/to/folder$ curl -X GET localhost:3000
(Thread_6) Your message was:
GET / HTTP/1.1
Host: localhost:3000
User-Agent: curl/7.47.0
Accept: */*

```
