# Network Layer Implementation Guide

## Overview

This guide describes the minimal compilable version of the IRC server's Network layer. The version contains the core components for managing sockets, clients, and I/O events.

## Network Layer Structure

```
Network Layer (Alex)
├── Server         - main class for server management
├── Poller         - I/O event handling via poll()
├── Config         - server configuration (port, password)
├── Client         - client state (stub for now)
└── MessageBuffer  - buffer for partial messages (stub for now)
```

## Key Components

### 1. Config (src/Config.cpp, include/irc/Config.hpp)

**Purpose:** Manage server configuration

**Main Methods:**
```cpp
Config(int port, const std::string& password);  // Constructor
int getPort() const;                            // Get port
const std::string& getPassword() const;         // Get password
Config parseArgs(int argc, char** argv);        // Parse command-line arguments
```

**How it works:**
```bash
./ircserv 6667 mypassword
# parseArgs() will extract port 6667 and password mypassword
```

### 2. Poller (src/Poller.cpp, include/irc/Poller.hpp)

**Purpose:** The only class that calls poll() to monitor file descriptors

**CRITICAL RULE:** Only Poller calls poll()!

**Main Methods:**
```cpp
Poller(Server* server);              // Constructor, stores Server pointer
void addFd(int fd, short events);    // Add fd for monitoring (POLLIN, POLLOUT)
void removeFd(int fd);               // Remove fd from monitoring
int poll(int timeout = -1);          // Call poll(), return ready fds
void processEvents();                // Process events (calls Server methods)
```

**Lifecycle:**
1. Server adds server socket fd to Poller
2. Poller maintains list of file descriptors in `pollfds_`
3. poll() is called once per main loop
4. processEvents() iterates results and calls appropriate Server methods

### 3. Server (src/Server.cpp, include/irc/Server.hpp)

**Purpose:** Coordinate all components, manage sockets and clients

**Main Methods:**

#### Initialization:
```cpp
Server(const Config& config);  // Constructor
~Server();                     // Destructor, cleanup resources
void start();                  // Create socket, bind, listen
void run();                    // Main event processing loop
```

#### Connection Management:
```cpp
void handleNewConnection();         // [TODO] Accept new connection
void handleClientInput(int fd);     // [TODO] Read data from client
void disconnectClient(int fd);      // [TODO] Disconnect client
```

#### Data Transmission:
```cpp
void sendToClient(int fd, const std::string& msg);  // [TODO] Send message
void sendResponse(...);                              // [TODO] Send IRC response
void broadcastToChannel(...);                        // [TODO] Broadcast to channel
```

#### Client Management:
```cpp
Client* getClient(int fd);                   // Get client by fd
// Client* getClientByNickname(...);         // [TODO] Get by nickname
// void addClient(...);                      // [TODO] Add client
// void removeClient(...);                   // [TODO] Remove client
```

#### Helper Methods:
```cpp
void createServerSocket();    // Create TCP socket
void bindSocket();            // Bind to port
void listenSocket();          // Enable listening mode
void setNonBlocking(int fd);  // Set non-blocking mode
int getServerFd() const;      // Get main socket fd
```

### 4. Client & MessageBuffer

These classes contain only stubs for now. They will be filled in subsequent development phases.

## Main Execution Loop

```
main()
  ↓
Config::parseArgs(argc, argv)  // Parse arguments
  ↓
Server::Server(config)         // Create server
  ↓
Server::start()                // Initialize socket
  ├─ createServerSocket()
  ├─ bindSocket()
  ├─ listenSocket()
  ├─ new Poller(this)
  └─ poller_->addFd(serverSocketFd_, POLLIN)
  ↓
Server::run()                  // Main loop
  ├─ poller_->poll(timeout)
  └─ poller_->processEvents()  // If ready fds
       ├─ handleNewConnection()
       ├─ handleClientInput(fd)
       └─ disconnectClient(fd)
```

## Architectural Decisions

### 1. Forward Declaration in Poller.hpp

**Problem:** Circular dependency (Poller needs Server*, Server needs Poller*)

