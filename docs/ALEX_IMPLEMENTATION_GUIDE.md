# Alex (Dev A) — Implementation Guide for Missing Server Methods

**File:** `src/Server.cpp` + `include/irc/Server.hpp`  
**Status:** Tasks 1–3 already applied by Copilot on March 1, 2026. Task 4 partially fixed.

---

## What was already done before this guide

### `Server.hpp` — headers and declarations added

```cpp
// These includes were added:
#include "Channel.hpp"
#include "Parser.hpp"
#include "CommandRegistry.hpp"

// These members were added to private section:
Parser          parser_;
CommandRegistry registry_;
std::map<std::string, Channel*> channels_;   // was commented out

// These methods were added to public section:
Channel* getChannel(const std::string& name);
Channel* createChannel(const std::string& name);
void     removeChannel(const std::string& name);
```

### `Server.cpp` — includes added

```cpp
#include "irc/Channel.hpp"
#include "irc/Utils.hpp"
```

### `Server.cpp` — 5 methods implemented (appended at bottom of file)

---

## How each method works

---

### `getClientByNickname(nickname)`

**What it does:** Searches all connected clients and returns the one whose nickname matches.  
**Case:** IRC nicknames are case-insensitive — `BOB`, `bob`, `Bob` all refer to same user.  
**How:** Convert search string to lowercase, compare against `client->getNickname()` which is always stored lowercase.

```cpp
Client* Server::getClientByNickname(const std::string& nickname) {
    std::string lowerNick = Utils::toLower(nickname);
    std::map<int, Client*>::iterator it;
    for (it = clients_.begin(); it != clients_.end(); ++it) {
        if (it->second->getNickname() == lowerNick)
            return it->second;
    }
    return NULL;
}
```

**Returns:** `Client*` if found, `NULL` if not found.  
**Used by:** `Nick.cpp`, `Privmsg.cpp`, `Invite.cpp`, `Kick.cpp`, `Mode.cpp`

---

### `getChannel(name)`

**What it does:** Looks up a channel by name in `channels_` map.  
**Case:** Channel names are case-insensitive — `#Test` and `#test` are the same channel.  
**How:** The map key is always stored lowercase. Convert input to lowercase before lookup.

```cpp
Channel* Server::getChannel(const std::string& name) {
    std::string lower = Utils::toLower(name);
    std::map<std::string, Channel*>::iterator it = channels_.find(lower);
    if (it != channels_.end())
        return it->second;
    return NULL;
}
```

**Returns:** `Channel*` if found, `NULL` if channel does not exist.  
**Used by:** `Join.cpp`, `Part.cpp`, `Privmsg.cpp`, `Invite.cpp`, `Kick.cpp`, `Mode.cpp`, `Topic.cpp`

---

### `createChannel(name)`

**What it does:** Allocates a new `Channel` object and stores it in the map.  
**If already exists:** Returns the existing channel (no duplicates).  
**Key storage:** lowercase. Display name (original case) is preserved inside `Channel` object itself.

```cpp
Channel* Server::createChannel(const std::string& name) {
    Channel* existing = getChannel(name);
    if (existing)
        return existing;
    std::string lower = Utils::toLower(name);
    Channel* channel = new Channel(name);   // Channel stores original case internally
    channels_[lower] = channel;
    return channel;
}
```

**Returns:** The newly created (or already existing) `Channel*`.  
**Used by:** `Join.cpp` — called when client joins a channel that doesn't exist yet.

---

### `removeChannel(name)`

**What it does:** Deletes a channel from the map and frees its memory — but **only if it is empty**.  
**Why only if empty:** A channel with members should never be deleted while users are in it.

```cpp
void Server::removeChannel(const std::string& name) {
    std::string lower = Utils::toLower(name);
    std::map<std::string, Channel*>::iterator it = channels_.find(lower);
    if (it == channels_.end())
        return;
    if (it->second->isEmpty()) {
        delete it->second;
        channels_.erase(it);
    }
}
```

**Used by:** `Part.cpp`, `Quit.cpp`, `disconnectClient()` — after removing a client from a channel, check if empty and clean up.

---

### `sendToClient(clientFd, message)`

**What it does:** Sends a formatted IRC message string to a client over their socket.  
**`\r\n`:** IRC protocol requires all messages to end with `\r\n`. This method adds it automatically if missing.  
**Error handling:**
- `EPIPE` / `ECONNRESET` → client disconnected → call `disconnectClient()`  
- `EAGAIN` / `EWOULDBLOCK` → socket buffer full → silently ignore (non-blocking mode)  
- Other errors → log to stderr

```cpp
void Server::sendToClient(int clientFd, const std::string& message) {
    std::string msg = message;
    if (msg.size() < 2 || msg.substr(msg.size() - 2) != "\r\n")
        msg += "\r\n";

    ssize_t sent = send(clientFd, msg.c_str(), msg.size(), 0);
    if (sent < 0) {
        if (errno == EPIPE || errno == ECONNRESET)
            disconnectClient(clientFd);
        else if (errno != EAGAIN && errno != EWOULDBLOCK)
            std::cerr << "[Server] send() error fd=" << clientFd
                      << ": " << strerror(errno) << std::endl;
    }
}
```

**Used by:** Every single command handler (`Nick.cpp`, `Join.cpp`, `Pass.cpp`, etc.)

---

## Task 4 — Parser integration in `handleClientInput()`

**Status:** The old commented code used wrong types (`IRCMessage` doesn't exist).  
**Fix already applied:** The correct call is now:

```cpp
// In handleClientInput(), inside the messages loop:
Command cmd;
if (parser_.parse(messages[i], cmd)) {
    registry_.execute(*this, *client, cmd);
}
```

**Why this is correct:**
- `Parser::parse()` signature: `bool parse(const std::string&, Command&)` — takes `Command` by reference, not return value
- `CommandRegistry::execute()` signature: `bool execute(Server&, Client&, const Command&)` — takes references, not pointers
- There is no `IRCMessage` type in this codebase — `Command` struct is the parsed message type

---

## Task 5 — Hostname from real IP (still TODO)

In `handleNewConnection()`, after `accept()`, add:

```cpp
// After: int clientFd = accept(serverSocketFd_, (struct sockaddr*)&clientAddr, &clientLen);
char ipStr[INET_ADDRSTRLEN];
inet_ntop(AF_INET, &clientAddr.sin_addr, ipStr, sizeof(ipStr));
client->setHostname(std::string(ipStr));
```

Requires `#include <arpa/inet.h>` in `Server.cpp`.  
Without this, all clients show as `nick!user@unknown` in messages.

---

## Task 6 — Channel cleanup in `disconnectClient()` (still TODO)

The commented block at line ~312 in `Server.cpp` must be uncommented when `Channel::removeClient()` and `Client::getChannels()` are confirmed available (Sara's code). The logic is:

```cpp
std::vector<std::string> chans = client->getChannels();
for (size_t i = 0; i < chans.size(); ++i) {
    Channel* chan = getChannel(chans[i]);
    if (chan) {
        chan->removeClient(client);
        if (chan->isEmpty())
            removeChannel(chans[i]);
    }
}
```

This must run **before** `delete client` in `disconnectClient()`.
