# MessageBuffer Integration Test 2

**Terminal 1 - Server:**
```bash
./ircserv 6667 password
```

**Terminal 2 - Client:**
## 1. full message:
```
printf 'NICK test1\r\n' | nc localhost 6667
```
After Test 1, Ctrl+C is required in Terminal 2 because nc hangs — the server is waiting for a USER command after NICK and doesn't respond to the client. nc in turn is waiting for a response from the server — a deadlock:
```
nc sent:        NICK test1\r\n
Server thinks:  "waiting for USER..."
nc thinks:      "waiting for response..."
→ both wait forever
```

## 2.1 interrupted message:
an important nuance
For this one, Ctrl+C is needed on the client side after ~3 seconds from launch, because the command won't exit on its own — just wait:

```
(printf "NICK te"; sleep 1; printf "st\r\n") | nc localhost 6667
```
Launch it → wait ~2-3 seconds

nc will hang (waiting for a response from the server)
 → press Ctrl+C in Terminal 2

## 2.2  terminal of server after:
```
[Server] recv() returned: 7           ← first chunk "NICK te"
[Server] Received 7 bytes from fd=...
                                       ← 1 second pause
[Server] recv() returned: 4           ← second chunk "st\r\n"
[Server] Received 4 bytes from fd=...
[Server] Complete message: NICK test  ← reassembled and delivered!
```
## 3.1 two messages in one package:
```
printf 'NICK alice\r\nUSER alice 0 * :Alice\r\n' | nc localhost 6667
```
## 3.2 expected on server terminal:
```
[Server] recv() returned: 44
[Server] Received 44 bytes from fd=...
[Server] Complete message: NICK alice
[Server] Complete message: USER alice 0 * :Alice

```


