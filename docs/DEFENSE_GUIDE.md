# Defense Preparation Guide

**Project:** ft_irc (IRC Server)  
**Date:** March 1, 2026  
**Status:** ✅ Ready for evaluation

---

## 📋 Quick Reference

### Project Info
- **Language:** C++98
- **Compilation:** `make`
- **Usage:** `./ircserv <port> <password>`
- **Example:** `./ircserv 6667 mypassword`

### Key Features
- ✅ Non-blocking I/O with `poll()`
- ✅ Multiple simultaneous clients
- ✅ All required IRC commands
- ✅ All channel modes (+i, +t, +k, +o, +l)
- ✅ Proper cleanup and no memory leaks

---

## 🎯 Demo Script

### 1. Compilation (30 seconds)
```bash
# Show clean compilation
make clean
make

# Result: Should compile without warnings or errors
```

### 2. Start Server (10 seconds)
```bash
./ircserv 6667 test123
```

Expected output:
```
[Server] Created with port=6667
[Server] Bound to port 6667
[Server] Listening backlog=10
[Server] Listening on port 6667
[Server] Running event loop...
```

### 3. Basic Connection Test (1 minute)
In another terminal:
```bash
nc localhost 6667
```

Then type:
```irc
PASS test123
NICK evaluator
USER eval 0 * :Evaluation User
```

Expected response:
```
:ft_irc 001 evaluator :Welcome to the IRC Network evaluator!eval@127.0.0.1
:ft_irc 002 evaluator :Your host is ft_irc, running version 1.0
:ft_irc 003 evaluator :This server was created today
:ft_irc 004 evaluator ft_irc 1.0 o itkol
```

### 4. Channel Operations (2 minutes)
```irc
JOIN #test
```

Expected:
```
:evaluator!eval@127.0.0.1 JOIN :#test
:ft_irc 331 evaluator #test :No topic is set
:ft_irc 353 evaluator = #test :@evaluator
:ft_irc 366 evaluator #test :End of /NAMES list
```

Set topic:
```irc
TOPIC #test :This is a test channel
```

Send message:
```irc
PRIVMSG #test :Hello everyone!
```

### 5. Multiple Clients (2 minutes)
Open another terminal and connect:
```bash
nc localhost 6667
PASS test123
NICK alice
USER alice 0 * :Alice Smith
JOIN #test
PRIVMSG #test :Hi from Alice!
```

Both clients should see each other's messages.

### 6. Operator Commands (2 minutes)

In first client (operator):
```irc
MODE #test +i
INVITE alice #test
KICK alice :Just testing
MODE #test +k secretkey
MODE #test +l 5
```

### 7. Cleanup Test (1 minute)
```irc
QUIT :Goodbye
```

Server should properly clean up the connection.

---

## 🤔 Expected Questions & Answers

### Technical Questions

**Q: Why did you choose `poll()` instead of `select()` or `epoll()`?**

A: We chose `poll()` because:
1. It's required by the subject (can use poll, select, epoll, or kqueue)
2. It's more portable than `epoll` (Linux-only) or `kqueue` (BSD/macOS-only)
3. It doesn't have the FD_SETSIZE limitation of `select()`
4. For our use case (moderate number of clients), performance is sufficient

**Code reference:** `src/Poller.cpp` - single `poll()` call in the entire codebase

---

**Q: How do you handle partial data?**

A: We use a `MessageBuffer` class for each client:
1. When data arrives, we append it to the buffer
2. We search for complete messages (ending with `\r\n`)
3. We extract complete messages and leave incomplete data in the buffer
4. The next `recv()` adds to the remaining buffer

**Code reference:** `src/MessageBuffer.cpp` - `append()` and `extractMessages()` methods

**Example:**
```
Receive: "NICK tes"        → Buffer: "NICK tes"
Receive: "tuser\r\nJOIN"   → Extract: "NICK testuser\r\n", Buffer: "JOIN"
Receive: " #test\r\n"      → Extract: "JOIN #test\r\n", Buffer: ""
```

---

**Q: How does your registration system work?**

A: Three-step authentication (PASS → NICK → USER):
1. Client must send PASS first
2. Then NICK to set nickname (checked for uniqueness)
3. Finally USER to complete registration
4. Only after all three, client can use other commands

**Code reference:** 
- `src/commands/Pass.cpp` - Sets password
- `src/commands/Nick.cpp` - Sets nickname
- `src/commands/User.cpp` - Completes registration and sends welcome

---

**Q: How do you implement channel operators?**

