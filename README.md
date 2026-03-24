# IRC Server (ft_irc)

42 School project - A fully functional IRC server implementation in C++98.

## ✅ Status: COMPLETE AND FUNCTIONAL

All required features have been implemented and tested. The server is ready for evaluation.

---

## 🚀 Quick Start

### Compilation
```bash
make
```

### Running the Server
```bash
./ircserv <port> <password>
```

Example:
```bash
./ircserv 6667 mypassword
```

### Connecting with an IRC Client

**Using netcat (nc):**
```bash
nc localhost 6667
PASS mypassword
NICK mynickname
USER myuser 0 * :My Real Name
JOIN #test
PRIVMSG #test :Hello everyone!
QUIT :Goodbye
```
### our imlementation does not need to proceed <hostname> <servername>
### howhever it saves it and WHO or WHOIS would easily access if needed (call to write)
```
USER <username> <hostname> <servername> :<realname>
USER  alice      0           *           :Alice Smith
```
**Using irssi:**
```bash
irssi -c localhost -p 6667
# In irssi:
/server mypassword
/join #test
/msg #test Hello!
```

---

## 📋 Implemented Features

### ✅ Core IRC Commands
- **PASS** - Server password authentication
- **NICK** - Set/change nickname
- **USER** - Set username and realname
- **PING/PONG** - Connection keep-alive
- **QUIT** - Disconnect from server

### ✅ Channel Operations
- **JOIN** - Join or create a channel
- **PART** - Leave a channel
- **PRIVMSG** - Send messages to users or channels

### ✅ Operator Commands
- **KICK** - Remove user from channel
- **INVITE** - Invite user to invite-only channel
- **TOPIC** - Set/view channel topic
- **MODE** - Change channel modes

### ✅ Channel Modes
- **+i** - Invite-only channel
- **+t** - Topic protected (only operators can change)
- **+k** - Channel password/key
- **+o** - Channel operator privileges
- **+l** - User limit

### ✅ Technical Features
- Non-blocking I/O with `poll()`
- Multiple simultaneous clients
- Proper message buffering (handles partial data)
- Case-insensitive nicknames and channels
- Automatic cleanup on disconnect
- No memory leaks

---

## 📁 Project Structure

```
irc_server/
├── Makefile                    # Build configuration
├── README.md                   # This file
├── compile_flags.txt           # Compiler flags for IDE
├── include/irc/                # Header files
│   ├── Client.hpp              # Client state management
│   ├── Channel.hpp             # Channel state management
│   ├── Server.hpp              # Server core
│   ├── Parser.hpp              # IRC message parser
│   ├── CommandRegistry.hpp    # Command dispatcher
│   └── commands/               # Command headers
├── src/                        # Implementation files
│   ├── main.cpp               # Entry point
│   ├── Server.cpp             # Server implementation
│   ├── Client.cpp             # Client implementation
│   ├── Channel.cpp            # Channel implementation
│   ├── Parser.cpp             # Parser implementation
│   └── commands/              # Command implementations
└── docs/                      # Documentation
    ├── TEAM_CONVENTIONS.md    # Development conventions
    ├── DEVELOPMENT_PLAN.md    # Original development plan
    ├── MISSING_TASKS.md       # Task list (all complete!)
    ├── COMPLETION_STATUS.md   # Detailed completion status
    └── PROJECT_COMPLETE.md    # Final summary
```

---

## 🧪 Testing

**Complete Testing Suite in `tests/` directory!** ⭐

### Quick Test
```bash
# Terminal 1: Start server
./ircserv 6667 test123

# Terminal 2: Run automated tests
cd tests
./test_multi_clients.sh      # Multi-client test
./stress_test_10_clients.sh  # Stress test (10 clients)
```

### Testing Documentation
- **`tests/README.md`** - Testing suite overview
- **`tests/MANUAL_TESTING_GUIDE.md`** - 30 manual test cases ⭐
- **`tests/AUTOMATED_TESTING_GUIDE.md`** - Automated tests guide

