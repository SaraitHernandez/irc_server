# Dev A — Manual Test Guide
**What we test:** Server socket, connection, data flow, disconnection  
**Tools needed:** 2 terminals, `nc` (netcat)

---

## ⚠️ IMPORTANT — always use `nc -C`

`MessageBuffer::extractMessages()` splits messages on `\r\n` (IRC standard).  
Plain `nc` sends only `\n` → messages never extracted → server appears to ignore input.

**Always connect with:**
```bash
nc -C localhost 6667
```
The `-C` flag makes nc send `\r\n` on every Enter.

---

## Setup — start the server

```bash
# Terminal 1
make re
./ircserv 6667 testpass
```

Expected output:
```
[Server] Created with port=6667
[Server] Bound to port 6667
[Server] Listening backlog=10
[Server] Listening on port 6667
[Server] Running event loop...
```

---

## Test 1 — Client connects (handleNewConnection)

```bash
# Terminal 2
nc -C localhost 6667
```

Expected in server terminal:
```
[Server] New connection fd=5 from 127.0.0.1
```

✅ Pass: fd number printed, IP is `127.0.0.1` not `unknown`  
❌ Fail: `from unknown` → hostname task not working

---

## Test 2 — Server receives data (handleClientInput + MessageBuffer)

In the `nc` terminal, type:
```
PING :test
```
Press Enter.

Expected in server terminal:
```
[Server] recv() returned: 12
[Server] Received 12 bytes from fd=5
[Server] Complete message: PING :test
```

✅ Pass: message appears under "Complete message:"  
❌ Fail: nothing printed after recv → **you used plain `nc` without `-C`** — buffer never sees `\r\n`, messages never extracted. Use `nc -C`.

> **Observed (March 1, 2026):** ✅ Works correctly with `nc -C`. Server prints "Complete message: PING :test".

---

## Test 3 — Full registration (PASS + NICK + USER) → 001 welcome

> **🔴 BLOCKED — needs Artur (Dev B) to fix `Config::parseArgs()` first**

**Why it's blocked:**  
`./ircserv 6667 testpass` — the password `testpass` is passed as a positional argument.  
`Config::parseArgs()` currently only reads flags (`-p`, `--password`), so it ignores positional args.  
Result: password stored in config = **empty string**.  
`Pass.cpp` compares `"testpass" != ""` → always 464, always fails.

**Observed output (March 1, 2026) — what you actually see:**
```
PASS testpass              →  :ft_irc 464 * :Password incorrect
NICK alex                  →  :ft_irc 451 * :You have not registered
USER alex 0 * :Alex Test  →  :ft_irc 451 * :You have not registered
```

**What needs to happen for this test to pass:**  
Artur (Dev B) must fix `src/Config.cpp` → `Config::parseArgs()` to support positional args:
```cpp
// When argc == 3: argv[1] = port, argv[2] = password
if (argc == 3) {
    port_     = std::atoi(argv[1]);
    password_ = argv[2];
    return;
}
```
After that fix, `./ircserv 6667 testpass` will store `"testpass"` correctly and PASS will work.

**Expected output after Artur's fix:**
```
PASS testpass              →  (no reply, step 0→1)
NICK alex                  →  (no reply, step 1→2)
USER alex 0 * :Alex Test  →  :ft_irc 001 alex :Welcome to the IRC Network alex!alex@127.0.0.1
                              :ft_irc 002 alex :Your host is ft_irc
                              :ft_irc 003 alex :This server was created ...
                              :ft_irc 004 alex ft_irc ...
```