A: 
1. First user to join a channel automatically becomes operator
2. Operators stored in `Channel::operators_` vector
3. Commands check `channel->isOperator(&client)` before allowing actions
4. Operators can: KICK, INVITE (in +i channels), change TOPIC (in +t channels), change MODE

**Code reference:** `src/Channel.cpp` - operator management methods

---

**Q: How do you handle case-insensitive nicknames?**

A: We store TWO versions of each nickname:
1. **Lowercase version** - for comparison and lookups
2. **Original case version** - for display in messages

Example: User sets nickname "BoB"
- Stored as `nickname_ = "bob"` (for comparison)
- Stored as `nicknameDisplay_ = "BoB"` (for display)
- Lookup for "bob", "BOB", or "Bob" all find the same user
- Messages show "BoB!user@host" (original case)

**Code reference:** `src/Client.cpp` - `setNickname()` method

---

**Q: How do you prevent memory leaks?**

A: Multiple strategies:
1. **Destructors** - Proper cleanup in `~Server()`, `~Client()`, `~Channel()`
2. **Disconnect cleanup** - `disconnectClient()` removes from all channels
3. **Empty channel deletion** - Channels deleted when last user leaves
4. **Smart resource management** - Every `new` has corresponding `delete`

**Verification:** Run `leaks` (macOS) or `valgrind` (Linux) to verify no leaks.

---

**Q: How does MODE command work?**

A: Simplified implementation (one mode at a time):
1. Parse mode string (e.g., "+i", "+k password", "+o nick")
2. Check if client is operator
3. Apply mode to channel
4. Broadcast MODE change to all members

Supported modes:
- **+i/-i** - Invite-only (no parameter)
- **+t/-t** - Topic protected (no parameter)
- **+k/-k** - Channel key (requires password parameter)
- **+o/-o** - Operator (requires nickname parameter)
- **+l/-l** - User limit (requires number parameter)

**Code reference:** `src/commands/Mode.cpp`

---

### Architecture Questions

**Q: How is your code organized?**

A: Three-layer architecture:

1. **Network Layer (Alex)**
   - `Server.cpp` - Socket operations, accept, send/recv
   - `Poller.cpp` - I/O multiplexing with `poll()`
   - `MessageBuffer.cpp` - Message buffering

2. **Parser Layer (Artur)**
   - `Parser.cpp` - Parse IRC messages
   - `Replies.cpp` - Format IRC responses
   - `CommandRegistry.cpp` - Route commands to handlers

3. **Logic Layer (Sara)**
   - `Client.cpp` - Client state management
   - `Channel.cpp` - Channel state management
   - `commands/*.cpp` - Command implementations

**Benefit:** Clear separation of concerns, easier to test and maintain.

---

**Q: How do commands get executed?**

A: Command execution flow:

```
1. Client sends: "PRIVMSG #test :Hello\r\n"
2. Server::handleClientInput() receives data
3. MessageBuffer::append() stores data
4. MessageBuffer::extractMessages() finds complete message
5. Parser::parse() creates Command struct
6. CommandRegistry::execute() finds PRIVMSG handler
7. Privmsg::execute() processes the command
8. Server::sendToClient() sends responses
```

**Code references:**
- `src/Server.cpp` - handleClientInput() (lines 260-305)
- `src/Parser.cpp` - parse()
- `src/CommandRegistry.cpp` - execute()

---

**Q: How do you handle errors?**

A: Multiple levels:

1. **Network errors**
   - EAGAIN/EWOULDBLOCK - Non-blocking would block
   - EPIPE - Client disconnected
   - Error handling in `sendToClient()` and `handleClientInput()`

2. **IRC protocol errors**
   - ERR_NEEDMOREPARAMS (461) - Missing parameters
   - ERR_NOTREGISTERED (451) - Not authenticated
   - ERR_NOSUCHNICK (401) - User not found
   - ERR_NOSUCHCHANNEL (403) - Channel not found
   - etc.

3. **State errors**
   - Commands check registration state
   - Commands check operator privileges
   - Commands validate parameters

**Code reference:** `include/irc/Replies.hpp` - Error codes

---

### Bonus Questions

**Q: Can you show me the poll() loop?**

A: The main loop is in `Server::run()`:
```cpp
while (running_) {
    int ready = poller_->poll(1000);  // 1 second timeout
    if (ready > 0) {
        poller_->processEvents();  // Handle ready fds
    }
}
```

`Poller::poll()` is the ONLY place `::poll()` system call is made.

**Code reference:** `src/Server.cpp` - run() method

---

**Q: How do you test with multiple clients?**

A: Multiple ways:

1. **Multiple nc connections:**
```bash
# Terminal 1
./ircserv 6667 test

# Terminal 2
nc localhost 6667

# Terminal 3
nc localhost 6667

# Both connect simultaneously
```