### Test Results
- ✅ **31/31 tests passed** (100%)
- ✅ **Halloy compatible** (4/4 tests)
- ✅ **Multi-client verified** (3+ simultaneous)
- ✅ **No memory leaks** (valgrind/leaks verified)

### Manual Testing Example
```bash
# Connect with netcat
nc localhost 6667
PASS test123
NICK testuser
USER test 0 * :Test User
JOIN #test
PRIVMSG #test :Hello!
QUIT
```

### Memory Leak Check
```bash
# macOS
leaks -atExit -- ./ircserv 6667 test123

# Linux
valgrind --leak-check=full ./ircserv 6667 test123
```

---

## 👥 Team & Architecture

### Development Team
- **Alex** - Network Layer (Server, Poller, I/O)
- **Artur** - Parser Layer (Parser, Replies, Registry)
- **Sara** - Logic Layer (Client, Channel, Commands)

### Architecture Layers

```
┌─────────────────────────────────────┐
│   Logic Layer (Sara)                │
│   Client, Channel, Commands         │
└──────────────┬──────────────────────┘
               │
┌──────────────▼──────────────────────┐
│   Parser Layer (Artur)              │
│   Parser, Replies, Registry         │
└──────────────┬──────────────────────┘
               │
┌──────────────▼──────────────────────┐
│   Network Layer (Alex)              │
│   Server, Poller, I/O               │
└─────────────────────────────────────┘
```

---

## 📚 Documentation

### For Developers
- **docs/TEAM_CONVENTIONS.md** - Module interfaces and conventions
- **docs/DEVELOPMENT_PLAN.md** - Original development roadmap
- **docs/IRC_LOGIC_AND_DATA_STRUCTURE.md** - IRC protocol details

---

## 🔧 Technical Details

### Compilation
- **Standard:** C++98
- **Compiler:** c++ (clang++ on macOS, g++ on Linux)
- **Flags:** `-Wall -Wextra -Werror -std=c++98`
- **Platform:** macOS and Linux

### Network
- **Protocol:** TCP/IPv4
- **I/O Model:** Non-blocking with `poll()`
- **Socket Options:** `SO_REUSEADDR`
- **Message Framing:** CRLF (`\r\n`) delimited

### Features
- **Case-insensitive** nickname and channel matching
- **Case-preserving** nickname and channel display
- **Message buffering** for partial data
- **Automatic cleanup** on disconnect
- **Operator privileges** for channel management

---

## 🎯 Evaluation Checklist

### Basic Requirements
- [x] Compiles without errors
- [x] No memory leaks
- [x] Non-blocking I/O
- [x] Uses `poll()` (or equivalent)
- [x] Multiple simultaneous clients
- [x] Handles partial data correctly

### IRC Commands
- [x] PASS, NICK, USER (authentication)
- [x] JOIN, PART (channel management)
- [x] PRIVMSG (messaging)
- [x] KICK, INVITE, TOPIC, MODE (operator commands)
- [x] PING, PONG, QUIT (connection management)

### Channel Modes
- [x] +i (invite-only)
- [x] +t (topic protected)
- [x] +k (key/password)
- [x] +o (operator)
- [x] +l (user limit)

### Error Handling
- [x] Proper IRC error codes
- [x] Graceful disconnect handling
- [x] Invalid command handling
- [x] Permission checks

---

## 📖 Resources

### IRC Protocol
- [RFC 1459](https://tools.ietf.org/html/rfc1459) - Original IRC Protocol
- [RFC 2812](https://tools.ietf.org/html/rfc2812) - IRC Client Protocol

### Networking
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- `man 2 socket`, `man 2 poll`, `man 2 send`, `man 2 recv`

---

## Contributing

See `docs/TEAM_CONVENTIONS.md` for the change workflow and interface change rules.

---

## License

This is a 42 School project. Educational use only.

---

**Project:** ft_irc  
**School:** 42  
**Team:** Alex, Artur, Sara  
**Status:** Complete ✅
