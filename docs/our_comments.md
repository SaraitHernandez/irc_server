последовательно комменты в мере хронологии:
**Description:** Implement the read pipeline that receives data from clients, appends it to MessageBuffer, and extracts complete messages. Must handle partial data correctly.

**Tasks:**

1. Implement `MessageBuffer::append(const std::string& data)` in `src/MessageBuffer.cpp`
    - Append data to internal buffer
    - Handle binary data safely
2. Implement `MessageBuffer::extractMessages()` in `src/MessageBuffer.cpp`
    - Find all complete messages (ending with `\r\n`)
    - Extract each complete message
    - Remove extracted messages from buffer
    - Return vector of complete message strings
    - Keep incomplete data in buffer for next append
3. Implement `Server::handleClientInput(int clientFd)` in `src/Server.cpp`
    - Find client by fd
    - Read data using `recv()` (handle EAGAIN/EWOULDBLOCK)
    - Append data to client's MessageBuffer
    - Extract complete messages from buffer
    - For each complete message:
        - Parse using Parser (will be implemented by Dev B)
        - Execute command using CommandRegistry (will be implemented by Dev C)
    - Handle errors (disconnect on error or EOF)
4. Test with partial data:
    
    ```bash
    # Start server
    ./ircserv 6667 password
    
    # In another terminal, send partial data
    echo -n "NICK test" | nc localhost 6667
    # Then send rest
    echo -n "user\r\n" | nc localhost 6667
    
    ```
    

**Acceptance Criteria:**

- ✅ Handles partial messages correctly
- ✅ Accumulates data until `\r\n` found
- ✅ Extracts complete messages
- ✅ Keeps incomplete data in buffer
- ✅ Handles multiple messages in one recv()
- ✅ Handles EAGAIN/EWOULDBLOCK correctly
- ✅ Disconnects client on error or EOF

**Files to modify:**

- `src/MessageBuffer.cpp`
- `include/irc/MessageBuffer.hpp` (uncomment and implement)
- `src/Server.cpp` (implement `handleClientInput()`)

**Key Implementation Details:**

```cpp
// In MessageBuffer::extractMessages()
std::vector<std::string> MessageBuffer::extractMessages(){
    std::vector<std::string> messages;
    size_t pos = 0;

    while ((pos = buffer_.find("\r\n", pos)) != std::string::npos) {
        std::string msg = buffer_.substr(0, pos + 2);
        messages.push_back(msg);
        buffer_.erase(0, pos + 2);
        pos = 0;
    }

    return messages;
}

// In Server::handleClientInput()
void Server::handleClientInput(int clientFd){
    Client* client = getClient(clientFd);
    if (!client) return;

    char buffer[4096];
    ssize_t n = recv(clientFd, buffer, sizeof(buffer) - 1, 0);

    if (n <= 0) {
        if (n == 0 || (errno != EAGAIN && errno != EWOULDBLOCK)) {
            disconnectClient(clientFd);
        }
        return;
    }

    buffer[n] = '\0';
    client->getMessageBuffer().append(std::string(buffer, n));

    std::vector<std::string> messages = client->getMessageBuffer().extractMessages();
    for (size_t i = 0; i < messages.size(); ++i) {
        // Parse and execute (will be implemented by Dev B and Dev C)
    }
}

```

***3 ways of manage the buffer:***