2. **Real IRC client (irssi):**
```bash
irssi -c localhost -p 6667
```

3. **Script test:**
```bash
echo -e "PASS test\r\nNICK test1\r\nUSER test 0 * :Test\r\n" | nc localhost 6667 &
echo -e "PASS test\r\nNICK test2\r\nUSER test 0 * :Test\r\n" | nc localhost 6667 &
```

---

**Q: What happens when a client disconnects unexpectedly?**

A: 
1. `poll()` detects POLLHUP event or `recv()` returns 0
2. Server calls `disconnectClient(fd)`
3. Client removed from all channels
4. Empty channels deleted
5. Socket closed
6. Client object deleted
7. Buffer cleaned up

**Code reference:** `src/Server.cpp` - disconnectClient() (lines 309-351)

---

**Q: How do you handle the subject's Ctrl+D test?**

A: The `MessageBuffer` handles this:
```bash
nc -C 127.0.0.1 6667
com^Dman^Dd
```

This sends in three parts: "com", "man", "d\r\n"

1. First recv: "com" → buffered
2. Second recv: "man" → buffered (now "comman")
3. Third recv: "d\r\n" → complete message "command\r\n" extracted

**Code reference:** `src/MessageBuffer.cpp` - extractMessages()

---

## 🔍 Common Issues to Check

### Before Defense

- [ ] Clean compilation: `make clean && make`
- [ ] No warnings or errors
- [ ] Server starts successfully
- [ ] Can connect with nc
- [ ] Can authenticate (PASS, NICK, USER)
- [ ] Can join channels
- [ ] Can send messages
- [ ] Multiple clients work
- [ ] Clean disconnect

### During Demo

- [ ] Show server output (demonstrates event handling)
- [ ] Show multiple clients simultaneously
- [ ] Show operator commands working
- [ ] Show proper error messages
- [ ] Show clean quit

---

## 🎓 Key Points to Remember

### What Makes Your Implementation Good

1. **Follows C++98 Standard**
   - No C++11 features
   - No lambda, auto, nullptr
   - Uses proper C++98 STL

2. **Non-Blocking I/O**
   - All sockets non-blocking
   - Uses poll() correctly
   - Handles EAGAIN/EWOULDBLOCK

3. **Proper Error Handling**
   - IRC error codes
   - Network error handling
   - State validation

4. **Memory Management**
   - No leaks (verified with leaks/valgrind)
   - Proper cleanup on disconnect
   - RAII where appropriate

5. **Team Collaboration**
   - Clear module separation
   - Well-documented interfaces
   - Followed conventions

---

## 📊 Testing Checklist

### Must Demonstrate

- [x] Server compiles
- [x] Server starts
- [x] Client connects
- [x] Client authenticates (PASS, NICK, USER)
- [x] Client joins channel
- [x] Client sends messages
- [x] Multiple clients interact
- [x] Operator commands work
- [x] Modes work (+i, +t, +k, +o, +l)
- [x] Clean disconnect

### Nice to Show

- [x] Multiple channels
- [x] Private messages
- [x] Kick/invite commands
- [x] Topic setting
- [x] No memory leaks
- [x] Handles partial data

---

## 💡 Tips for Defense

1. **Be confident** - The server works, you tested it
2. **Know your code** - You can explain every part
3. **Show, don't just tell** - Demo the features
4. **Be honest** - If you don't know something, say so
5. **Explain decisions** - Why you chose certain approaches
6. **Know the limitations** - What's not implemented (NOTICE, NAMES)
7. **Be prepared for "what if"** - How would you add X feature?

---

## 🚀 Final Checklist

### Code
- [x] Compiles without errors
- [x] No warnings
- [x] Follows C++98 standard
- [x] No forbidden functions
- [x] No memory leaks

### Features
- [x] Non-blocking I/O
- [x] Multiple clients
- [x] All required commands
- [x] All channel modes
- [x] Proper authentication

### Documentation
- [x] README updated
- [x] Comments where needed
- [x] Team conventions documented
- [x] Architecture explained

### Testing
- [x] Basic test works
- [x] Multiple clients work
- [x] IRC client (irssi) works
- [x] Partial data handled
- [x] No crashes

---

## 🎉 You're Ready!

Your IRC server is:
- ✅ Fully functional
- ✅ Well-tested
- ✅ Properly documented
- ✅ Memory safe
- ✅ Standards compliant

**Good luck with your defense!** 🚀

---

**Remember:** You built a real IRC server from scratch. That's impressive! Be proud of your work.

---

**END OF DEFENSE GUIDE**
