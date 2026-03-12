# TODO by Role — IRC Server

**Updated:** March 1, 2026  
**Server currently does NOT compile/run — all critical tasks are unfinished.**

---

## Alex (Dev A) — Network Layer · `src/Server.cpp` + `include/irc/Server.hpp`

### 🔴 CRITICAL (blocks everything)

1. **Implement `sendToClient()`**  
   Currently only a TODO comment. Every single command depends on this to send data to clients.

2. **Uncomment `channels_` map and implement channel methods**  
   In `Server.hpp` line 24: `// std::map<std::string, Channel*> channels_;` → uncomment.  
   Then implement in `Server.cpp`:
   - `getChannel(name)` — find channel by name (case-insensitive)
   - `createChannel(name)` — allocate and store new `Channel*`
   - `removeChannel(name)` — delete if empty

3. **Uncomment and implement `getClientByNickname()`**  
   In `Server.hpp` line 67: `// Client* getClientByNickname(...)` → uncomment.  
   In `Server.cpp`: iterate `clients_` map, compare by lowercase nickname.

4. **Connect Parser + CommandRegistry in `handleClientInput()`**  
   Two lines are commented out (`parser_.parse(...)` and `registry_.execute(...)`).  
   Uncomment them — without this the server receives data but never processes it.

### 🟠 IMPORTANT (causes incorrect behavior)

5. **Set real client hostname on connect**  
   In `handleNewConnection()`: after `accept()`, use `getpeername()` + `inet_ntoa()` to get client IP and call `client->setHostname(ip)`. Currently defaults to `"unknown"`.

6. **Complete `disconnectClient()` channel cleanup**  
   Commented-out block at line ~312. Must iterate client's channels, call `channel->removeClient()`, and call `removeChannel()` if empty. Without this, ghost users remain in channels.

---

## Artur (Dev B) — Config/Parser · `src/Config.cpp`

### 🟠 IMPORTANT

7. **Fix `Config::parseArgs()` to accept positional args**  
   Subject requires: `./ircserv <port> <password>`  
   Currently only parses flags like `-p`, `--port`. Positional args are ignored → password is always empty → all clients fail auth.

### 🟡 MINOR

8. **Centralize error message format via `Replies` class**  
   Some commands hardcode strings like `":server 412 ..."`. Should use `Replies::numeric(...)` everywhere for consistency.

---

## Sara (Dev C) — Commands/Logic · `src/commands/`

### 🟡 MINOR

9. **Fix operator assignment bug in `Join.cpp` (lines 116–118)**  
   `channel->addClient()` is called first, then `getClientCount() == 1` is checked — but count is already 1 so condition is unreliable. Check count **before** `addClient()` or use an `isNew` flag. Without fix, first user may not become channel operator.

### ✅ OPTIONAL

10. **Implement `NOTICE` command** — same as PRIVMSG but silently ignores errors (no replies).
11. **Implement standalone `NAMES` command** — currently NAMES is only sent on JOIN; add as separate command.

---

## Quick Status

| # | Task | Owner | Priority | Done? |
|---|------|-------|----------|-------|
| 1 | `sendToClient()` | Alex | 🔴 Critical | ❌ |
| 2 | Channel map + `getChannel/create/remove` | Alex | 🔴 Critical | ❌ |
| 3 | `getClientByNickname()` | Alex | 🔴 Critical | ❌ |
| 4 | Connect Parser in `handleClientInput()` | Alex | 🔴 Critical | ❌ |
| 5 | Real hostname on connect | Alex | 🟠 Important | ❌ |
| 6 | Channel cleanup in `disconnectClient()` | Alex | 🟠 Important | ❌ |
| 7 | `Config::parseArgs()` positional args | Artur | 🟠 Important | ❓ |
| 8 | Centralize error format | Artur | 🟡 Minor | ❓ |
| 9 | Join.cpp operator bug | Sara | 🟡 Minor | ❓ |
| 10 | NOTICE command | Sara | ✅ Optional | — |
| 11 | NAMES command | Sara | ✅ Optional | — |