[https://www.notion.so/ft_irc-2cbbb1f38a4b802298b2f409599c0f74?source=copy_link](https://www.notion.so/ft_irc-2cbbb1f38a4b802298b2f409599c0f74?pvs=21)

```
## Proposal: Separate BufferManager Component

### Current Approach (Variant 1): Buffer inside Client

```
┌─────────────────────────────────────┐
│ Logic Layer (Dev C)                 │
│  ┌─────────────────────────────┐    │
│  │ Client                      │    │
│  │  - receiveBuffer_  ← HERE   │    │
│  │  + appendToBuffer()         │    │
│  │  + extractMessages()        │    │
│  └─────────────────────────────┘    │
└──────────────▲──────────────────────┘
               │ calls Client methods
┌──────────────┴──────────────────────┐
│ Network Layer (Dev A)               │
│  Server::handleClientInput():       │
│    client->appendToBuffer(data)     │
│    msgs = client->extractMessages() │
└─────────────────────────────────────┘
```

**Problems:**

-Client has TWO responsibilities: business logic + buffering
-Network depends on Client's buffer methods
-Hard to test buffering separately
-Violates Single Responsibility Principle

### Proposed Approach (Variant 3): Separate BufferManager

```
┌─────────────────────────────────────┐
│ Parser Layer (Dev B)                │
│  ┌─────────────────────────────┐    │
│  │ BufferManager               │    │
│  │  - buffers_[fd]  ← HERE     │    │
│  │  + feed(fd, data)           │    │
│  │  + extractMessages(fd)      │    │
│  └─────────────────────────────┘    │
└──────────────▲──────────────────────┘
               │ uses BufferManager
┌──────────────┴──────────────────────┐
│ Network Layer (Dev A)               │
│  Server:                            │
│    - bufferManager_                 │
│    handleClientInput():             │
│      bufferManager_.feed(fd, data)  │
│      msgs = bufferManager_          │
│             .extractMessages(fd)    │
└─────────────────────────────────────┘

┌─────────────────────────────────────┐
│ Logic Layer (Dev C)                 │
│  Client - NO buffer knowledge!      │
│    - nickname_, username_...        │
│    (NO receiveBuffer_)              │
└─────────────────────────────────────┘
```

**Benefits:**

-Clear separation of concerns
-Client only handles business logic
-BufferManager only handles CRLF framing
-Easy to test independently
-Follows Radio France architecture

## Key Arguments

### 1. **Architecture Alignment to production standarts**

**Streaming platform:**

```
SRT Listener (transport) → buffer() → Liquidsoap (processing)
     ↓                        ↓              ↓
  Network              BufferManager      Parser
```

Our IRC could follow the same pattern.

### 3. **Testing Benefits**

**Variant 1** (Buffer in Client):

```cpp
// Need full Client object to test buffering
Client client(fd);
client.setNickname("test");
client.appendToBuffer("NICK\r\n");  // Mixed concerns!
```

**Variant 3** (Separate BufferManager):

```cpp
// Test buffering independently
BufferManager bm;
bm.feed(1, "NICK te");
bm.feed(1, "st\r\n");
assert(bm.extractMessages(1).size() == 1);

// Test Client independently
Client client(1);
client.setNickname("test");
assert(client.getNickname() == "test");
```

### 4. **Dependency Flow**

**Variant 1:**

```
Network → Client (tight coupling)
```

**Variant 3:**

```
Network → BufferManager (loose coupling)
Client ← (no dependency on buffering)
```

## Code Comparison

### Variant 1: Buffer in Client

```cpp
// Client.hpp (Logic Layer)
class Client {
private:
    std::string receiveBuffer_;  // ← mixing concerns
public:
    void appendToBuffer(const std::string& data);
    std::vector<std::string> extractMessages();
};

// Server.cpp (Network Layer)
void Server::handleClientInput(int fd) {
    recv(fd, buf, 4096, 0);
    client->appendToBuffer(buf);  // ← depends on Client
    msgs = client->extractMessages();
}
```

### Variant 3: Separate BufferManager

```cpp
// BufferManager.hpp (Parser Layer)
class BufferManager {
private:
    std::map<int, std::string> buffers_;
public:
    void feed(int fd, const std::string& chunk);
    std::vector<std::string> extractMessages(int fd);
    void clear(int fd);
};

// Server.cpp (Network Layer)
class Server {
    BufferManager bufferManager_;  // ← clean dependency

    void handleClientInput(int fd) {
        recv(fd, buf, 4096, 0);
        bufferManager_.feed(fd, buf);
        msgs = bufferManager_.extractMessages(fd);
    }
};

// Client.hpp (Logic Layer)
class Client {
    // NO receiveBuffer_!
    std::string nickname_;  // ← only business logic
};
```

## Implementation Plan

### For Dev B (Parser Layer):

**Create:** `include/irc/BufferManager.hpp`, `src/BufferManager.cpp`

```cpp
class BufferManager {
private:
    std::map<int, std::string> buffers_;

public:
    void feed(int fd, const std::string& chunk) {
        buffers_[fd].append(chunk);
    }

    std::vector<std::string> extractMessages(int fd) {
        std::vector<std::string> messages;
        std::string& buf = buffers_[fd];
        size_t pos;

        while ((pos = buf.find("\r\n")) != std::string::npos) {
            messages.push_back(buf.substr(0, pos));
            buf.erase(0, pos + 2);
        }
        return messages;
    }

    void clear(int fd) {
        buffers_.erase(fd);
    }
};
```

### For Dev A (Network Layer):

**Modify:** `src/Server.cpp`

```cpp
// Add member
BufferManager bufferManager_;

// In handleClientInput():
void Server::handleClientInput(int clientFd) {
    char buffer[^4096];
    ssize_t n = recv(clientFd, buffer, sizeof(buffer), 0);

    if (n <= 0) {
        disconnectClient(clientFd);
        return;
    }

    // Feed to BufferManager
    bufferManager_.feed(clientFd, std::string(buffer, n));

    // Extract complete messages
    std::vector<std::string> messages =
        bufferManager_.extractMessages(clientFd);

    // Process messages
    Client* client = clients_[clientFd];
    for (size_t i = 0; i < messages.size(); ++i) {
        IRCMessage msg = parser_.parse(messages[i]);
        registry_.execute(this, client, msg);
    }
}

// In disconnectClient():
void Server::disconnectClient(int clientFd) {
    bufferManager_.clear(clientFd);  // Clean up buffer
    // ... rest of cleanup
}
```

### For Dev C (Logic Layer):

**Modify:** `include/irc/Client.hpp` - **Remove** buffer-related code:

```cpp
class Client {
private:
    int fd_;
    std::string nickname_;
    // NO receiveBuffer_!

public:
    // NO appendToBuffer()!
    // NO extractMessages()!

    // Only business logic
    void setNickname(const std::string& nick);
    bool isRegistered() const;
};
```

## Summary

**Recommendation:** Use **Variant 3** (Separate BufferManager)

**Reasons:**

1.✅ Matches architecture industry standard
2.✅ Clear separation of concerns (SRP)
3.✅ Easy to test independently
4.✅ Loose coupling between layers
5.✅ BufferManager naturally belongs to Parser Layer (Dev B)

**File Structure:**

```
include/irc/
  ├── Server.hpp         (Network - Dev A)
  ├── BufferManager.hpp  (Parser - Dev B)  ← NEW
  ├── Parser.hpp         (Parser - Dev B)
  └── Client.hpp         (Logic - Dev C)

src/
  ├── Server.cpp
  ├── BufferManager.cpp  ← NEW (Dev B implements)
  ├── Parser.cpp
  └── Client.cpp
```

**Owner:** Dev B (Parser Layer) implements BufferManager
**Users:** Dev A (Network) uses it, Dev C (Logic) doesn't know about it

```
ответ B Parser:
Code - Plain Text
## Proposal: Separate BufferManager Component

###…
what do you think about buffer that stays at parser side as separate unit to easy test and clean logic? @Sarait Hernandez @Artur 
A
Artur (B Parser here and further)
Jan 4 (edited)
Hi,
For me it makes sense and is OK. While I was doing my knowledge investigation part I have came to the same conclusion as Variant 3.
One thing I will need from you Alex is to specify the feed method - what will be produced by Network module, will it be a simple string just to be appended or will it be more complex?
Also, I saw the repository already contains the MessageBuffer.hpp file, so it can be used to implement it as a separate class
*******
далее я писал (A Net)
Hi @Sarait Hernández @Alex 
Here you can find the results of my investigation and knowledge gathering. Below this line there is summarized flow and connection points between parser and other modules. 
Above this comment in this page you can find detailed results of my study covering every point I have found important. 
# **IRC Parser & Protocol
Plan for study**

### **1. IRC Protocol (RFC 1459 and RFC 2812)**

- Message format: `[:<prefix>] <command> [<params>] [:<trailing>]`
- What is the prefix (message origin, optional)
- Commands and their parameters
- Numeric replies (001, 433, 462, etc.)
- CRLF (`\r\n`) as message delimiter
- Connection handshake: CAP, PASS, NICK, USER

**Resources:**

- RFC 1459 (sections 2.3 and 2.4) - **MANDATORY**
- RFC 2812 (sections 2.3 - Message format)
- Connect to a real IRC server (irc.libera.chat) with irssi and observe traffic
- Use `tcpdump` or `wireshark` to see raw messages

**Practice:**

- Connect to real IRC server and manually send commands
- Observe numeric replies and their format

### **2. String Parsing in C++98**

- `std::string` and its methods (find, substr, erase, compare)
- Manual tokenization (no `split()` in C++98)
- Handling CRLF delimiters (`\r\n`)
- Parsing with spaces and colons
- Extracting trailing parameter (text after `:`)

**Resources:**

- C++98 string reference
- Practice parsing: `"NICK alice\r\n"`, `"PRIVMSG #test :hello world\r\n"`

**Practice:**

- Write parser for simple IRC messages
- Handle edge cases (multiple spaces, no trailing, etc.)

### **3. Data Buffering & Partial Messages**

- Why data arrives fragmented (TCP is a stream protocol)
- How to accumulate data until a complete message
- The subject test with `nc` and Ctrl+D (sending in parts)
- Buffer management strategies
- When to extract complete messages (CRLF found)

**Resources:**

- Beej's Guide Chapter 6.3
- Subject example: `nc -C 127.0.0.1 6667` then send `com^Dman^Dd`

**Practice:**

- Write buffer class that accumulates until `\r\n`
- Test with fragmented input

### **4. IRC Response Format & Numeric Replies**

- Structure of numeric replies: `:server 001 nick :Welcome message\r\n`
- Standard error messages format
- How a real server responds (observe with irssi)
- Building correct IRC messages to send
- Common numeric replies needed:
- 001-004: Welcome sequence
- 331-332: Topic replies
- 353, 366: Names list
- 401, 403: Not found errors
- 431-433: Nickname errors
- 461-464: Registration errors
- 473, 475: Channel errors
- 482: Operator privileges

**Resources:**

- RFC 2812 section 5 (Replies)
- Connect to real server and capture all numeric replies

**Practice:**

- Write helper functions to build numeric replies
- Test format with irssi

### **5. Command Registry Pattern**

- How to route parsed commands to handlers
- Map/function pointer pattern in C++98
- Unknown command handling

**Checklist for Dev B:**

- [x]  Can parse IRC message format correctly
- [x]  Understands CRLF framing
- [x]  Can handle partial messages
- [x]  Knows common numeric replies
- [x]  Can build correctly formatted IRC responses

---

# Study results

You can find TL;DR section at the end of this note.

## 1. IRC Protocol Foundation (RFC 1459/2812)

### Message Anatomy

Every message is a single line of text ending in `\\r\\n`.

| **Component** | **Description** | **Example** |
| --- | --- | --- |
| **Prefix** | (Optional) Starts with `:`. Identifies the sender. | `:Angel!batman@island.org` |
| **Command** | A string (e.g., `KICK`) or 3-digit numeric (e.g., `404`). | `PRIVMSG` |
| **Params** | Space-separated arguments (Max 15). | `#42Warsaw` |
| **Trailing** | The last param; starts with `:` to allow spaces. | `:Hello world!` |
- The **Prefix** is rarely sent by a client to a server, but it is mandatory for your server to include it when sending messages to clients. It identifies the source (e.g., `:YourServerName` or `:Nickname!User@Host`).
- The **Trailing** parameter is the only one allowed to contain spaces. It is identified by a leading colon (`:`).

### The Handshake Sequence

Before a user can execute commands like JOIN or PRIVMSG, they must register. The typical flow is:

1. PASS: PASS secret_password — Validates the server password.
2. NICK: NICK user123 — Sets the display name.
3. USER: USER guest 0 * :Real Name — Sets username and real name.
4. Server Reply: Once valid, the server sends Numeric 001 (RPL_WELCOME).

- **Connection Handshake:**
    - **PASS:** Must be checked first. If the password is wrong or missing, the connection should be rejected.
    - **NICK/USER:** These populate the client's identity. A user is not considered "registered" until they have successfully sent PASS, NICK, and USER.
    - **CAP:** Modern clients send this to ask for "Capabilities" (like multi-prefix). Since you are doing a basic server, you can usually ignore this or send a "Numeric 421" (Unknown Command).

### Important Numeric Replies

Numerics are how the server reports success or error.

- `001`: Welcome to the network (Successful registration).
- `433`: Nickname is already in use (Error).
- `461`: Need more parameters (Error).
- `464`: Password incorrect (Error).

**CRLF (`\\r\\n`):**

- The protocol specifies `\\r\\n` as the delimiter. However, many modern clients (and `nc`) might only send `\\n`. Your parser should ideally handle both to be robust.

### Testing Tools

- Netcat (nc): Best for testing raw, broken, or partial packets. Use nc -C [host] [port] to ensure \r\n is sent.
- Irssi: A standard CLI client to test "real-world" behavior.
- **Tcpdump:** `sudo tcpdump -i lo0 -A port 6667` to see raw traffic on localhost.

---

### **2. String Parsing in C++98**

### Technical Expansion

- **`std::string` Methods:**
    - **`find()`**: primary tool for locating spaces (`' '`) or the CRLF (`\\r\\n`) delimiter.
    - **`substr()`**: used to extract the command and parameters once their positions are found.
    - **`erase()`**: helpful for cleaning up the buffer once a command has been processed.
- **Manual Tokenization:**
    - Since C++98 lacks a split function, you must loop through the string. A common pitfall is multiple spaces (e.g., `COMMAND param1`). A simple split might create empty strings; a robust parser skips consecutive delimiters.
- **The "Trailing" Logic:**
    - The character `:` is a special marker in IRC. If it appears at the start of a parameter, that parameter includes **everything** until the end of the line, including spaces. You must check for the prefix `:` at the start of a message and the trailing `:` for the last parameter.
- **Handling CRLF (`\\r\\n`):**
    - The subject specifies that messages must be aggregated and rebuilt. You cannot assume a message is complete until you find the `\\r\\n` sequence.

### Core `std::string` Toolkit

In C++98, we rely on these to dissect messages:

- `s.find(str, pos)`: Returns `std::string::npos` if not found. Essential for finding the first space.
- `s.substr(start, len)`: Extracts a piece of the string. **Warning:** `len` is the number of characters, not the end index.
- `s.erase(pos, len)`: Removes characters from the buffer after they are parsed.
- `s.at(index)`: Safer than `[]` because it throws an exception if out of bounds.

### Parsing Strategy: Step-by-Step

To parse a message like `:Guest1 PRIVMSG #chan :Hello world\\r\\n`:

1. **Check for Prefix:** If `msg[0] == ':'`, find the first space. Everything between `msg[1]` and that space is your **prefix**.
2. **Extract Command:** Find the next non-space character. Read until the next space. This is your **command** (e.g., `PRIVMSG`).
3. **Loop for Params:**
    - If the next character is `:`, the rest of the string (minus the `\\r\\n`) is the **trailing parameter**. Stop looping.
    - Otherwise, read until the next space. This is a **regular parameter**.

4. **Stop at CRLF:** Always ensure you stop before the `\\r\\n`.

### Edge Cases to Handle

- **Multiple Spaces:** `KICK #channel user` should be treated the same as `KICK #channel user`.
- **No Trailing:** `JOIN #42` is valid; not every command needs a colon.
- **Empty Messages:** If a user just hits "Enter," your parser should ignore the empty string and not crash.
- **Leading/Trailing Spaces:** Be sure to `trim` spaces if your client sends them unexpectedly.

### Error handling

It is highly likely that a client will send incorrect messages, either due to human error (manual `nc` entry) or a buggy client implementation. Your server must be robust enough to handle these without crashing.

### Types of Possible Errors

When a message like `KICCK` arrives, your parser will encounter several types of failures:

- **Unknown Commands:** The command does not exist in the IRC protocol (e.g., `KICCK`, `LOL`, `HLEP`).
- **Malformed Syntax:** The message violates the protocol structure, such as missing spaces, starting with multiple colons, or exceeding the 512-character limit.
- **Parameter Mismatch:** The command is valid (e.g., `KICK`), but it has too few parameters (e.g., `KICK #channel` without a username).
- **Incomplete Data:** The message is cut off before the CRLF (`\\r\\n`) terminator.

### Required Error Handling

According to the subject, your error handling must follow two main rules:

- **No Crashing:** Your program must not crash under any circumstances, even if it runs out of memory or receives total gibberish. If it quits unexpectedly, the grade is 0.
- **Protocol-Appropriate Responses:** When a user sends an error, you shouldn't just "print" an error to your server console. You must send a **Numeric Reply** back to the client so they know what went wrong.

**Common Numeric Replies for Parser Errors:**

- `421 (ERR_UNKNOWNCOMMAND)`: Sent when the command (like `KICCK`) is unrecognized.
- `461 (ERR_NEEDMOREPARAMS)`: Sent when a command lacks the required number of arguments.
- `417 (ERR_INPUTTOOLONG)`: (Optional but good) Sent if a single message exceeds the protocol size limit.

### Logic for the Parser

Instead of hardcoding every error, your parser should be a "filter."

**Goal:** The server must never crash and must always inform the user of mistakes using IRC Numerics.

1. The "Unknown Command" Rule

If a user sends `KICCK #chan user`:

- **Parser action:** Identify the string `KICCK`.
- **Logic action:** Check the Command Registry. If not found, return: `:<servername421 <nicknameKICCK :Unknown command`

2. The "Missing Params" Rule

If a user sends `KICK #channel` (missing the target user):

- **Parser action:** Split into `KICK` and `#channel`.
- **Logic action:** Check the required parameter count for `KICK` (which is 2).
- **Server response:** `461 ERR_NEEDMOREPARAMS`.

3. Buffer Overflows & Length

- **Strict Limit:** The IRC protocol limits messages to 512 characters. * **Action:** If a client sends a message longer than this without a `\\n`, you must decide whether to truncate it or drop it to prevent memory exhaustion.

4. Empty or Whitespace-Only Lines

- If a client sends `\\r\\n` or `\\r\\n`, the parser should silently ignore it. Do not attempt to process an empty command.

---

### **3. Data Buffering & Partial Messages**

### The "Stream" Problem

TCP does **not** guarantee that one message arrives in one piece. We might receive `NICK t`, then `om\\r`, then `\\n`. Our code must handle this by "remembering" the fragments.

### Implementation Logic

To handle this according to the subject requirements:

1. **Storage:** Each `Client` object must have a `std::string _buffer;` or a separate sub-class that will contain it and related methods.
2. **Reception:** When `poll()` signals `POLLIN`, use `recv()` and append the result to `_buffer`.
3. **Validation:**
    - Search for `\\r\\n` in the `_buffer`.
    - **If found:** Extract the substring from the start to `\\r\\n`. This is your "Command."
    - **Cleanup:** Remove that extracted substring from the `_buffer`.
    - **Loop:** Check if there is another `\\r\\n` (the client might have sent multiple commands at once).
    - **If NOT found:** Do nothing. Leave the data in the buffer and wait for the next `poll()` event.

### Common Pitfalls

- **Forgetting the Buffer:** Never use a global or temporary buffer that clears after the function ends. Data will be lost.
- **Blocking:** Ensure `fcntl(fd, F_SETFL, O_NONBLOCK)` is set so `recv()` doesn't hang your whole server.
- **The "nc" Test:** You must be able to handle a command sent byte-by-byte via `nc -C` and Ctrl+D.

---

### **4. IRC Response Format & Numeric Replies**

### Structure of Numeric Replies

Most communication from the server to the client uses Numeric Replies. These are 3-digit codes followed by parameters. The standard format is:
`:<servername> <numeric_code> <target_nick> <params> :<text_message>\\r\\n`

- **Prefix (`:servername`)**: Tells the client which server is sending the message.
- **Numeric (`001`)**: A 3-digit code identifying the type of reply.
- **Target (`nick`)**: The nickname of the user receiving the message.
- **Trailing (`:text`)**: Often contains the human-readable part of the message, preceded by a colon.

### Standard Error Messages

When a command fails (e.g., trying to join a password-protected channel without a key), the server must return a specific error numeric.

- Format: `:<servername> <error_numeric> <nick> <offending_param> :<error_reason>\\r\\n`
- **Example (461 - ERR_NEEDMOREPARAMS):** `:ft_irc.42.pl 461 user1 KICK :Not enough parameters\\r\\n`.

### The Welcome Sequence (Registration)

A client is not fully "logged in" until they receive the welcome sequence. These are the first things your server must send after a successful `NICK` and `USER` command:

- **001 (RPL_WELCOME)**: The "Welcome to the Network" message.
- **002-004**: System info (Your host, server version, and available user/channel modes).

### Observing Real Servers

Using a client like **irssi** or **nc** to connect to an official server is the best way to see these in action. By using `irssi` with the `/raw log on` command, you can see the raw strings coming from the server, which includes all the numerics and colons usually hidden by the UI.

### Format Cheat Sheet

Every message sent from your server **must** end with `\\r\\n`.

| **Type** | **Format Template** |
| --- | --- |
| **Numeric Reply** | `:<servername> <3-digit-code> <nick> <params> :<text>\\r\\n` |
| **Command Response** | `:<user_prefix> <COMMAND> <params>\\r\\n` |

### Required Numerics for ft_irc

Based on the subject requirements, we must implement at least these:
**Connection & Registration:**

- `001` (RPL_WELCOME): Must be sent after successful registration.
- `461` (ERR_NEEDMOREPARAMS): Sent if a command (like `KICK`) is missing arguments.
- `464` (ERR_PASSWDMISMATCH): Sent if the password provided doesn't match the server password.

**Channel Operations (KICK, INVITE, TOPIC, MODE):**

- `331` / `332` (RPL_NOTOPIC / RPL_TOPIC): For the `TOPIC` command.
- `353` (RPL_NAMREPLY): List of users in a channel (sent after `JOIN`).
- `403` (ERR_NOSUCHCHANNEL): Sent if a user tries to `JOIN` or `KICK` from a non-existent channel.
- `482` (ERR_CHANOPRIVSNEEDED): Sent if a regular user tries to use an operator-only command.

**Nickname Errors:**

- `431` (ERR_NONICKNAMEGIVEN): No nickname supplied.
- `433` (ERR_NICKNAMEINUSE): The nickname is already taken by another client.

### 💡 Best Practice: The "Reply Builder"

Don't hardcode strings in your logic. Create a helper function or static class:
C++

```
// Example Concept
std::string msg = Reply::make(RPL_WELCOME, user, "Welcome to our irc server!");
// Output: ":ft_irc 001 tom :Welcome to our irc server!\\r\\n"

```

---

### **5. Command Registry Pattern**

### How to route parsed commands to handlers

Once your parser identifies a command (like `KICK` or `JOIN`), the server needs to find the specific block of code responsible for that action. In a professional C++98 architecture, you should decouple the **parsing** (identifying the string "KICK") from the **execution** (the logic that removes a user from a channel).

### Map/Function Pointer Pattern in C++98

The most efficient way to do this in C++98 is using a `std::map` that links a `std::string` (the command name) to a **function pointer** or a **Command Object**

- **The Signature:** All command handlers should have the same signature, usually taking a reference to the `Client` who sent it and a `vector` of arguments.
- **The Registry:** A `std::map<std::string, void (*)(Client&, const std::vector<std::string>&)>`.
- **The Lookup:** Instead of 20 `if` statements, you simply do `_commands.find("KICK")`. If it exists, you execute the function pointer.

### Unknown Command Handling

If the `find()` operation fails, the command is "Unknown." According to the IRC protocol, the server shouldn't just stay silent

- **Error 421 (ERR_UNKNOWNCOMMAND):** The server must send a numeric reply back to the client: `:<servername> 421 <nick> <command> :Unknown command\\r\\n`.

### The Goal

To avoid "Spaghetti Code" by creating a central dispatcher that routes strings to their respective logic functions.

### C++98 Implementation (Map of Function Pointers)

Since we are restricted to **C++98**, we use `std::map` and standard function pointers.

**1. Define the Handler Type:**

```
typedef void (*CommandHandler)(Client& sender, const std::vector<std::string>& args);
```

**2. The Command Map:** Store this inside your `Server` class:

```
std::map<std::string, CommandHandler> _commandRegistry;
```

**3. Initialization:** Populate the map when the server starts:

```
_commandRegistry["NICK"] = &handleNick;
_commandRegistry["JOIN"] = &handleJoin;
_commandRegistry["KICK"] = &handleKick; // Operator specific [cite: 116]
```

### Execution Flow

1. **Parse:** Extract the command name (e.g., `TOPIC`) from the incoming buffer.
2. **Search:** `it = _commandRegistry.find(cmdName);`
3. **Execute:** 
    - If `it != _commandRegistry.end()`, call `it->second(sender, args);`.
    - Else, send `ERR_UNKNOWNCOMMAND (421)`.

### Benefits for our Team

- **Modularity:** The "Network" dev just hands a string to the "Parser" dev, who then hands a function pointer to the "Logic" dev.
- **Scalability:** Adding a new command like `INVITE` or `MODE` only requires adding one line to the registry and writing the handler function.
- **Clean Code:** Keeps the `Server.cpp` file clean and organized, which is a requirement of the project.

---

# TL;DR

## The Parser Flow: From Bytes to Action

1. **Input: The Raw Buffer (Entry)**
    - **Receiver:** The Parser receives a reference to a `Client` object the data recently received from a `recv()` call.
    - **Context:** The "Network Dev" has already placed this data into the client-specific buffer to handle fragmentation.
2. **Step 1: Message Aggregation & Extraction**
    - **The Action:** The parser searches for the `\\r\\n` delimiter (CRLF).
    - **Internal Work:** It "slices" the first complete message out of the buffer. If the buffer contains `KICK #chan user\\r\\nNICK newname\\r\\n`, the parser must extract them one by one.
    - **Buffer Cleanup:** The processed portion is removed from the client's internal buffer so it isn't processed twice.
3. **Step 2: Syntax Decomposition**
    - **The Action:** The raw string (e.g., `KICK #channel user :Reason here`) is split into components.
    - **Internal Work:**
        - Identify the **Command** (the first word, converted to uppercase: `KICK`).
        - Identify the **Parameters** (space-separated: `#channel`, `user`).
        - Identify the **Trailing** (anything after the first colon `:`: `Reason here`).
4. **Step 3: Validation & The Registry Lookup**
    - **The Action:** The parser checks if the command exists in the **Command Registry**.
    - **Error Handling:** If the command is not found, the parser immediately generates a **Numeric Reply 421 (ERR_UNKNOWNCOMMAND)** to be sent back to the client.
5. **Result: The "Command Request" Object**
    - **The Output:** The parser produces a clean structure (often a `Command` object or a struct) containing the sender's reference, the command name, and a `std::vector<std::string>` of arguments.
    - **Where it finishes:** The parser's job ends once it has identified **who** is calling **what** and with **which parameters**.
6. **Handover: Execution**
    - **Who executes?** The **Logic Dev's** handlers.
    - **The Mechanism:** The Parser uses the **Command Registry** (the Map of Function Pointers) to trigger the specific function (e.g., `handle_kick()`) developed by Logic dev.
    - **Execution Data:** The Logic handler receives the validated parameters and performs the actual server-side changes (removing the user from the channel, updating the channel list, etc.).

### Networking ↔ Parser

Currently I’m in favor of Variant 3 from this note:
[https://www.notion.so/ft_irc-2cbbb1f38a4b802298b2f409599c0f74?source=copy_link#2cfbb1f38a4b80f092fdd5a205299fc7](https://www.notion.so/2cbbb1f38a4b802298b2f409599c0f74?pvs=21)

assuming:

- MessageBuffer will be a separate class; can be managed by Parser
- Networking will specify what it needs for the feeding method there to fill the buffer with incoming data
- Parser will create extract and clear methods (and further methods if necessary)
- It will be added as a subclass to Client class by Logic

### Parser ↔ Logic

In the architecture of your **ft_irc** server, the **Command Registry** and **Numeric Responses** represent the two sides of a conversation: the Registry is how the server "listens" and routes requests, while the Numeric Responses are how the server "speaks" back to the client.

### Connection & Differences

| **Feature** | **Command Registry** | **Numeric Responses** |
| --- | --- | --- |
| **Direction** | **Inbound** (Client → Server) | **Outbound** (Server → Client) |
| **Purpose** | To route a recognized command (e.g., `KICK`) to its logic handler. | To inform the client of success, status, or errors. |
| **Format** | Plain text string (e.g., "JOIN"). | 3-digit code + parameters (e.g., "461"). |
| **Trigger** | Triggered by the **Parser** after finding `\\r\\n`. | Triggered by **Logic Handlers** or the **Parser** during errors. |

### How They Relate (The "Chain of Custody")

They are connected through a standard execution flow. One often leads to the other:

1. **The Parser** checks the **Command Registry** to see if the command exists.
2. **Scenario A (Failure):** If the command is *not* in the Registry, the **Parser** immediately triggers a **Numeric Response** (e.g., `421 ERR_UNKNOWNCOMMAND`).
3. **Scenario B (Success):** If the command *is* in the Registry, the **Parser** hands control to the **Logic Handler** (e.g., `handle_kick`).
4. **The Logic Handler** then performs checks (e.g., "Is this user an operator?").
5. **The Result:** The Logic Handler triggers various **Numeric Responses** (e.g., `482 ERR_CHANOPRIVSNEEDED` if they aren't an operator, or `332 RPL_TOPIC` on success).

### 📥 Inbound: The Parser & Registry

- **The Parser** is the only module that talks to the **Command Registry**.
- It identifies the command from the stream and "plugs" it into the map.
- **Trigger:** Arrival of a complete message ending in `\\r\\n`.

### 📤 Outbound: The Logic & Numerics

- **The Logic Handlers** are the primary users of **Numeric Responses**.
- **The Parser** only sends a Numeric if the command is unrecognizable or the syntax is broken.
- **The Server** sends the welcome sequence (`001-004`) only after the Parser confirms both `NICK` and `USER` are correctly received.
- **Constraint:** All responses must be sent through the non-blocking `send()` function via the `poll()` (or equivalent) loop to avoid hanging the server.

### Summary for your Team

- **You (Parser):** You own the **Registry**. You use it to find the right logic. If you can't find it, you send a **Numeric Error**.
- **Logic Dev:** They own the **Handlers**. They use the **Numerics** to tell the client if their request (KICK, JOIN, etc.) worked or failed.
- **Network Dev:** They provide the "transport" (the `send()` calls) to make sure those **Numerics** actually reach the client without blocking the server.

### **Parsed Command Structure Format**

The format as provided in the link below is in my opinion sufficient and enough. The contract part also is OK for me.

https://github.com/SaraitHernandez/irc_server/blob/main/docs/TEAM_CONVENTIONS.md#3-parsed-command-structure-format

### Message Interface

Messages structures and contract assumptions are good for me. 

https://github.com/SaraitHernandez/irc_server/blob/main/docs/TEAM_CONVENTIONS.md#6-message-interface

## Git branching convention

I propose:
[network/parser/logic]/task_number/feature_description

e.g.

parser/09/create_message_buffer

*******
**Description:** Implement the read pipeline that receives data from clients, appends it to MessageBuffer, and extracts complete messages. Must handle partial data correctly.

**Tasks:**

1. Implement `MessageBuffer::append(const std::string& data)` in `src/MessageBuffer.cpp`
    - Append data to internal buffer
    - Handle binary data safely
2. Implement `MessageBuffer::extractMessages()` in `src/MessageBuffer.cpp`
    - Find all complete messages (ending with `\r\n`)
    - Extract each complete message
    - Remove extracted messages from buffer
    - Return vector of complete message strings
    - Keep incomplete data in buffer for next append
3. Implement `Server::handleClientInput(int clientFd)` in `src/Server.cpp`
    - Find client by fd
    - Read data using `recv()` (handle EAGAIN/EWOULDBLOCK)
    - Append data to client's MessageBuffer
    - Extract complete messages from buffer
    - For each complete message:
        - Parse using Parser (will be implemented by Dev B)
        - Execute command using CommandRegistry (will be implemented by Dev C)
    - Handle errors (disconnect on error or EOF)
4. Test with partial data:
    
    ```bash
    # Start server
    ./ircserv 6667 password
    
    # In another terminal, send partial data
    echo -n "NICK test" | nc localhost 6667
    # Then send rest
    echo -n "user\r\n" | nc localhost 6667
    
    ```
    

**Acceptance Criteria:**

- ✅ Handles partial messages correctly
- ✅ Accumulates data until `\r\n` found
- ✅ Extracts complete messages
- ✅ Keeps incomplete data in buffer
- ✅ Handles multiple messages in one recv()
- ✅ Handles EAGAIN/EWOULDBLOCK correctly
- ✅ Disconnects client on error or EOF

**Files to modify:**

- `src/MessageBuffer.cpp`
- `include/irc/MessageBuffer.hpp` (uncomment and implement)
- `src/Server.cpp` (implement `handleClientInput()`)

**Key Implementation Details:**

```cpp
// In MessageBuffer::extractMessages()
std::vector<std::string> MessageBuffer::extractMessages(){
    std::vector<std::string> messages;
    size_t pos = 0;

    while ((pos = buffer_.find("\r\n", pos)) != std::string::npos) {
        std::string msg = buffer_.substr(0, pos + 2);
        messages.push_back(msg);
        buffer_.erase(0, pos + 2);
        pos = 0;
    }

    return messages;
}

// In Server::handleClientInput()
void Server::handleClientInput(int clientFd){
    Client* client = getClient(clientFd);
    if (!client) return;

    char buffer[4096];
    ssize_t n = recv(clientFd, buffer, sizeof(buffer) - 1, 0);

    if (n <= 0) {
        if (n == 0 || (errno != EAGAIN && errno != EWOULDBLOCK)) {
            disconnectClient(clientFd);
        }
        return;
    }

    buffer[n] = '\0';
    client->getMessageBuffer().append(std::string(buffer, n));

    std::vector<std::string> messages = client->getMessageBuffer().extractMessages();
    for (size_t i = 0; i < messages.size(); ++i) {
        // Parse and execute (will be implemented by Dev B and Dev C)
    }
}

```

***3 ways of manage the buffer:***

[https://www.notion.so/ft_irc-2cbbb1f38a4b802298b2f409599c0f74?source=copy_link](https://www.notion.so/2cbbb1f38a4b802298b2f409599c0f74?pvs=21)

```
## Proposal: Separate BufferManager Component

### Current Approach (Variant 1): Buffer inside Client

```
┌─────────────────────────────────────┐
│ Logic Layer (Dev C)                 │
│  ┌─────────────────────────────┐    │
│  │ Client                      │    │
│  │  - receiveBuffer_  ← HERE   │    │
│  │  + appendToBuffer()         │    │
│  │  + extractMessages()        │    │
│  └─────────────────────────────┘    │
└──────────────▲──────────────────────┘
               │ calls Client methods
┌──────────────┴──────────────────────┐
│ Network Layer (Dev A)               │
│  Server::handleClientInput():       │
│    client->appendToBuffer(data)     │
│    msgs = client->extractMessages() │
└─────────────────────────────────────┘
```

**Problems:**

-Client has TWO responsibilities: business logic + buffering
-Network depends on Client's buffer methods
-Hard to test buffering separately
-Violates Single Responsibility Principle

### Proposed Approach (Variant 3): Separate BufferManager

```
┌─────────────────────────────────────┐
│ Parser Layer (Dev B)                │
│  ┌─────────────────────────────┐    │
│  │ BufferManager               │    │
│  │  - buffers_[fd]  ← HERE     │    │
│  │  + feed(fd, data)           │    │
│  │  + extractMessages(fd)      │    │
│  └─────────────────────────────┘    │
└──────────────▲──────────────────────┘
               │ uses BufferManager
┌──────────────┴──────────────────────┐
│ Network Layer (Dev A)               │
│  Server:                            │
│    - bufferManager_                 │
│    handleClientInput():             │
│      bufferManager_.feed(fd, data)  │
│      msgs = bufferManager_          │
│             .extractMessages(fd)    │
└─────────────────────────────────────┘

┌─────────────────────────────────────┐
│ Logic Layer (Dev C)                 │
│  Client - NO buffer knowledge!      │
│    - nickname_, username_...        │
│    (NO receiveBuffer_)              │
└─────────────────────────────────────┘
```

**Benefits:**

-Clear separation of concerns
-Client only handles business logic
-BufferManager only handles CRLF framing
-Easy to test independently
-Follows Radio France architecture

## Key Arguments

### 1. **Architecture Alignment to production standarts**

**Streaming platform:**

```
SRT Listener (transport) → buffer() → Liquidsoap (processing)
     ↓                        ↓              ↓
  Network              BufferManager      Parser
```

Our IRC could follow the same pattern.

### 3. **Testing Benefits**

**Variant 1** (Buffer in Client):

```cpp
// Need full Client object to test buffering
Client client(fd);
client.setNickname("test");
client.appendToBuffer("NICK\r\n");  // Mixed concerns!
```

**Variant 3** (Separate BufferManager):

```cpp
// Test buffering independently
BufferManager bm;
bm.feed(1, "NICK te");
bm.feed(1, "st\r\n");
assert(bm.extractMessages(1).size() == 1);

// Test Client independently
Client client(1);
client.setNickname("test");
assert(client.getNickname() == "test");
```

### 4. **Dependency Flow**

**Variant 1:**

```
Network → Client (tight coupling)
```

**Variant 3:**

```
Network → BufferManager (loose coupling)
Client ← (no dependency on buffering)
```

## Code Comparison

### Variant 1: Buffer in Client

```cpp
// Client.hpp (Logic Layer)
class Client {
private:
    std::string receiveBuffer_;  // ← mixing concerns
public:
    void appendToBuffer(const std::string& data);
    std::vector<std::string> extractMessages();
};

// Server.cpp (Network Layer)
void Server::handleClientInput(int fd) {
    recv(fd, buf, 4096, 0);
    client->appendToBuffer(buf);  // ← depends on Client
    msgs = client->extractMessages();
}
```

### Variant 3: Separate BufferManager

```cpp
// BufferManager.hpp (Parser Layer)
class BufferManager {
private:
    std::map<int, std::string> buffers_;
public:
    void feed(int fd, const std::string& chunk);
    std::vector<std::string> extractMessages(int fd);
    void clear(int fd);
};

// Server.cpp (Network Layer)
class Server {
    BufferManager bufferManager_;  // ← clean dependency

    void handleClientInput(int fd) {
        recv(fd, buf, 4096, 0);
        bufferManager_.feed(fd, buf);
        msgs = bufferManager_.extractMessages(fd);
    }
};

// Client.hpp (Logic Layer)
class Client {
    // NO receiveBuffer_!
    std::string nickname_;  // ← only business logic
};
```

## Implementation Plan

### For Dev B (Parser Layer):

**Create:** `include/irc/BufferManager.hpp`, `src/BufferManager.cpp`

```cpp
class BufferManager {
private:
    std::map<int, std::string> buffers_;

public:
    void feed(int fd, const std::string& chunk) {
        buffers_[fd].append(chunk);
    }

    std::vector<std::string> extractMessages(int fd) {
        std::vector<std::string> messages;
        std::string& buf = buffers_[fd];
        size_t pos;

        while ((pos = buf.find("\r\n")) != std::string::npos) {
            messages.push_back(buf.substr(0, pos));
            buf.erase(0, pos + 2);
        }
        return messages;
    }

    void clear(int fd) {
        buffers_.erase(fd);
    }
};
```

### For Dev A (Network Layer):

**Modify:** `src/Server.cpp`

```cpp
// Add member
BufferManager bufferManager_;

// In handleClientInput():
void Server::handleClientInput(int clientFd) {
    char buffer[^4096];
    ssize_t n = recv(clientFd, buffer, sizeof(buffer), 0);

    if (n <= 0) {
        disconnectClient(clientFd);
        return;
    }

    // Feed to BufferManager
    bufferManager_.feed(clientFd, std::string(buffer, n));

    // Extract complete messages
    std::vector<std::string> messages =
        bufferManager_.extractMessages(clientFd);

    // Process messages
    Client* client = clients_[clientFd];
    for (size_t i = 0; i < messages.size(); ++i) {
        IRCMessage msg = parser_.parse(messages[i]);
        registry_.execute(this, client, msg);
    }
}

// In disconnectClient():
void Server::disconnectClient(int clientFd) {
    bufferManager_.clear(clientFd);  // Clean up buffer
    // ... rest of cleanup
}
```

### For Dev C (Logic Layer):

**Modify:** `include/irc/Client.hpp` - **Remove** buffer-related code:

```cpp
class Client {
private:
    int fd_;
    std::string nickname_;
    // NO receiveBuffer_!

public:
    // NO appendToBuffer()!
    // NO extractMessages()!

    // Only business logic
    void setNickname(const std::string& nick);
    bool isRegistered() const;
};
```

## Summary

**Recommendation:** Use **Variant 3** 

**Reasons:**

1.✅ Matches architecture industry standard
2.✅ Clear separation of concerns (SRP)
3.✅ Easy to test independently
4.✅ Loose coupling between layers
5.✅ BufferManager naturally belongs to Parser Layer (Dev B)

**File Structure:**

```
include/irc/
  ├── Server.hpp         (Network - Dev A)
  ├── BufferManager.hpp  (Parser - Dev B)  ← NEW
  ├── Parser.hpp         (Parser - Dev B)
  └── Client.hpp         (Logic - Dev C)

src/
  ├── Server.cpp
  ├── BufferManager.cpp  ← NEW (Dev B implements)
  ├── Parser.cpp
  └── Client.cpp
```

**Owner:** Dev B (Parser Layer) implements BufferManager
**Users:** Dev A (Network) uses it, Dev C (Logic) doesn't know about it

```
может это повтор но лучше больше чем меньше выше , мы выбрали вариант 3 (Separate BufferManager)

*******
такой еще был разговор со студентом закончившим проект и я передал как мог :
**Description:** Implement registration state machine that tracks client registration progress. Client is registered only when PASS + NICK + USER are all completed.

**Tasks:**

1. Review registration flow:
    - Client connects → not registered
    - Client sends PASS → password set
    - Client sends NICK → nickname set
    - Client sends USER → username set
    - All three complete → register client
2. Implement `Client::canRegister()` in `src/Client.cpp`
    - Check: `hasPassword() && hasNickname() && hasUsername()`
    - Return true if all set
3. Update `Client::setNickname()` to check registration
    - After setting nickname, check `canRegister()`
    - If yes, call `registerClient()`
4. Update `Client::setUsername()` to check registration
    - After setting username, check `canRegister()`
    - If yes, call `registerClient()`
5. Reject commands before registration (except PASS, NICK, USER, PING, PONG, QUIT):
    - In CommandRegistry or individual commands
    - Send ERR_NOTREGISTERED (451) if not registered
6. Test registration flow:
    
    ```bash
    # Correct flow
    echo -e "PASS password\r\nNICK test\r\nUSER user host server :Real\r\n" | nc localhost 6667
    
    # Wrong order
    echo -e "NICK test\r\nPASS password\r\n" | nc localhost 6667
    # Should still work (order flexible)
    
    ```
    

**Acceptance Criteria:**

- ✅ Registered only with PASS + NICK + USER
- ✅ Commands rejected before registration (except PASS, NICK, USER, PING, PONG, QUIT)
- ✅ Registration order flexible (can send NICK before PASS, etc.)
- ✅ Welcome messages sent on registration
- ✅ State tracked correctly (`isRegistered()`)

**Files to modify:**

- `src/Client.cpp` (implement `canRegister()`, update setters)
- `src/commands/*.cpp` (add registration check to commands)
*******
ну и последнее прислал Parser B

Hi @Sarait Hernández @Alex 
Here you can find the results of my investigation and knowledge gathering. Below this line there is summarized flow and connection points between parser and other modules. 
Above this comment in this page you can find detailed results of my study covering every point I have found important. 
# **IRC Parser & Protocol
Plan for study**

### **1. IRC Protocol (RFC 1459 and RFC 2812)**

- Message format: `[:<prefix>] <command> [<params>] [:<trailing>]`
- What is the prefix (message origin, optional)
- Commands and their parameters
- Numeric replies (001, 433, 462, etc.)
- CRLF (`\r\n`) as message delimiter
- Connection handshake: CAP, PASS, NICK, USER

**Resources:**

- RFC 1459 (sections 2.3 and 2.4) - **MANDATORY**
- RFC 2812 (sections 2.3 - Message format)
- Connect to a real IRC server (irc.libera.chat) with irssi and observe traffic
- Use `tcpdump` or `wireshark` to see raw messages

**Practice:**

- Connect to real IRC server and manually send commands
- Observe numeric replies and their format

### **2. String Parsing in C++98**

- `std::string` and its methods (find, substr, erase, compare)
- Manual tokenization (no `split()` in C++98)
- Handling CRLF delimiters (`\r\n`)
- Parsing with spaces and colons
- Extracting trailing parameter (text after `:`)

**Resources:**

- C++98 string reference
- Practice parsing: `"NICK alice\r\n"`, `"PRIVMSG #test :hello world\r\n"`

**Practice:**

- Write parser for simple IRC messages
- Handle edge cases (multiple spaces, no trailing, etc.)

### **3. Data Buffering & Partial Messages**

- Why data arrives fragmented (TCP is a stream protocol)
- How to accumulate data until a complete message
- The subject test with `nc` and Ctrl+D (sending in parts)
- Buffer management strategies
- When to extract complete messages (CRLF found)

**Resources:**

- Beej's Guide Chapter 6.3
- Subject example: `nc -C 127.0.0.1 6667` then send `com^Dman^Dd`

**Practice:**

- Write buffer class that accumulates until `\r\n`
- Test with fragmented input

### **4. IRC Response Format & Numeric Replies**

- Structure of numeric replies: `:server 001 nick :Welcome message\r\n`
- Standard error messages format
- How a real server responds (observe with irssi)
- Building correct IRC messages to send
- Common numeric replies needed:
- 001-004: Welcome sequence
- 331-332: Topic replies
- 353, 366: Names list
- 401, 403: Not found errors
- 431-433: Nickname errors
- 461-464: Registration errors
- 473, 475: Channel errors
- 482: Operator privileges

**Resources:**

- RFC 2812 section 5 (Replies)
- Connect to real server and capture all numeric replies

**Practice:**

- Write helper functions to build numeric replies
- Test format with irssi

### **5. Command Registry Pattern**

- How to route parsed commands to handlers
- Map/function pointer pattern in C++98
- Unknown command handling

**Checklist for Dev B:**

- [x]  Can parse IRC message format correctly
- [x]  Understands CRLF framing
- [x]  Can handle partial messages
- [x]  Knows common numeric replies
- [x]  Can build correctly formatted IRC responses

---

# Study results

You can find TL;DR section at the end of this note.

## 1. IRC Protocol Foundation (RFC 1459/2812)

### Message Anatomy

Every message is a single line of text ending in `\\r\\n`.

| **Component** | **Description** | **Example** |
| --- | --- | --- |
| **Prefix** | (Optional) Starts with `:`. Identifies the sender. | `:Angel!batman@island.org` |
| **Command** | A string (e.g., `KICK`) or 3-digit numeric (e.g., `404`). | `PRIVMSG` |
| **Params** | Space-separated arguments (Max 15). | `#42Warsaw` |
| **Trailing** | The last param; starts with `:` to allow spaces. | `:Hello world!` |
- The **Prefix** is rarely sent by a client to a server, but it is mandatory for your server to include it when sending messages to clients. It identifies the source (e.g., `:YourServerName` or `:Nickname!User@Host`).
- The **Trailing** parameter is the only one allowed to contain spaces. It is identified by a leading colon (`:`).

### The Handshake Sequence

Before a user can execute commands like JOIN or PRIVMSG, they must register. The typical flow is:

1. PASS: PASS secret_password — Validates the server password.
2. NICK: NICK user123 — Sets the display name.
3. USER: USER guest 0 * :Real Name — Sets username and real name.
4. Server Reply: Once valid, the server sends Numeric 001 (RPL_WELCOME).

- **Connection Handshake:**
    - **PASS:** Must be checked first. If the password is wrong or missing, the connection should be rejected.
    - **NICK/USER:** These populate the client's identity. A user is not considered "registered" until they have successfully sent PASS, NICK, and USER.
    - **CAP:** Modern clients send this to ask for "Capabilities" (like multi-prefix). Since you are doing a basic server, you can usually ignore this or send a "Numeric 421" (Unknown Command).

### Important Numeric Replies

Numerics are how the server reports success or error.

- `001`: Welcome to the network (Successful registration).
- `433`: Nickname is already in use (Error).
- `461`: Need more parameters (Error).
- `464`: Password incorrect (Error).

**CRLF (`\\r\\n`):**

- The protocol specifies `\\r\\n` as the delimiter. However, many modern clients (and `nc`) might only send `\\n`. Your parser should ideally handle both to be robust.

### Testing Tools

- Netcat (nc): Best for testing raw, broken, or partial packets. Use nc -C [host] [port] to ensure \r\n is sent.
- Irssi: A standard CLI client to test "real-world" behavior.
- **Tcpdump:** `sudo tcpdump -i lo0 -A port 6667` to see raw traffic on localhost.

---

### **2. String Parsing in C++98**

### Technical Expansion

- **`std::string` Methods:**
    - **`find()`**: primary tool for locating spaces (`' '`) or the CRLF (`\\r\\n`) delimiter.
    - **`substr()`**: used to extract the command and parameters once their positions are found.
    - **`erase()`**: helpful for cleaning up the buffer once a command has been processed.
- **Manual Tokenization:**
    - Since C++98 lacks a split function, you must loop through the string. A common pitfall is multiple spaces (e.g., `COMMAND param1`). A simple split might create empty strings; a robust parser skips consecutive delimiters.
- **The "Trailing" Logic:**
    - The character `:` is a special marker in IRC. If it appears at the start of a parameter, that parameter includes **everything** until the end of the line, including spaces. You must check for the prefix `:` at the start of a message and the trailing `:` for the last parameter.
- **Handling CRLF (`\\r\\n`):**
    - The subject specifies that messages must be aggregated and rebuilt. You cannot assume a message is complete until you find the `\\r\\n` sequence.

### Core `std::string` Toolkit

In C++98, we rely on these to dissect messages:

- `s.find(str, pos)`: Returns `std::string::npos` if not found. Essential for finding the first space.
- `s.substr(start, len)`: Extracts a piece of the string. **Warning:** `len` is the number of characters, not the end index.
- `s.erase(pos, len)`: Removes characters from the buffer after they are parsed.
- `s.at(index)`: Safer than `[]` because it throws an exception if out of bounds.

### Parsing Strategy: Step-by-Step

To parse a message like `:Guest1 PRIVMSG #chan :Hello world\\r\\n`:

1. **Check for Prefix:** If `msg[0] == ':'`, find the first space. Everything between `msg[1]` and that space is your **prefix**.
2. **Extract Command:** Find the next non-space character. Read until the next space. This is your **command** (e.g., `PRIVMSG`).
3. **Loop for Params:**
    - If the next character is `:`, the rest of the string (minus the `\\r\\n`) is the **trailing parameter**. Stop looping.
    - Otherwise, read until the next space. This is a **regular parameter**.

4. **Stop at CRLF:** Always ensure you stop before the `\\r\\n`.

### Edge Cases to Handle

- **Multiple Spaces:** `KICK #channel user` should be treated the same as `KICK #channel user`.
- **No Trailing:** `JOIN #42` is valid; not every command needs a colon.
- **Empty Messages:** If a user just hits "Enter," your parser should ignore the empty string and not crash.
- **Leading/Trailing Spaces:** Be sure to `trim` spaces if your client sends them unexpectedly.

### Error handling

It is highly likely that a client will send incorrect messages, either due to human error (manual `nc` entry) or a buggy client implementation. Your server must be robust enough to handle these without crashing.

### Types of Possible Errors

When a message like `KICCK` arrives, your parser will encounter several types of failures:

- **Unknown Commands:** The command does not exist in the IRC protocol (e.g., `KICCK`, `LOL`, `HLEP`).
- **Malformed Syntax:** The message violates the protocol structure, such as missing spaces, starting with multiple colons, or exceeding the 512-character limit.
- **Parameter Mismatch:** The command is valid (e.g., `KICK`), but it has too few parameters (e.g., `KICK #channel` without a username).
- **Incomplete Data:** The message is cut off before the CRLF (`\\r\\n`) terminator.

### Required Error Handling

According to the subject, your error handling must follow two main rules:

- **No Crashing:** Your program must not crash under any circumstances, even if it runs out of memory or receives total gibberish. If it quits unexpectedly, the grade is 0.
- **Protocol-Appropriate Responses:** When a user sends an error, you shouldn't just "print" an error to your server console. You must send a **Numeric Reply** back to the client so they know what went wrong.

**Common Numeric Replies for Parser Errors:**

- `421 (ERR_UNKNOWNCOMMAND)`: Sent when the command (like `KICCK`) is unrecognized.
- `461 (ERR_NEEDMOREPARAMS)`: Sent when a command lacks the required number of arguments.
- `417 (ERR_INPUTTOOLONG)`: (Optional but good) Sent if a single message exceeds the protocol size limit.

### Logic for the Parser

Instead of hardcoding every error, your parser should be a "filter."

**Goal:** The server must never crash and must always inform the user of mistakes using IRC Numerics.

1. The "Unknown Command" Rule

If a user sends `KICCK #chan user`:

- **Parser action:** Identify the string `KICCK`.
- **Logic action:** Check the Command Registry. If not found, return: `:<servername421 <nicknameKICCK :Unknown command`

2. The "Missing Params" Rule

If a user sends `KICK #channel` (missing the target user):

- **Parser action:** Split into `KICK` and `#channel`.
- **Logic action:** Check the required parameter count for `KICK` (which is 2).
- **Server response:** `461 ERR_NEEDMOREPARAMS`.

3. Buffer Overflows & Length

- **Strict Limit:** The IRC protocol limits messages to 512 characters. * **Action:** If a client sends a message longer than this without a `\\n`, you must decide whether to truncate it or drop it to prevent memory exhaustion.

4. Empty or Whitespace-Only Lines

- If a client sends `\\r\\n` or `\\r\\n`, the parser should silently ignore it. Do not attempt to process an empty command.

---

### **3. Data Buffering & Partial Messages**

### The "Stream" Problem

TCP does **not** guarantee that one message arrives in one piece. We might receive `NICK t`, then `om\\r`, then `\\n`. Our code must handle this by "remembering" the fragments.

### Implementation Logic

To handle this according to the subject requirements:

1. **Storage:** Each `Client` object must have a `std::string _buffer;` or a separate sub-class that will contain it and related methods.
2. **Reception:** When `poll()` signals `POLLIN`, use `recv()` and append the result to `_buffer`.
3. **Validation:**
    - Search for `\\r\\n` in the `_buffer`.
    - **If found:** Extract the substring from the start to `\\r\\n`. This is your "Command."
    - **Cleanup:** Remove that extracted substring from the `_buffer`.
    - **Loop:** Check if there is another `\\r\\n` (the client might have sent multiple commands at once).
    - **If NOT found:** Do nothing. Leave the data in the buffer and wait for the next `poll()` event.

### Common Pitfalls

- **Forgetting the Buffer:** Never use a global or temporary buffer that clears after the function ends. Data will be lost.
- **Blocking:** Ensure `fcntl(fd, F_SETFL, O_NONBLOCK)` is set so `recv()` doesn't hang your whole server.
- **The "nc" Test:** You must be able to handle a command sent byte-by-byte via `nc -C` and Ctrl+D.

---

### **4. IRC Response Format & Numeric Replies**

### Structure of Numeric Replies

Most communication from the server to the client uses Numeric Replies. These are 3-digit codes followed by parameters. The standard format is:
`:<servername> <numeric_code> <target_nick> <params> :<text_message>\\r\\n`

- **Prefix (`:servername`)**: Tells the client which server is sending the message.
- **Numeric (`001`)**: A 3-digit code identifying the type of reply.
- **Target (`nick`)**: The nickname of the user receiving the message.
- **Trailing (`:text`)**: Often contains the human-readable part of the message, preceded by a colon.

### Standard Error Messages

When a command fails (e.g., trying to join a password-protected channel without a key), the server must return a specific error numeric.

- Format: `:<servername> <error_numeric> <nick> <offending_param> :<error_reason>\\r\\n`
- **Example (461 - ERR_NEEDMOREPARAMS):** `:ft_irc.42.pl 461 user1 KICK :Not enough parameters\\r\\n`.

### The Welcome Sequence (Registration)

A client is not fully "logged in" until they receive the welcome sequence. These are the first things your server must send after a successful `NICK` and `USER` command:

- **001 (RPL_WELCOME)**: The "Welcome to the Network" message.
- **002-004**: System info (Your host, server version, and available user/channel modes).

### Observing Real Servers

Using a client like **irssi** or **nc** to connect to an official server is the best way to see these in action. By using `irssi` with the `/raw log on` command, you can see the raw strings coming from the server, which includes all the numerics and colons usually hidden by the UI.

### Format Cheat Sheet

Every message sent from your server **must** end with `\\r\\n`.

| **Type** | **Format Template** |
| --- | --- |
| **Numeric Reply** | `:<servername> <3-digit-code> <nick> <params> :<text>\\r\\n` |
| **Command Response** | `:<user_prefix> <COMMAND> <params>\\r\\n` |

### Required Numerics for ft_irc

Based on the subject requirements, we must implement at least these:
**Connection & Registration:**

- `001` (RPL_WELCOME): Must be sent after successful registration.
- `461` (ERR_NEEDMOREPARAMS): Sent if a command (like `KICK`) is missing arguments.
- `464` (ERR_PASSWDMISMATCH): Sent if the password provided doesn't match the server password.

**Channel Operations (KICK, INVITE, TOPIC, MODE):**

- `331` / `332` (RPL_NOTOPIC / RPL_TOPIC): For the `TOPIC` command.
- `353` (RPL_NAMREPLY): List of users in a channel (sent after `JOIN`).
- `403` (ERR_NOSUCHCHANNEL): Sent if a user tries to `JOIN` or `KICK` from a non-existent channel.
- `482` (ERR_CHANOPRIVSNEEDED): Sent if a regular user tries to use an operator-only command.

**Nickname Errors:**

- `431` (ERR_NONICKNAMEGIVEN): No nickname supplied.
- `433` (ERR_NICKNAMEINUSE): The nickname is already taken by another client.

### 💡 Best Practice: The "Reply Builder"

Don't hardcode strings in your logic. Create a helper function or static class:
C++

```
// Example Concept
std::string msg = Reply::make(RPL_WELCOME, user, "Welcome to our irc server!");
// Output: ":ft_irc 001 tom :Welcome to our irc server!\\r\\n"

```

---

### **5. Command Registry Pattern**

### How to route parsed commands to handlers

Once your parser identifies a command (like `KICK` or `JOIN`), the server needs to find the specific block of code responsible for that action. In a professional C++98 architecture, you should decouple the **parsing** (identifying the string "KICK") from the **execution** (the logic that removes a user from a channel).

### Map/Function Pointer Pattern in C++98

The most efficient way to do this in C++98 is using a `std::map` that links a `std::string` (the command name) to a **function pointer** or a **Command Object**

- **The Signature:** All command handlers should have the same signature, usually taking a reference to the `Client` who sent it and a `vector` of arguments.
- **The Registry:** A `std::map<std::string, void (*)(Client&, const std::vector<std::string>&)>`.
- **The Lookup:** Instead of 20 `if` statements, you simply do `_commands.find("KICK")`. If it exists, you execute the function pointer.

### Unknown Command Handling

If the `find()` operation fails, the command is "Unknown." According to the IRC protocol, the server shouldn't just stay silent

- **Error 421 (ERR_UNKNOWNCOMMAND):** The server must send a numeric reply back to the client: `:<servername> 421 <nick> <command> :Unknown command\\r\\n`.

### The Goal

To avoid "Spaghetti Code" by creating a central dispatcher that routes strings to their respective logic functions.

### C++98 Implementation (Map of Function Pointers)

Since we are restricted to **C++98**, we use `std::map` and standard function pointers.

**1. Define the Handler Type:**

```
typedef void (*CommandHandler)(Client& sender, const std::vector<std::string>& args);
```

**2. The Command Map:** Store this inside your `Server` class:

```
std::map<std::string, CommandHandler> _commandRegistry;
```

**3. Initialization:** Populate the map when the server starts:

```
_commandRegistry["NICK"] = &handleNick;
_commandRegistry["JOIN"] = &handleJoin;
_commandRegistry["KICK"] = &handleKick; // Operator specific [cite: 116]
```

### Execution Flow

1. **Parse:** Extract the command name (e.g., `TOPIC`) from the incoming buffer.
2. **Search:** `it = _commandRegistry.find(cmdName);`
3. **Execute:** 
    - If `it != _commandRegistry.end()`, call `it->second(sender, args);`.
    - Else, send `ERR_UNKNOWNCOMMAND (421)`.

### Benefits for our Team

- **Modularity:** The "Network" dev just hands a string to the "Parser" dev, who then hands a function pointer to the "Logic" dev.
- **Scalability:** Adding a new command like `INVITE` or `MODE` only requires adding one line to the registry and writing the handler function.
- **Clean Code:** Keeps the `Server.cpp` file clean and organized, which is a requirement of the project.

---

# TL;DR

## The Parser Flow: From Bytes to Action

1. **Input: The Raw Buffer (Entry)**
    - **Receiver:** The Parser receives a reference to a `Client` object the data recently received from a `recv()` call.
    - **Context:** The "Network Dev" has already placed this data into the client-specific buffer to handle fragmentation.
2. **Step 1: Message Aggregation & Extraction**
    - **The Action:** The parser searches for the `\\r\\n` delimiter (CRLF).
    - **Internal Work:** It "slices" the first complete message out of the buffer. If the buffer contains `KICK #chan user\\r\\nNICK newname\\r\\n`, the parser must extract them one by one.
    - **Buffer Cleanup:** The processed portion is removed from the client's internal buffer so it isn't processed twice.
3. **Step 2: Syntax Decomposition**
    - **The Action:** The raw string (e.g., `KICK #channel user :Reason here`) is split into components.
    - **Internal Work:**
        - Identify the **Command** (the first word, converted to uppercase: `KICK`).
        - Identify the **Parameters** (space-separated: `#channel`, `user`).
        - Identify the **Trailing** (anything after the first colon `:`: `Reason here`).
4. **Step 3: Validation & The Registry Lookup**
    - **The Action:** The parser checks if the command exists in the **Command Registry**.
    - **Error Handling:** If the command is not found, the parser immediately generates a **Numeric Reply 421 (ERR_UNKNOWNCOMMAND)** to be sent back to the client.
5. **Result: The "Command Request" Object**
    - **The Output:** The parser produces a clean structure (often a `Command` object or a struct) containing the sender's reference, the command name, and a `std::vector<std::string>` of arguments.
    - **Where it finishes:** The parser's job ends once it has identified **who** is calling **what** and with **which parameters**.
6. **Handover: Execution**
    - **Who executes?** The **Logic Dev's** handlers.
    - **The Mechanism:** The Parser uses the **Command Registry** (the Map of Function Pointers) to trigger the specific function (e.g., `handle_kick()`) developed by Logic dev.
    - **Execution Data:** The Logic handler receives the validated parameters and performs the actual server-side changes (removing the user from the channel, updating the channel list, etc.).

### Networking ↔ Parser

Currently I’m in favor of Variant 3 from this note:
[https://www.notion.so/ft_irc-2cbbb1f38a4b802298b2f409599c0f74?source=copy_link#2cfbb1f38a4b80f092fdd5a205299fc7](https://www.notion.so/2cbbb1f38a4b802298b2f409599c0f74?pvs=21)

assuming:

- MessageBuffer will be a separate class; can be managed by Parser
- Networking will specify what it needs for the feeding method there to fill the buffer with incoming data
- Parser will create extract and clear methods (and further methods if necessary)
- It will be added as a subclass to Client class by Logic

### Parser ↔ Logic

In the architecture of your **ft_irc** server, the **Command Registry** and **Numeric Responses** represent the two sides of a conversation: the Registry is how the server "listens" and routes requests, while the Numeric Responses are how the server "speaks" back to the client.

### Connection & Differences

| **Feature** | **Command Registry** | **Numeric Responses** |
| --- | --- | --- |
| **Direction** | **Inbound** (Client → Server) | **Outbound** (Server → Client) |
| **Purpose** | To route a recognized command (e.g., `KICK`) to its logic handler. | To inform the client of success, status, or errors. |
| **Format** | Plain text string (e.g., "JOIN"). | 3-digit code + parameters (e.g., "461"). |
| **Trigger** | Triggered by the **Parser** after finding `\\r\\n`. | Triggered by **Logic Handlers** or the **Parser** during errors. |

### How They Relate (The "Chain of Custody")

They are connected through a standard execution flow. One often leads to the other:

1. **The Parser** checks the **Command Registry** to see if the command exists.
2. **Scenario A (Failure):** If the command is *not* in the Registry, the **Parser** immediately triggers a **Numeric Response** (e.g., `421 ERR_UNKNOWNCOMMAND`).
3. **Scenario B (Success):** If the command *is* in the Registry, the **Parser** hands control to the **Logic Handler** (e.g., `handle_kick`).
4. **The Logic Handler** then performs checks (e.g., "Is this user an operator?").
5. **The Result:** The Logic Handler triggers various **Numeric Responses** (e.g., `482 ERR_CHANOPRIVSNEEDED` if they aren't an operator, or `332 RPL_TOPIC` on success).

### 📥 Inbound: The Parser & Registry

- **The Parser** is the only module that talks to the **Command Registry**.
- It identifies the command from the stream and "plugs" it into the map.
- **Trigger:** Arrival of a complete message ending in `\\r\\n`.

### 📤 Outbound: The Logic & Numerics

- **The Logic Handlers** are the primary users of **Numeric Responses**.
- **The Parser** only sends a Numeric if the command is unrecognizable or the syntax is broken.
- **The Server** sends the welcome sequence (`001-004`) only after the Parser confirms both `NICK` and `USER` are correctly received.
- **Constraint:** All responses must be sent through the non-blocking `send()` function via the `poll()` (or equivalent) loop to avoid hanging the server.

### Summary for your Team

- **You (Parser):** You own the **Registry**. You use it to find the right logic. If you can't find it, you send a **Numeric Error**.
- **Logic Dev:** They own the **Handlers**. They use the **Numerics** to tell the client if their request (KICK, JOIN, etc.) worked or failed.
- **Network Dev:** They provide the "transport" (the `send()` calls) to make sure those **Numerics** actually reach the client without blocking the server.

### **Parsed Command Structure Format**

The format as provided in the link below is in my opinion sufficient and enough. The contract part also is OK for me.

https://github.com/SaraitHernandez/irc_server/blob/main/docs/TEAM_CONVENTIONS.md#3-parsed-command-structure-format

### Message Interface

Messages structures and contract assumptions are good for me. 

https://github.com/SaraitHernandez/irc_server/blob/main/docs/TEAM_CONVENTIONS.md#6-message-interface

## Git branching convention

I propose:
[network/parser/logic]/task_number/feature_description

e.g.

parser/09/create_message_buffer

