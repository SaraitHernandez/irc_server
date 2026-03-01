# Dev A (Alex) — Progress Report
**Date:** March 1, 2026  
**Based on:** MISSING_TASKS.md  
**Build: `make re` exits 0 — compiles clean ✅**

---

## ✅ DONE (verified in src/Server.cpp)

| Task (MISSING_TASKS.md) | Method | Verified |
|--------------------------|--------|---------|
| Task 1 — sendToClient() | `Server::sendToClient()` — end of file, `send()` + `\r\n` + EPIPE/EAGAIN handling | ✅ |
| Task 2 — Channel management | `getChannel()`, `createChannel()`, `removeChannel()` — all implemented with `Utils::toLower()` | ✅ |
| Task 2 — channels_ map | `std::map<std::string, Channel*> channels_` — active in Server.hpp | ✅ |
| Task 3 — getClientByNickname() | Iterates `clients_`, compares lowercase nickname | ✅ |
| Task 4 — Parser + Registry | `Command cmd; if (parser_.parse(..., cmd)) registry_.execute(*this, *client, cmd);` | ✅ |
| Task 6 — Real hostname | `inet_ntoa(clientAddr.sin_addr)` → `client->setHostname()` in `handleNewConnection()` | ✅ |
| Extra — getPassword() | Returns `config_.getPassword()` — needed by Pass.cpp | ✅ |
| Extra — sendResponse() | Formats `":server numeric params :trailing"` and calls `sendToClient()` | ✅ |
| Extra — Destructor memory leak | `~Server()`: added delete loops for `buffers_` and `channels_` maps — confirmed by valgrind | ✅ |

---

## ❌ NOT DONE

### Task 7 — Channel cleanup in `disconnectClient()` ❌
**Status:** Code block is commented out in `Server.cpp` (~line 318).  
**Blocked by:** Sara (Dev C) must confirm `Client::getChannels()` and `Channel::removeClient()` are ready.  
**Impact:** When a client disconnects, they remain as ghost member in all channels.

### broadcastToChannel() — declared, NOT implemented ⚠️
**Status:** Declared in `Server.hpp`, but no implementation exists in `Server.cpp`.  
**Impact:** Not currently blocking (commands use `channel->broadcast()` directly). Will be needed if any command switches to calling `server.broadcastToChannel()`.  
**Guide:** See `docs/BROADCAST_IMPLEMENTATION.md`.

---

## Waiting on others

| What | Who | Needed for |
|------|-----|-----------|
| Task 5 — `Config::parseArgs()` positional args | **Artur (Dev B)** | `./ircserv 6667 password` format |
| Task 7 unblock | **Sara (Dev C)** | `Client::getChannels()` + `Channel::removeClient()` confirmed |
