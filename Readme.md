# Simple socket handle in code

* Compile with `gcc -o server server.c`
* Run with `./server <port>`
* test with curl like `curl -X GET localhost:<port>`

Sample output:
```
user@user-pc:~/path/to/folder$ curl -X GET localhost:3000
Your message was:
GET / HTTP/1.1
Host: localhost:3000
User-Agent: curl/7.47.0
Accept: */*

```
