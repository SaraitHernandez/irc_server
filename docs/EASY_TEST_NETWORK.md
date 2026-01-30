# 1st terminal
```
make
./ircserv 6667 password
```
# 2nd terminal
## 1
```
nc -C 127.0.0.1 6667
NICK test
```
## 2
```
Ctrl+C
```
*** Note: Ctrl+D does not work with netcat and probably should not ***
