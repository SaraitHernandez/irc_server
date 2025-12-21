# Team Conventions & Module Contracts

This document defines the interfaces and contracts between modules in the IRC server implementation.

---

## 1. Poll() Responsibility

**Rule: Only the Poller class calls `poll()`**

- The `Poller` class (or equivalent polling mechanism) is the **sole** owner of the `poll()` system call
- All I/O multiplexing is centralized in this class
- Other classes must not directly call `poll()`, `select()`, `epoll()`, or `kqueue()`
- The Poller class notifies other components (Server, Client handlers) when events occur

**Implementation Contract:**
```cpp
// Only Poller class should have:
int poll(pollfd* fds, nfds_t nfds, int timeout);
```

---

## 2. Sending Responses to Clients

**Rule: Responses are sent via `Server::sendToClient()`**

- All responses to clients must go through `Server::sendToClient(int clientFd, const std::string& message)`
- Command handlers should NOT directly write to socket file descriptors
- This centralizes I/O operations and allows for proper error handling and logging

**Interface Contract:**
```cpp
class Server {
public:
    // Primary method for sending data to a client
    // Throws exception or returns error code on failure
    void sendToClient(int clientFd, const std::string& message);
    
    // Convenience method for sending formatted IRC response
    void sendResponse(int clientFd, const std::string& numeric, 
                      const std::string& params, const std::string& trailing);
};
```

**Usage Pattern:**
```cpp
// In Command Handler:
server.sendToClient(clientFd, "PRIVMSG #channel :Hello\r\n");

// NOT:
write(clientFd, "PRIVMSG #channel :Hello\r\n", ...); // ❌ FORBIDDEN
```

---

## 3. Parsed Command Structure Format

**Structure: `IRCMessage`**

All parsed IRC messages follow this structure:

```cpp
// includes/Message.hpp
struct IRCMessage {
    std::string prefix;              // Optional origin (e.g., "nick!user@host")
    std::string command;             // Command name (e.g., "NICK", "PRIVMSG", "JOIN")
    std::vector<std::string> params; // Command parameters (space-separated)
    std::string trailing;            // Trailing parameter (after ':')
    std::string raw;                 // Original unparsed message
    
    // Helper methods
    bool hasPrefix() const;
    std::string getParam(size_t index) const;
    size_t paramCount() const;
};
```

**Example:**
```
Input:  ":nick!user@host PRIVMSG #channel :Hello world\r\n"
Output:
  prefix = "nick!user@host"
  command = "PRIVMSG"
  params = ["#channel"]
  trailing = "Hello world"
  raw = ":nick!user@host PRIVMSG #channel :Hello world\r\n"
```

**Parser Contract:**
- Parser must handle incomplete messages (partial data)
- Parser must validate basic IRC message format
- Parser must separate prefix, command, params, and trailing correctly
- All messages end with `\r\n` (CRLF)

---

## 4. Client Interface

**Class: `Client`**

```cpp
// includes/Client.hpp
class Client {
public:
    // File descriptor
    int getFd() const;
    
    // Identity
    std::string getNickname() const;
    std::string getUsername() const;
    std::string getHostname() const;
    std::string getRealname() const;
    
    // State
    bool isRegistered() const;
    bool hasPassword() const;
    bool hasNickname() const;
    bool hasUsername() const;
    
    // Channel operations
    bool isInChannel(const std::string& channelName) const;
    bool isOperator(const std::string& channelName) const;
    std::vector<std::string> getChannels() const;
    
    // Buffer management (for receiving data)
    void appendToBuffer(const std::string& data);
    std::string& getBuffer();
    void clearBuffer();
    
    // Registration
    void setPassword(const std::string& password);
    void setNickname(const std::string& nickname);
    void setUsername(const std::string& username, const std::string& realname);
    void registerClient();
};
```

**Contract:**
- Client manages its own state (registered, nickname, username, etc.)
- Client maintains a buffer for incomplete messages
- Client tracks which channels it belongs to
- Client does NOT directly send data (use `Server::sendToClient()`)

---

## 5. Channel Interface

**Class: `Channel`**

```cpp
// includes/Channel.hpp
class Channel {
public:
    // Identity
    std::string getName() const;
    
    // Membership
    bool hasClient(Client* client) const;
    void addClient(Client* client);
    void removeClient(Client* client);
    std::vector<Client*> getClients() const;
    size_t getClientCount() const;
    
    // Operators
    bool isOperator(Client* client) const;
    void addOperator(Client* client);
    void removeOperator(Client* client);
    std::vector<Client*> getOperators() const;
    
    // Channel properties
    std::string getTopic() const;
    void setTopic(const std::string& topic, const std::string& setBy);
    std::string getTopicSetter() const;
    time_t getTopicTime() const;
    
    // Modes (basic set for now)
    bool hasMode(char mode) const;
    void setMode(char mode, bool value);
    
    // Broadcasting
    void broadcast(const std::string& message, Client* exclude = NULL);
    void broadcastFrom(const std::string& message, Client* sender, Client* exclude = NULL);
};
```

**Contract:**
- Channel manages its own member list
- Channel tracks operators separately from regular members
- `broadcast()` sends message to all members except `exclude` (if provided)
- Channel does NOT directly send data (messages are formatted by Message Builder, sent via Server)

---

## 6. Message Interface

**Class: `Message` (Message Builder)**