**Workaround for testing NOW (without Artur's fix):**
```bash
./ircserv 6667 ""   # empty string password
```
Then in nc:
```
PASS anythingworks
NICK alex
USER alex 0 * :Alex Test
```
Will get 001–004 because empty password matches anything... or use actual empty PASS. Depends on Pass.cpp logic.

**Note:** Dev A pipeline is confirmed working — proof:
```
PING :test  →  :ft_irc PONG ft_irc :test   ✅
```
Parser → Registry → `sendToClient()` chain functions correctly.

---

## Test 4 — sendToClient delivers data

Type in `nc` (registration not required for PING):
```
PING :test
```

Expected in `nc` terminal:
```
:ft_irc PONG ft_irc :test
```

✅ Pass: PONG received → `sendToClient()` works  
❌ Fail: nothing received → `sendToClient()` broken

> **Observed (March 1, 2026):** ✅ Works correctly. `PING :test` → `:ft_irc PONG ft_irc :test` received immediately.

---

## Test 5 — Multiple clients connect simultaneously

> **🔴 BLOCKED — depends on Test 3 (needs Artur's Config fix for full registration)**

```bash
# Terminal 2
nc -C localhost 6667

# Terminal 3
nc -C localhost 6667
```

Register both with PASS + NICK + USER (different nicks).  
Expected: both get 001–004 welcome messages independently, server doesn't crash.

**Partial test available now (without Artur's fix):**  
Connect both clients and send `PING :test` from each — both should get `PONG` independently.  
This confirms fd routing and multi-client polling work.

✅ Pass: both clients get PONG → Poller + `clients_` map handle multiple connections  
❌ Fail: second client gets no response → Poller or `handleClientInput` broken

---

## Test 6 — Client disconnects cleanly (disconnectClient)

Close `nc` with `Ctrl+C` or `Ctrl+D`.

Expected in server terminal:
```
[Server] Client disconnected fd=5
[Server] Disconnecting fd=5
[Server] Client fd=5 disconnected and cleaned up
```

✅ Pass: cleanup messages printed, server keeps running  
❌ Fail: server crash or no cleanup message → `disconnectClient()` broken

> **⚠️ Partial — channel cleanup not yet active**  
> The block that removes the client from their channels is commented out in `disconnectClient()`.  
> **Needs Sara (Dev C)** to confirm `Client::getChannels()` and `Channel::removeClient()` are ready, then uncomment the block in `Server.cpp` (~line 318).  
> Without it: disconnect cleans up socket/memory correctly, but ghost members may remain in channels.

---

## Test 7 — Wrong password (PASS command)

> **🔴 BLOCKED — needs Artur (Dev B) to fix `Config::parseArgs()` first**  
> Until then, **every** password is "wrong" because the stored password is always empty string.

```bash
nc -C localhost 6667
```

Type:
```
PASS wrongpassword
```

Expected in `nc` terminal:
```
:ft_irc 464 * :Password incorrect
```

After 3 wrong passwords, expected: connection closed by server.

✅ Pass: ERR_PASSWDMISMATCH (464) received, kicked after 3 attempts  
❌ Fail: no response → `Pass.cpp` or `sendToClient()` broken  
⚠️ Currently: 464 is returned even for the correct password — blocked by Artur's Config bug

---

## Test 8 — Ctrl+C shuts down server gracefully

In server terminal, press `Ctrl+C`.

Expected:
```
[Server] Received SIGINT, shutting down...
[Server] Event loop stopped
```

✅ Pass: clean exit  
❌ Fail: crash or hanging → signal handler broken

---

## Test 9 — Valgrind: no memory leaks

**Terminal 1:**
```bash
make re
valgrind --leak-check=full --track-origins=yes ./ircserv 6667 testpass
```

**Terminal 2:**
```bash
nc -C localhost 6667
PING :test
```
Then press Ctrl+C in Terminal 2, then Ctrl+C in Terminal 1 (valgrind server).

Expected in valgrind output:
```
HEAP SUMMARY:
    definitely lost: 0 bytes in 0 blocks
    indirectly lost: 0 bytes in 0 blocks
```

✅ Pass: `definitely lost: 0 bytes`  
❌ Fail: any `definitely lost` bytes → destructor cleanup incomplete

**Fixed leak (March 1, 2026):** `Server::~Server()` was not deleting `buffers_` and `channels_` maps.
- `definitely lost: 32 bytes` = leaked `MessageBuffer*` object
- `indirectly lost: 31 bytes` = internal `std::string buffer_` inside it
- Fix: added delete loops for both maps in destructor

---

## Quick checklist

| # | Test | Blocked by | Result |
|---|------|-----------|--------|
| 1 | Connect: fd + IP logged | — | ✅ Works |
| 2 | Recv + extract message | — | ✅ Works (use `nc -C`) |
| 3 | PASS+NICK+USER → 001 welcome | **Artur (Dev B)**: fix `Config::parseArgs()` | 🔴 Blocked |
| 4 | PING → PONG (sendToClient) | — | ✅ Works |
| 5 | Multi-client PING | — | ✅ Partial (PING works, full reg blocked) |
| 6 | Disconnect cleanup (socket) | — | ✅ Works |
| 6b | Disconnect cleanup (channels) | **Sara (Dev C)**: `Client::getChannels()` + `Channel::removeClient()` | ❌ Not done |
| 7 | Wrong password → 464 x3 → kick | **Artur (Dev B)**: fix `Config::parseArgs()` | 🔴 Blocked |
| 8 | Ctrl+C graceful shutdown | — | ✅ Works |
| 9 | Valgrind: 0 leaks after Ctrl+C | — | ✅ Fixed (March 1, 2026) |
