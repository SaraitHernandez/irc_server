# Missing Tasks to Complete IRC Server

**Last Updated:** March 1, 2026  
**Status:** ✅ ALL CRITICAL AND IMPORTANT TASKS COMPLETED!

~~This document lists all pending tasks needed to complete the IRC server implementation, organized by developer responsibility and priority.~~

**UPDATE:** All 7 critical and important tasks have been completed! The server is now fully functional.

See `COMPLETION_STATUS.md` and `PROJECT_COMPLETE.md` for details.

---

## 🔴 CRITICAL TASKS (Server won't work without these)

These tasks are blocking the server from functioning end-to-end. They must be completed first.

### Task 1: Implement `Server::sendToClient()` [CRITICAL]

**Owner:** Alex (Dev A - Network Layer)  
**File:** `src/Server.cpp`  
**Priority:** P0 - BLOCKING ALL COMMANDS

**Problem:**
The method is declared in `include/irc/Server.hpp` but has no implementation. Every command calls this method to send responses to clients, causing linker errors.

**What needs to be done:**
Implement the method to send formatted messages to a specific client via their socket file descriptor.

**Implementation details:**
```cpp
void Server::sendToClient(int clientFd, const std::string& message) {
    // 1. Find the client by fd
    std::map<int, Client*>::iterator it = clients_.find(clientFd);
    if (it == clients_.end()) {
        std::cerr << "Error: client fd " << clientFd << " not found" << std::endl;
        return;
    }
    
    Client* client = it->second;
    
    // 2. Ensure message ends with \r\n
    std::string msg = message;
    if (msg.length() < 2 || msg.substr(msg.length() - 2) != "\r\n") {
        msg += "\r\n";
    }
    
    // 3. Send message using send()
    ssize_t sent = send(clientFd, msg.c_str(), msg.length(), 0);
    
    // 4. Handle errors
    if (sent < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // Socket buffer full, need to buffer the message
            // For now, log error (can implement send buffer later)
            std::cerr << "Warning: send() returned EAGAIN for fd " << clientFd << std::endl;
        } else if (errno == EPIPE || errno == ECONNRESET) {
            // Client disconnected
            std::cerr << "Client " << clientFd << " disconnected during send" << std::endl;
            disconnectClient(clientFd);
        } else {
            // Other error
            perror("send");
        }
    } else if (sent < (ssize_t)msg.length()) {
        // Partial send - for simplicity, log warning
        // A complete implementation would buffer the remaining data
        std::cerr << "Warning: partial send (" << sent << "/" << msg.length() 
                  << " bytes) to fd " << clientFd << std::endl;
    }
}
```

**Why it's critical:**
Without this method, no command can send responses to clients. The server will compile but crash or fail at runtime when any command tries to send data.

**References:**
- TEAM_CONVENTIONS.md Section 2 (Sending Responses to Clients)
- All command implementations use this method

---

### Task 2: Implement Channel Management Methods [CRITICAL]

**Owner:** Alex (Dev A - Network Layer)  
**Files:** `src/Server.cpp`, `include/irc/Server.hpp`  
**Priority:** P0 - BLOCKING JOIN, PART, MODE, KICK, INVITE, TOPIC

**Problem:**
Three critical methods are declared but not implemented:
1. `Server::getChannel(const std::string& name)`
2. `Server::createChannel(const std::string& name)`
3. `Server::removeChannel(const std::string& name)`

Additionally, the `channels_` map is commented out in `Server.hpp` line 24.

**What needs to be done:**

**Step 1:** Uncomment the channels map in `include/irc/Server.hpp`:
```cpp
// Line 24 in Server.hpp - UNCOMMENT THIS:
std::map<std::string, Channel*> channels_; // channel name -> Channel*
```

