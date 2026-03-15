```markdown
# 1 signals handled

## Implementation — `Server::run()`

Signals are registered at the top of `run()`, before the `poll()` loop.
`signalHandler` sets `Server::running_ = false` for all signals uniformly.

```cpp
static void signalHandler(int sig) {
    (void)sig;
    std::cout << "\n[Server] Signal received, shutting down..." << std::endl;
    Server::running_ = false;
}

void Server::run() {
    signal(SIGINT,  signalHandler); // Ctrl+C
    signal(SIGTERM, signalHandler); // kill <pid>
    signal(SIGQUIT, signalHandler); // Ctrl+\ — prevents core dump
    signal(SIGPIPE, SIG_IGN);       // ignore broken pipe on send()

    while (running_) {
        int ready = poller_->poll(1000);
        if (ready > 0)
            poller_->processEvents();
    }
}
```

## Signals Table

| Signal  | Shortcut    | Default Action        | Handled             |
|---------|-------------|-----------------------|---------------------|
| SIGINT  | Ctrl+C      | Terminate             | ✅ graceful shutdown |
| SIGTERM | kill \<pid\>| Terminate             | ✅ graceful shutdown |
| SIGQUIT | Ctrl+\      | Terminate + core dump | ✅ graceful shutdown |
| SIGPIPE | client drop | Terminate             | ✅ SIG_IGN           |

## Fix vs. Original

Original `signalHandler` had `if (signal == SIGINT)` check —
**SIGTERM was registered but silently ignored.**
Removed the check: all four signals now route through one handler.

## Ctrl+D (for reference)

`Ctrl+D` is **not a signal** — it sends EOF to `nc`'s stdin.
The server never reads stdin, so it is unaffected.
It is used only as a **test tool** to send partial commands via `nc`
to verify `MessageBuffer` reassembly.
```

***
```
<!-- ## 1. find a leak

```
valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all \
    ./ircserv 6667 password
``` -->

Run the server under valgrind, connect a client, disconnect, then stop the server with Ctrl+C — the destructor fires and valgrind reports everything.

**Terminal 1 — server:**
```bash
valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all \
    ./ircserv 6667 password
```

**Terminal 2 — client:**
```bash
nc -C 127.0.0.1 6667
# connected — server created Client* + MessageBuffer*
# Ctrl+C in nc — disconnected (disconnectClient should clean up)
```

**Terminal 1 — stop the server:**
```bash
Ctrl+C   # SIGINT → running_ = false → ~Server() is called
Ctrl+\
```

Valgrind prints the report after exit:
```
# if leak exists:
LEAK SUMMARY:
   definitely lost: 48 bytes in 2 blocks

# if all clean:
All heap blocks were freed -- no leaks are possible
```

**Important:** `~Server()` is only called on a **clean exit** via signal.
If you kill the process with `kill -9` (SIGKILL) — the destructor never runs and valgrind shows nothing.
Use only Ctrl+C / SIGTERM / SIGQUIT.



## 2. Destructor leak test

**Location:** [`tests/test_Server_leaks/test_Destructor_leak.cpp`](../tests/test_Server_leaks/test_Destructor_leak.cpp)

Simple unit test that creates a Server instance and lets the destructor clean up — no socket/bind required.

**Compile:**
```bash
cd tests/test_Server_leaks
g++ -std=c++98 -I../../include test_Destructor_leak.cpp \
    ../../src/Config.cpp ../../src/Server.cpp ../../src/Poller.cpp \
    ../../src/Parser.cpp ../../src/CommandRegistry.cpp ../../src/MessageBuffer.cpp \
    ../../src/Client.cpp ../../src/Channel.cpp ../../src/Utils.cpp \
    ../../src/Replies.cpp ../../src/commands/*.cpp -o test_leak
```

**Run with valgrind:**
```bash
valgrind --leak-check=full --track-origins=yes ./test_leak
```

**Expected result after destructor fix:**
```
All heap blocks were freed -- no leaks are possible
```