```cpp
// includes/Message.hpp (or MessageBuilder.hpp)
class MessageBuilder {
public:
    // Build numeric replies
    static std::string numericReply(const std::string& numeric,
                                    const std::string& nickname,
                                    const std::string& params,
                                    const std::string& trailing);
    
    // Build command messages
    static std::string command(const std::string& command,
                               const std::string& params,
                               const std::string& trailing = "");
    
    // Build messages with prefix
    static std::string prefixed(const std::string& prefix,
                                const std::string& command,
                                const std::string& params,
                                const std::string& trailing = "");
    
    // Format helpers
    static std::string formatClientPrefix(Client* client);
    static std::string formatChannelPrefix(const std::string& channel);
};
```

**Contract:**
- All messages end with `\r\n` (CRLF)
- Numeric replies follow RFC 2812 format
- Messages are formatted correctly for IRC protocol
- MessageBuilder does NOT send messages (only formats them)

---

## 7. Module Interaction Flow

```
┌─────────┐      ┌──────────┐      ┌──────────┐      ┌─────────┐
│ Poller  │─────>│  Server  │─────>│  Parser  │─────>│ Commands│
│ (poll)  │      │ (sendTo) │      │ (parse)  │      │ (handle)│
└─────────┘      └──────────┘      └──────────┘      └─────────┘
      │                │                 │                  │
      │                │                 │                  │
      ▼                ▼                 ▼                  ▼
┌─────────┐      ┌──────────┐      ┌──────────┐      ┌─────────┐
│ Clients │      │  Server  │      │  Message │      │ Client/ │
│ (fd)    │      │ (I/O)    │      │ Builder  │      │ Channel │
└─────────┘      └──────────┘      └──────────┘      └─────────┘
```

**Flow:**
1. **Poller** detects events via `poll()`
2. **Server** receives raw data, appends to Client buffer
3. **Parser** extracts complete messages from buffer, returns `IRCMessage`
4. **Commands** handle the parsed message, use `Client`/`Channel` interfaces
5. **MessageBuilder** formats responses
6. **Server::sendToClient()** sends formatted messages

---

## 8. Error Handling Conventions

- Socket errors should be handled gracefully (log and disconnect client)
- Invalid messages should be logged but not crash the server
- Use exceptions for unrecoverable errors
- Use return codes/status for recoverable errors
- All error messages to clients should follow IRC numeric reply format

---

## 9. Thread Safety

- **Current design: Single-threaded event loop**
- No mutexes or locks needed (all operations in poll() loop)
- If threading is added later, document thread-safe interfaces

---

## 10. Testing Conventions

- Unit tests should mock dependencies (e.g., mock Server when testing Commands)
- Integration tests should use actual socket connections
- Test files should be in `tests/` directory (not required by subject, but recommended)

---

## Team Meeting Notes

### Meeting 1: Initial Architecture Discussion
**Date:** [To be filled by team]
**Attendees:** Dev A, Dev B, Dev C

**Decisions:**
- ✅ Use `poll()` (not `epoll` or `kqueue`) for cross-platform compatibility
- ✅ Centralize I/O through `Server::sendToClient()`
- ✅ Define `IRCMessage` structure before starting implementation
- ✅ C++98 compliance is mandatory (`-Wall -Wextra -Werror -std=c++98`)
- ✅ Makefile must work on both Linux and Mac
- ✅ Only ONE `poll()` call in entire project (in Poller class)
- ✅ All file descriptors must be non-blocking
- ✅ MessageBuffer handles partial data reconstruction

**Action Items:**
- [x] Dev A: Create Makefile with OS detection
- [x] Dev B: Document module contracts
- [ ] Dev C: Review and approve interfaces

**Key Agreements:**
1. **Poll() Rule:** Only `Poller::poll()` calls the system `poll()`. No other class should call `poll()`, `select()`, `epoll()`, or `kqueue()`.
2. **I/O Rule:** All sending to clients goes through `Server::sendToClient()`. Commands never directly write to sockets.
3. **Message Format:** All IRC messages end with `\r\n` (CRLF). Parser must handle partial messages.
4. **Registration:** Client must complete PASS + NICK + USER before being registered. Commands (except PASS, NICK, USER, PING, PONG, QUIT) are rejected before registration.

---

### Meeting 2: Interface Review & Conventions Approval
**Date:** [To be filled by team]
**Attendees:** Dev A, Dev B, Dev C

**Discussion:**
- Reviewed all interfaces: Client, Channel, Server, Poller, Parser, IRCMessage
- Agreed on IRCMessage structure (prefix, command, params, trailing, raw)
- Confirmed Client interface methods (getters, setters, registration state)
- Confirmed Channel interface methods (membership, operators, modes, broadcast)
- Agreed on error handling: graceful disconnection, IRC numeric replies for errors
- Confirmed single-threaded design (no mutexes needed)

**Decisions:**
- ✅ IRCMessage structure approved (as documented in section 3)
- ✅ Client interface approved (as documented in section 4)
- ✅ Channel interface approved (as documented in section 5)
- ✅ Server::sendToClient() is the ONLY way to send data to clients
- ✅ Poller is the ONLY class that calls poll()
- ✅ MessageBuffer handles CRLF framing and partial data

**Action Items:**
- [x] All team members reviewed and approved conventions
- [x] Interfaces documented in TEAM_CONVENTIONS.md
- [ ] Start implementation following these contracts

**Notes:**
- If any interface needs to change during implementation, team must discuss and update this document
- All changes to interfaces must be documented in Revision History

---

## Revision History

- **v1.0** - Initial version
  - Defined poll() responsibility
  - Defined Server::sendToClient() contract
  - Defined IRCMessage structure
  - Defined Client, Channel, Message interfaces

- **v1.1** - Team conventions finalized
  - Added team meeting notes
  - Confirmed all interfaces approved by team
  - Documented key agreements and decisions
  - Added action items tracking