**Step 2:** Implement `getChannel()` in `src/Server.cpp`:
```cpp
Channel* Server::getChannel(const std::string& name) {
    // Convert channel name to lowercase for case-insensitive lookup
    std::string lowerName = Utils::toLower(name);
    
    std::map<std::string, Channel*>::iterator it = channels_.find(lowerName);
    if (it != channels_.end()) {
        return it->second;
    }
    return NULL;
}
```

**Step 3:** Implement `createChannel()` in `src/Server.cpp`:
```cpp
Channel* Server::createChannel(const std::string& name) {
    // Convert to lowercase for storage
    std::string lowerName = Utils::toLower(name);
    
    // Check if channel already exists
    if (getChannel(name) != NULL) {
        std::cerr << "Warning: attempted to create existing channel: " << name << std::endl;
        return getChannel(name);
    }
    
    // Create new channel (Channel constructor preserves original case for display)
    Channel* channel = new Channel(name);
    channels_[lowerName] = channel;
    
    std::cout << "Created channel: " << name << std::endl;
    return channel;
}
```

**Step 4:** Implement `removeChannel()` in `src/Server.cpp`:
```cpp
void Server::removeChannel(const std::string& name) {
    std::string lowerName = Utils::toLower(name);
    
    std::map<std::string, Channel*>::iterator it = channels_.find(lowerName);
    if (it != channels_.end()) {
        Channel* channel = it->second;
        
        // Only remove if channel is empty
        if (channel->isEmpty()) {
            std::cout << "Removing empty channel: " << name << std::endl;
            delete channel;
            channels_.erase(it);
        } else {
            std::cerr << "Warning: attempted to remove non-empty channel: " << name << std::endl;
        }
    }
}
```

**Why it's critical:**
- JOIN command needs `getChannel()` and `createChannel()` to work
- PART and QUIT need `removeChannel()` to clean up empty channels
- All channel-related commands depend on these methods

**References:**
- TEAM_CONVENTIONS.md Section 6 (Server Interface)
- Used by: Join.cpp, Part.cpp, Quit.cpp, Mode.cpp, Kick.cpp, Invite.cpp, Topic.cpp, Privmsg.cpp

---

### Task 3: Implement `Server::getClientByNickname()` [CRITICAL]

**Owner:** Alex (Dev A - Network Layer)  
**File:** `src/Server.cpp`  
**Priority:** P0 - BLOCKING NICK, PRIVMSG, INVITE, KICK, MODE

**Problem:**
Method is declared in `include/irc/Server.hpp` but has no implementation. Multiple commands need to find clients by their nickname.

**What needs to be done:**
Implement case-insensitive nickname search through the clients map.

**Implementation:**
```cpp
Client* Server::getClientByNickname(const std::string& nickname) {
    // Convert to lowercase for case-insensitive comparison
    std::string lowerNick = Utils::toLower(nickname);
    
    // Iterate through all clients
    std::map<int, Client*>::iterator it;
    for (it = clients_.begin(); it != clients_.end(); ++it) {
        Client* client = it->second;
        
        // Client stores nickname in lowercase internally for comparison
        if (client->getNickname() == lowerNick) {
            return client;
        }
    }
    
    // Not found
    return NULL;
}
```

**Why it's critical:**
- NICK command needs to check if nickname is already in use
- PRIVMSG needs to find target user by nickname
- INVITE needs to find user to invite
- KICK needs to find user to kick
- MODE +o/-o needs to find user to give/remove operator status

**Important notes:**
- Nicknames are case-INSENSITIVE: "Bob", "bob", and "BOB" refer to the same user
- Client class stores both lowercase (for comparison) and original case (for display)
- This method compares using lowercase version

**References:**
- TEAM_CONVENTIONS.md Section 6 (Server Interface)
- TEAM_CONVENTIONS.md Section 13 (Case Sensitivity)
- Used by: Nick.cpp, Privmsg.cpp, Invite.cpp, Kick.cpp, Mode.cpp

---

### Task 4: Connect Parser and CommandRegistry [CRITICAL]