**Solution:**
```cpp
// In Poller.hpp
class Server;  // Forward declaration, don't include Server.hpp

class Poller {
private:
    Server* server_;  // Can use pointer
};
```

**Why this works:**
- Pointer has known size, doesn't need full class definition
- Full Server definition is included in Poller.cpp

### 2. Only poll() in Poller

**Why this is important:**
- Centralized I/O multiplexing
- Easier debugging (single place where blocking occurs)
- Follows team conventions (TEAM_CONVENTIONS.md)

**Each class is responsible for its own:**
- Poller → poll() and processEvents()
- Server → socket and client management
- Parser → message parsing (future)
- CommandRegistry → command execution (future)

### 3. Non-blocking Sockets

All sockets are set to non-blocking mode:
```cpp
fcntl(fd, F_SETFL, O_NONBLOCK);
```

**Why this is necessary:**
- poll() works with non-blocking sockets
- If recv() returns EAGAIN → data not ready, loop continues
- If recv() returns 0 → connection closed

## What's Left to Do (TODO)

### Phase 2: Client Management
- [ ] Implement Server::handleNewConnection()
  - Accept new connection
  - Create Client object
  - Add fd to Poller with POLLIN
- [ ] Implement Client class
  - Store nickname, username, authentication state
  - Contain MessageBuffer

### Phase 3: Data Processing
- [ ] Implement Server::handleClientInput()
  - recv() with EAGAIN handling
  - Append to Client::messageBuffer_
  - extractMessages() for complete \r\n messages
- [ ] Implement MessageBuffer
  - append() to add data
  - extractMessages() to extract complete \r\n messages

### Phase 4: Commands and Responses
- [ ] Implement Server::sendToClient()
  - send() with error handling
  - Log sent data
- [ ] Connect Parser and CommandRegistry
  - Parse IRC commands
  - Execute PASS, NICK, USER, etc.

## How to Compile and Test

### Build:
```bash
make clean
make
```

### Run:
```bash
./ircserv 6667 mypassword
```

Server should output:
```
[Server] Created with port=6667
[Server] Bound to port 6667
[Server] Listening backlog=10
[Server] Listening on port 6667
[Server] Running event loop...
```

### Testing (for now):
```bash
# In another terminal
nc localhost 6667
# Nothing will happen yet (handleNewConnection not implemented)
```

## Team Conventions (from TEAM_CONVENTIONS.md)

### 1. Poll() Responsibility
- **Only Poller** calls poll()
- Server does NOT call poll()
- Other classes do NOT call poll()

### 2. Sending Responses
- **All** responses go through Server::sendToClient()
- Never write directly to fd
- Command handlers do NOT write directly to socket

### 3. Layer Interaction
```
Network Layer (Poller, Server)
        ↓ calls
Parser Layer (Parser, Replies)
        ↓ calls
Logic Layer (Client, Channel, Commands)
```

## Files Modified

### Created:
- `src/Config.cpp` - Config class implementation

### Uncommented in Headers:
- `include/irc/Config.hpp` - method and field declarations
- `include/irc/Server.hpp` - Network layer method declarations
- `include/irc/Poller.hpp` - with forward declaration for Server

### Added to Implementations:
- `src/Server.cpp` - stubs for methods used by Poller
- `src/main.cpp` - uncommented main startup loop
- `src/Poller.cpp` - added #include <cstring> for strerror()

### Removed/Hidden:
- Channel.hpp dependency in Server
- Channel management methods (commented for now)

## Next Steps

1. Implement handleNewConnection() in Server
2. Create Client class with full information
3. Implement MessageBuffer for data buffering
4. Add handleClientInput() and disconnectClient() handling
5. Connect parser layer for IRC command processing

## References

- [TEAM_CONVENTIONS.md](TEAM_CONVENTIONS.md) - team conventions
- [DEVELOPMENT_PLAN.md](DEVELOPMENT_PLAN.md) - full development plan
- [our_comments.md](our_comments.md) - detailed implementation comments