**Owner:** Alex (Dev A - Network Layer)  
**File:** `src/Server.cpp` (lines 291-293)  
**Priority:** P0 - BLOCKS ALL MESSAGE PROCESSING

**Problem:**
In `Server::handleClientInput()`, the lines that parse messages and execute commands are commented out:

```cpp
// Line 291-293 in Server.cpp:
// TODO (Issue 1.3): Parser integration
// IRCMessage msg = parser_.parse(messages[i]);
// registry_.execute(this, client, msg);
```

The server receives messages and stores them in the buffer, but never processes them.

**What needs to be done:**
Uncomment these lines to connect the parser and command execution.

**Implementation:**
```cpp
// In Server::handleClientInput() around line 291:
std::vector<std::string> messages = client->getMessageBuffer().extractMessages();
for (size_t i = 0; i < messages.size(); ++i) {
    std::cout << "Received from fd " << clientFd << ": " << messages[i];
    
    // UNCOMMENT THESE LINES:
    IRCMessage msg = parser_.parse(messages[i]);
    registry_.execute(this, client, msg);
}
```

**Why it's critical:**
Without this, the server receives data but never processes it. No commands will work at all - the server will appear to accept connections but won't respond to any IRC commands.

**Note:**
This requires that `Parser` and `CommandRegistry` are properly initialized in the Server constructor and all commands are registered.

**References:**
- TEAM_CONVENTIONS.md Section 14 (Module Interaction Flow)
- DEVELOPMENT_PLAN.md Issue 1.3 (Read pipeline)

---

## 🟠 IMPORTANT TASKS (Server will work but with issues)

These tasks don't block basic functionality but cause significant problems.

### Task 5: Fix `Config::parseArgs()` for Positional Arguments [IMPORTANT]

**Owner:** Artur (Dev B - Parser Layer)  
**File:** `src/Config.cpp`  
**Priority:** P1 - BLOCKS AUTHENTICATION

**Problem:**
The `main.cpp` expects positional arguments:
```bash
./ircserv <port> <password>
```

But `Config::parseArgs()` only looks for flags like `-p`, `--port`, `-pass`, `--password`. When you run `./ircserv 6667 secret`, the password remains empty and authentication fails.

**What needs to be done:**
Modify `Config::parseArgs()` to support positional arguments as well as flags.

**Implementation approach:**
```cpp
void Config::parseArgs(int argc, char* argv[]) {
    // Handle positional arguments: ./ircserv <port> <password>
    if (argc == 3) {
        // Positional: argv[1] = port, argv[2] = password
        port_ = std::atoi(argv[1]);
        password_ = argv[2];
        return;
    }
    
    // Otherwise parse flags as before
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-p" || arg == "--port") {
            if (i + 1 < argc) {
                port_ = std::atoi(argv[++i]);
            }
        } else if (arg == "-pass" || arg == "--password") {
            if (i + 1 < argc) {
                password_ = argv[++i];
            }
        }
    }
}
```

**Why it's important:**
Without this fix, you must run the server as:
```bash
./ircserv --port 6667 --password secret
```

Instead of the simpler expected format:
```bash
./ircserv 6667 secret
```

This breaks the subject's expected interface and makes testing harder.

**References:**
- DEVELOPMENT_PLAN.md Issue 0.1
- main.cpp expects positional arguments

---

### Task 6: Set Client Hostname from Connection [IMPORTANT]

**Owner:** Alex (Dev A - Network Layer)  
**File:** `src/Server.cpp` (in `handleNewConnection()`)  
**Priority:** P1 - AFFECTS CLIENT PREFIX

**Problem:**
When a new client connects, the hostname is set to "unknown" and never updated. The client prefix becomes `nick!user@unknown` instead of showing the real IP address.

**What needs to be done:**
In `Server::handleNewConnection()`, after accepting a new connection, get the client's IP address and set it as the hostname.

**Implementation:**
```cpp
// In Server::handleNewConnection() after accept():
int clientFd = accept(serverSocketFd_, (struct sockaddr*)&clientAddr, &clientLen);
if (clientFd < 0) {
    // ... error handling ...
    return;
}

// Get client's IP address
struct sockaddr_in clientAddr;
socklen_t clientLen = sizeof(clientAddr);
if (getpeername(clientFd, (struct sockaddr*)&clientAddr, &clientLen) == 0) {
    char* ipStr = inet_ntoa(clientAddr.sin_addr);
    std::string hostname = std::string(ipStr);
    
    // Set hostname when creating client
    Client* client = new Client(clientFd);
    client->setHostname(hostname);
    clients_[clientFd] = client;
} else {
    // Fallback to "unknown"
    Client* client = new Client(clientFd);
    client->setHostname("unknown");
    clients_[clientFd] = client;
}
```

**Why it's important:**
- Client prefix is used in all messages: `:nick!user@host PRIVMSG #channel :message`
- Without real hostname, debugging is harder
- Some IRC clients expect a valid hostname
- The prefix `nick!user@unknown` looks unprofessional

**Note:**
You may need to adjust the `accept()` call to also capture the client address:
```cpp
struct sockaddr_in clientAddr;
socklen_t clientLen = sizeof(clientAddr);
int clientFd = accept(serverSocketFd_, (struct sockaddr*)&clientAddr, &clientLen);
```

**References:**
- TEAM_CONVENTIONS.md Section 4 (Client Interface)
- Client::getPrefix() uses hostname for the format `nick!user@host`

---

### Task 7: Complete Channel Cleanup in `disconnectClient()` [IMPORTANT]

**Owner:** Alex (Dev A - Network Layer)  
**File:** `src/Server.cpp` (lines 312-324)  
**Priority:** P1 - CAUSES ORPHANED CHANNEL MEMBERS

**Problem:**
In `Server::disconnectClient()`, there's a TODO comment at lines 312-324. Currently, when a client disconnects:
1. Client is removed from `clients_` map ✓
2. Client is removed from Poller ✓
3. Socket is closed ✓
4. Client object is deleted ✓
5. **Client is NOT removed from their channels ✗**

This leaves "ghost" users in channels.

**What needs to be done:**
Before deleting the client, iterate through all channels they're in and remove them.

**Implementation:**
```cpp
void Server::disconnectClient(int clientFd) {
    Client* client = getClient(clientFd);
    if (!client) {
        return;
    }
    
    std::cout << "Disconnecting client fd " << clientFd 
              << " (" << client->getNickname() << ")" << std::endl;
    
    // NEW CODE: Remove client from all channels
    std::vector<std::string> clientChannels = client->getChannels();
    for (size_t i = 0; i < clientChannels.size(); ++i) {
        Channel* channel = getChannel(clientChannels[i]);
        if (channel) {
            // Remove client from channel
            channel->removeClient(client);
            
            // If channel is now empty, delete it
            if (channel->isEmpty()) {
                removeChannel(clientChannels[i]);
            }
        }
    }
    
    // Remove from clients map
    clients_.erase(clientFd);
    
    // Remove from poller
    poller_.removeFd(clientFd);
    
    // Close socket
    close(clientFd);
    
    // Delete client object
    delete client;
}
```

**Why it's important:**
- Without this, channels will have "ghost" members
- Channel membership count will be incorrect
- Channel might never be deleted even when empty
- NAMES list will show disconnected users
- Messages sent to channel might try to send to closed sockets

**Note:**
The QUIT command should call this method AFTER broadcasting the QUIT message to all channels. The order is:
1. QUIT handler broadcasts `:nick QUIT :reason` to all channels
2. QUIT handler calls `server.disconnectClient(fd)`
3. `disconnectClient()` cleans up channels and closes connection

**References:**
- TEAM_CONVENTIONS.md Section 11 (Disconnect Behavior)
- Current TODO in Server.cpp line 312

---

## 🟡 MINOR TASKS (Bugs and improvements)

These are small fixes that improve code quality but don't block functionality.

### Task 8: Fix Bug in Join.cpp (First Member Operator Check) [MINOR]

**Owner:** Sara (Dev C - Logic Layer)  
**File:** `src/commands/Join.cpp` (lines 116-118)  
**Priority:** P2 - SMALL BUG

**Problem:**
When a user joins a newly created channel, they should become an operator. However, the logic checks if `channel->getClientCount() == 1` AFTER calling `channel->addClient()`. Since the count increases before the check, this condition might never be true.

**Current code (lines 116-118):**
```cpp
channel->addClient(client);  // Count becomes 1

if (channel->getClientCount() == 1) {  // This checks AFTER adding
    channel->addOperator(client);
}
```

**What needs to be done:**
Check BEFORE adding the client, or use the `isNew` flag from channel creation.

**Implementation (Option 1 - Check before adding):**
```cpp
bool shouldBeOperator = (channel->getClientCount() == 0);

channel->addClient(client);

if (shouldBeOperator) {
    channel->addOperator(client);
}
```

**Implementation (Option 2 - Use isNew flag):**
```cpp
Channel* channel = server.getChannel(channelName);
bool isNew = false;

if (!channel) {
    channel = server.createChannel(channelName);
    isNew = true;
}

channel->addClient(client);

if (isNew) {
    channel->addOperator(client);
}
```

**Why it matters:**
If the first user to join a channel doesn't become operator, the channel becomes "stuck" with no operators. Nobody can change modes, kick users, or manage the channel.

**Testing:**
```bash
# User 1 creates channel
JOIN #test
MODE #test  # Should show user as operator (@user in NAMES)

# If bug exists, user might not be operator
```

**References:**
- Current implementation in Join.cpp lines 116-118

---

### Task 9: Centralize Error Format with `Replies::formatServerName()` [MINOR]

**Owner:** Sara (Dev B - Parser Layer)  
**Files:** Multiple command files  
**Priority:** P3 - CONSISTENCY

**Problem:**
Some error messages use inconsistent formats:
- Some use `":server 412"` hardcoded
- Some use `":server 441"` with different spacing
- Server name should be centralized through `Replies` class

**What needs to be done:**
Review all error messages and ensure they use the `Replies` class helpers instead of hardcoded strings.

**Example issue in Privmsg.cpp:**
```cpp
// Current (inconsistent):
server.sendToClient(clientFd, ":server 412 " + nick + " :No text to send\r\n");

// Should be:
std::string reply = Replies::errorReply("412", nick, "", "No text to send");
server.sendToClient(clientFd, reply);
```

**Why it matters:**
- Server name should come from config, not be hardcoded
- Consistent formatting makes code more maintainable
- Easier to change format globally if needed

**References:**
- TEAM_CONVENTIONS.md Section 8 (IRC Numeric Replies Format)
- Replies class implementation

---

## ✅ OPTIONAL TASKS (Nice to have)

These tasks are mentioned in the documentation but are not required for the project.

### Task 10: Implement NOTICE Command [OPTIONAL]

**Owner:** Sara (Dev C - Logic Layer)  
**File:** Create `src/commands/Notice.cpp` and `include/irc/commands/Notice.hpp`  
**Priority:** P4 - OPTIONAL

**What it is:**
NOTICE is exactly like PRIVMSG but with one difference: it should NOT generate any automatic replies or error messages. It's used for automated messages to avoid loops.

**Implementation:**
Copy PRIVMSG implementation but remove all error replies (ERR_NOSUCHNICK, ERR_CANNOTSENDTOCHAN, etc.). Just silently fail if target doesn't exist.

**References:**
- RFC 2812 Section 3.3.2
- Mentioned in TEAM_CONVENTIONS.md

---

### Task 11: Implement Standalone NAMES Command [OPTIONAL]

**Owner:** Sara (Dev C - Logic Layer)  
**File:** Create `src/commands/Names.cpp` and `include/irc/commands/Names.hpp`  
**Priority:** P4 - OPTIONAL

**What it is:**
Currently, NAMES list is sent automatically when a user joins a channel. A standalone `/NAMES` command would allow users to query the member list at any time.

**Implementation:**
Extract the NAMES sending logic from Join.cpp into a separate command handler.

**References:**
- RFC 2812 Section 3.2.5
- Mentioned in TEAM_CONVENTIONS.md

---

## 📊 Summary Statistics

| Priority | Count | Status |
|----------|-------|--------|
| 🔴 Critical | 4 tasks | BLOCKS SERVER |
| 🟠 Important | 3 tasks | CAUSES ISSUES |
| 🟡 Minor | 2 tasks | SMALL BUGS |
| ✅ Optional | 2 tasks | NOT REQUIRED |
| **TOTAL** | **11 tasks** | **9 must be fixed** |

---

## 🎯 Recommended Implementation Order

### Phase 1: Make Server Functional (Must do first)
1. ✅ Alex: Implement `sendToClient()` [Task 1]
2. ✅ Alex: Implement channel management (3 methods) [Task 2]
3. ✅ Alex: Implement `getClientByNickname()` [Task 3]
4. ✅ Alex: Connect Parser in `handleClientInput()` [Task 4]

**After Phase 1:** Server should accept connections and process commands.

### Phase 2: Fix Critical Issues
5. ✅ Artur: Fix `Config::parseArgs()` [Task 5]
6. ✅ Alex: Set client hostname [Task 6]
7. ✅ Alex: Complete `disconnectClient()` [Task 7]

**After Phase 2:** Server should work correctly for all use cases.

### Phase 3: Polish (If time permits)
8. ⚠️ Sara: Fix Join.cpp bug [Task 8]
9. ⚠️ Sara: Centralize error format [Task 9]
10. ⚠️ Sara: Implement NOTICE [Task 10] (optional)
11. ⚠️ Sara: Implement NAMES [Task 11] (optional)

---

## 🔗 Related Documentation

- **TEAM_CONVENTIONS.md** - Explains module responsibilities and interfaces
- **DEVELOPMENT_PLAN.md** - Original development roadmap
- **IRC_LOGIC_AND_DATA_STRUCTURE.md** - IRC protocol details
- **.cursor/plans/** - Specific implementation plans

---

## ✅ Testing Checklist

After implementing the critical tasks, test the following:

### Basic Functionality
- [ ] Server starts: `./ircserv 6667 password`
- [ ] Client connects: `nc localhost 6667`
- [ ] Client registers: `PASS password`, `NICK test`, `USER test 0 * :Real`
- [ ] Client receives welcome (001-004)

### Channel Operations
- [ ] Create channel: `JOIN #test`
- [ ] First user becomes operator
- [ ] Send message: `PRIVMSG #test :Hello`
- [ ] Other user joins: second client `JOIN #test`
- [ ] Both see messages

### Operator Commands
- [ ] Set modes: `MODE #test +i`
- [ ] Kick user: `KICK #test user2`
- [ ] Invite user: `INVITE user2 #test`
- [ ] Change topic: `TOPIC #test :New topic`

### Cleanup
- [ ] Leave channel: `PART #test`
- [ ] Quit: `QUIT :Goodbye`
- [ ] Empty channel deleted
- [ ] No memory leaks (check with `valgrind` or `leaks`)

### Edge Cases
- [ ] Multiple clients simultaneously
- [ ] Partial data (use `nc` with Ctrl+D)
- [ ] Wrong password
- [ ] Duplicate nickname
- [ ] Invalid commands

---

**END OF DOCUMENT**
