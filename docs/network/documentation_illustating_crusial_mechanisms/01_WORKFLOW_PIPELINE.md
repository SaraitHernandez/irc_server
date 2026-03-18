# IRC Server Workflow Pipeline

Complete ASCII workflow diagram showing the entire call chain and program flow.

---

## Full Pipeline: Event Loop & Command Execution

```
═══════════════════════════════════════════════════════════════════════════════
                    IRC SERVER PIPELINE - CALL CHAIN FLOW
═══════════════════════════════════════════════════════════════════════════════

START: main()
│
├─► Config::parseArgs(argc, argv)
│   │
│   └─► Config object created with <port> <password>
│
├─► Server(config)
│   │
│   └─► Initialize members: serverSocketFd_, clients_, channels_, buffers_
│
├─► Server::start()
│   │
│   ├─► createServerSocket()
│   │   └─► socket(AF_INET, SOCK_STREAM, 0)
│   │
│   ├─► bindSocket()
│   │   └─► bind() on specified port
│   │
│   ├─► listenSocket()
│   │   └─► listen() with backlog=10
│   │
│   ├─► setNonBlocking(serverSocketFd_)
│   │   └─► fcntl(F_SETFL, O_NONBLOCK)
│   │
│   └─► Poller::Poller(this)
│       └─► poller_->addFd(serverSocketFd_, POLLIN)
│
│
├─► Server::run()  [MAIN EVENT LOOP]
│   │
│   ├─► signal(SIGINT, signalHandler)
│   ├─► signal(SIGTERM, signalHandler)
│   ├─► signal(SIGQUIT, signalHandler)
│   └─► signal(SIGPIPE, SIG_IGN)
│
│   WHILE (Server::running_ == true)
│   │
│   ├─► Poller::poll(timeout=1000ms)
│   │   │
│   │   └─► ::poll(&pollfds_[], timeout)  [ONLY place poll() is called]
│   │       │
│   │       └─► Returns: number of ready file descriptors
│   │
│   ├─ ready > 0 ? YES ✓
│   │   │
│   │   └─► Poller::processEvents()
│   │       │
│   │       FOR each pollfd with revents != 0
│   │       │
│   │       ├─ fd == serverSocketFd_ ? YES ✓
│   │       │   │
│   │       │   ├─ revents & POLLIN ? YES ✓
│   │       │   │   │
│   │       │   │   └─► Server::handleNewConnection()
│   │       │   │       │
│   │       │   │       ├─► accept(serverSocketFd_, ...)
│   │       │   │       │   │
│   │       │   │       │   ├─ errno == EAGAIN/EWOULDBLOCK ? YES ✓
│   │       │   │       │   │   └─► Return (no connection)
│   │       │   │       │   │
│   │       │   │       │   └─ NO ✗
│   │       │   │       │
│   │       │   │       ├─► setNonBlocking(clientFd)
│   │       │   │       ├─► inet_ntoa(clientAddr) → hostname
│   │       │   │       ├─► Client* client = new Client(clientFd)
│   │       │   │       ├─► client->setHostname(hostname)
│   │       │   │       ├─► clients_[clientFd] = client
│   │       │   │       ├─► MessageBuffer* buffer = new MessageBuffer()
│   │       │   │       ├─► buffers_[clientFd] = buffer
│   │       │   │       └─► poller_->addFd(clientFd, POLLIN)
│   │       │   │
│   │       │   └─ NO ✗
│   │       │
│   │       └─ fd != serverSocketFd_ ? YES ✓  [CLIENT SOCKET]
│   │           │
│   │           ├─ revents & (POLLIN | POLLHUP) ? YES ✓
│   │           │   │
│   │           │   └─► Server::handleClientInput(fd)
│   │           │       │
│   │           │       ├─► recv(fd, buffer, 4096)
│   │           │       │   │
│   │           │       │   ├─ bytesRead < 0 ? YES ✓
│   │           │       │   │   │
│   │           │       │   │   ├─ errno == EAGAIN/EWOULDBLOCK ? YES ✓
│   │           │       │   │   │   └─► Return (no data yet)
│   │           │       │   │   │
│   │           │       │   │   └─ NO ✗
│   │           │       │   │       └─► disconnectClient(fd)
│   │           │       │   │
│   │           │       │   ├─ bytesRead == 0 ? YES ✓
│   │           │       │   │   └─► disconnectClient(fd)
│   │           │       │   │
│   │           │       │   └─ bytesRead > 0 ? YES ✓
│   │           │       │       │
│   │           │       │       ├─► MessageBuffer::append(data)
│   │           │       │       │
│   │           │       │       ├─► MessageBuffer::extractMessages()
│   │           │       │       │   │
│   │           │       │       │   └─► findMessageEnd() → Extract all \r\n delimited messages
│   │           │       │       │
│   │           │       │       └─► FOR each complete message
│   │           │       │           │
│   │           │       │           ├─► Parser::parse(message, cmd)
│   │           │       │           │   │
│   │           │       │           │   ├─► Extract: prefix, command, params, trailing
│   │           │       │           │   │
│   │           │       │           │   └─ Parsing successful ? YES ✓
│   │           │       │           │       │
│   │           │       │           │       └─► CommandRegistry::execute(*server, *client, cmd)
│   │           │       │           │           │
│   │           │       │           │           ├─ handlers_.find(cmd.command) exists ? YES ✓
│   │           │       │           │           │   │
│   │           │       │           │           │   └─► CommandHandler(server, client, cmd)
│   │           │       │           │           │       │
│   │           │       │           │           │       ├─ Command: PASS ? YES ✓
│   │           │       │           │           │       │   │
│   │           │       │           │           │       │   └─► handlePass(server, client, cmd)
│   │           │       │           │           │       │       │
│   │           │       │           │           │       │       ├─ registrationStep > 1 ? YES ✓
│   │           │       │           │           │       │       │   └─► ERR_ALREADYREGISTRED
│   │           │       │           │           │       │       │
│   │           │       │           │           │       │       ├─ params.empty() ? YES ✓
│   │           │       │           │           │       │       │   └─► ERR_NEEDMOREPARAMS
│   │           │       │           │           │       │       │
│   │           │       │           │           │       │       ├─ password != server.password ? YES ✓
│   │           │       │           │           │       │       │   │
│   │           │       │           │           │       │       │   ├─► client->incrementPasswordAttempts()
│   │           │       │           │           │       │       │   │
│   │           │       │           │           │       │       │   ├─ attempts > 3 ? YES ✓
│   │           │       │           │           │       │       │   │   │
│   │           │       │           │           │       │       │   │   ├─► ERR_PASSWDMISMATCH
│   │           │       │           │           │       │       │   │   └─► disconnectClient(fd)
│   │           │       │           │           │       │       │   │
│   │           │       │           │           │       │       │   └─ NO ✗
│   │           │       │           │           │       │       │       └─► ERR_PASSWDMISMATCH (allow retry)
│   │           │       │           │           │       │       │
│   │           │       │           │           │       │       └─ NO ✗ [PASS OK]
│   │           │       │           │           │       │           │
│   │           │       │           │           │       │           ├─► client->setPassword()
│   │           │       │           │           │       │           └─► registrationStep: 0 → 1
│   │           │       │           │           │       │
│   │           │       │           │           │       ├─ Command: NICK ? YES ✓
│   │           │       │           │           │       │   │
│   │           │       │           │           │       │   └─► handleNick(server, client, cmd)
│   │           │       │           │           │       │       │
│   │           │       │           │           │       │       ├─ !registered && step < 1 ? YES ✓
│   │           │       │           │           │       │       │   └─► ERR_NOTREGISTERED
│   │           │       │           │           │       │       │
│   │           │       │           │           │       │       ├─ params.empty() ? YES ✓
│   │           │       │           │           │       │       │   └─► ERR_NONICKNAMEGIVEN
│   │           │       │           │           │       │       │
│   │           │       │           │           │       │       ├─ !Utils::isValidNickname(newNick) ? YES ✓
│   │           │       │           │           │       │       │   └─► ERR_ERRONEUSNICKNAME
│   │           │       │           │           │       │       │
│   │           │       │           │           │       │       ├─ getClientByNickname(newNick) exists ? YES ✓
│   │           │       │           │           │       │       │   └─► ERR_NICKNAMEINUSE
│   │           │       │           │           │       │       │
│   │           │       │           │           │       │       └─ NO ✗ [NICK OK]
│   │           │       │           │           │       │           │
│   │           │       │           │           │       │           ├─► client->setNickname(newNick)
│   │           │       │           │           │       │           ├─ step == 1 ? YES ✓
│   │           │       │           │           │       │           │   └─► client->completeNickStep() → step 1 → 2
│   │           │       │           │           │       │           │
│   │           │       │           │           │       │           └─ !wasRegistered && onChannel ? YES ✓
│   │           │       │           │           │       │               └─► Broadcast NICK to channels
│   │           │       │           │           │       │
│   │           │       │           │           │       ├─ Command: USER ? YES ✓
│   │           │       │           │           │       │   │
│   │           │       │           │           │       │   └─► handleUser(server, client, cmd)
│   │           │       │           │           │       │       │
│   │           │       │           │           │       │       ├─ !has PASS ? YES ✓
│   │           │       │           │           │       │       │   └─► ERR_NOTREGISTERED
│   │           │       │           │           │       │       │
│   │           │       │           │           │       │       ├─ params.size() < 4 ? YES ✓
│   │           │       │           │           │       │       │   └─► ERR_NEEDMOREPARAMS
│   │           │       │           │           │       │       │
│   │           │       │           │           │       │       └─ NO ✗ [USER OK]
│   │           │       │           │           │       │           │
│   │           │       │           │           │       │           ├─► client->setUsername(username, realname)
│   │           │       │           │           │       │           ├─► client->registerClient()
│   │           │       │           │           │       │           │   └─► registrationStep: 2 → 3
│   │           │       │           │           │       │           │
│   │           │       │           │           │       │           ├─ client->isRegistered() ? YES ✓
│   │           │       │           │           │       │           │   └─► Send RPL_WELCOME, RPL_YOURHOST, RPL_CREATED, RPL_MYINFO
│   │           │       │           │           │       │           │
│   │           │       │           │           │       │           └─► server->sendResponse(...) → send(...) to socket
│   │           │       │           │           │       │
│   │           │       │           │           │       ├─ Command: JOIN ? YES ✓
│   │           │       │           │           │       │   │
│   │           │       │           │           │       │   └─► handleJoin(server, client, cmd)
│   │           │       │           │           │       │       │
│   │           │       │           │           │       │       ├─ !isRegistered ? YES ✓
│   │           │       │           │           │       │       │   └─► ERR_NOTREGISTERED
│   │           │       │           │           │       │       │
│   │           │       │           │           │       │       ├─ params.empty() ? YES ✓
│   │           │       │           │           │       │       │   └─► ERR_NEEDMOREPARAMS
│   │           │       │           │           │       │       │
│   │           │       │           │           │       │       ├─ !channel exists ? YES ✓
│   │           │       │           │           │       │       │   │
│   │           │       │           │           │       │       │   └─► Channel* = new Channel(name)
│   │           │       │           │           │       │       │       └─► channels_[name] = channel
│   │           │       │           │           │       │       │
│   │           │       │           │           │       │       └─ YES ✓ [JOIN OK]
│   │           │       │           │           │       │           │
│   │           │       │           │           │       │           ├─► channel->addClient(client)
│   │           │       │           │           │       │           ├─► client->addChannel(channelName)
│   │           │       │           │           │       │           ├─► Broadcast JOIN to all channel members
│   │           │       │           │           │       │           │
│   │           │       │           │           │       │           ├─ channel->hasTopic() ? YES ✓
│   │           │       │           │           │       │           │   └─► Send RPL_TOPIC
│   │           │       │           │           │       │           │
│   │           │       │           │           │       │           └─► Send RPL_NAMREPLY (list members)
│   │           │       │           │           │       │
│   │           │       │           │           │       ├─ Command: PRIVMSG ? YES ✓
│   │           │       │           │           │       │   │
│   │           │       │           │           │       │   └─► handlePrivmsg(server, client, cmd)
│   │           │       │           │           │       │       │
│   │           │       │           │           │       │       ├─ !isRegistered ? YES ✓
│   │           │       │           │           │       │       │   └─► ERR_NOTREGISTERED
│   │           │       │           │           │       │       │
│   │           │       │           │           │       │       ├─ params.empty() || no trailing ? YES ✓
│   │           │       │           │           │       │       │   └─► ERR_NORECIPIENT / ERR_NOTEXTTOSEND
│   │           │       │           │           │       │       │
│   │           │       │           │           │       │       └─ NO ✗ [PRIVMSG OK]
│   │           │       │           │           │       │           │
│   │           │       │           │           │       │           ├─ target starts with # ? YES ✓ [CHANNEL]
│   │           │       │           │           │       │           │   │
│   │           │       │           │           │       │           │   ├─ channel exists ? YES ✓
│   │           │       │           │           │       │           │   │   │
│   │           │       │           │           │       │           │   │   ├─ client in channel ? YES ✓
│   │           │       │           │           │       │           │   │   │   │
│   │           │       │           │           │       │           │   │   │   └─► channel->broadcast(message, exclude=client)
│   │           │       │           │           │       │           │   │   │       │
│   │           │       │           │           │       │           │   │   │       └─► server->sendToClient(targetFd, message)
│   │           │       │           │           │       │           │   │   │           └─► send(socket, message, MSG_NOSIGNAL)
│   │           │       │           │           │       │           │   │   │
│   │           │       │           │           │       │           │   │   └─ NO ✗
│   │           │       │           │           │       │           │   │       └─► ERR_NOTONCHANNEL
│   │           │       │           │           │       │           │   │
│   │           │       │           │           │       │           │   └─ NO ✗
│   │           │       │           │           │       │           │       └─► ERR_NOSUCHCHANNEL
│   │           │       │           │           │       │           │
│   │           │       │           │           │       │           ├─ NO ✗ [NICKNAME TARGET]
│   │           │       │           │           │       │           │   │
│   │           │       │           │           │       │           │   ├─ nickname exists ? YES ✓
│   │           │       │           │           │       │           │   │   │
│   │           │       │           │           │       │           │   │   └─► server->sendToClient(targetFd, message)
│   │           │       │           │           │       │           │   │
│   │           │       │           │           │       │           │   └─ NO ✗
│   │           │       │           │           │       │           │       └─► ERR_NOSUCHNICK
│   │           │       │           │           │       │           │
│   │           │       │           │           │       │           └─► (more commands: PART, QUIT, KICK, MODE, TOPIC, PING, PONG)
│   │           │       │           │           │       │
│   │           │       │           │           │       └─ NO ✗ [Command not found]
│   │           │       │           │           │           └─► (silently ignore or send ERR_UNKNOWNCOMMAND)
│   │           │       │           │           │
│   │           │       │           │           └─ NO ✗ [Parsing failed]
│   │           │       │           │               └─► (silently ignore malformed message)
│   │           │       │           │
│   │           │       │           └─ [Continue next message in extractMessages() loop]
│   │           │       │
│   │           │   └─ NO ✗
│   │           │
│   │           └─ NO ✗
│   │
│   └─ ready <= 0 ? YES ✓
│       └─► [No ready fds, continue loop with timeout]
│
└─► Server::~Server()
    │
    ├─► FOR each client in clients_
    │   ├─► close(fd)
    │   └─► delete client
    │
    ├─► FOR each buffer in buffers_
    │   └─► delete buffer
    │
    ├─► FOR each channel in channels_
    │   └─► delete channel
    │
    ├─► close(serverSocketFd_)
    │
    └─► delete poller_


═══════════════════════════════════════════════════════════════════════════════
LEGEND:
│     = Vertical flow continuation
├─►   = Command/method call with branching
└─►   = Final command/method call
─ condition ? YES ✓ / NO ✗  = Decision/branching point
═════════════════════════════════════════════════════════════════════════════════
```

---

## Key Points

### 1. Initialization Phase
- Parse command-line arguments: `<port>` and `<password>`
- Create server socket, bind to port, set to listen mode
- Initialize Poller with server socket FD
- Register signal handlers (SIGINT, SIGTERM, SIGQUIT, SIGPIPE)

### 2. Main Event Loop (Server::run)
- Continuously polls file descriptors with `poll()` system call
- Poller is the ONLY class that calls `poll()` (see TEAM_CONVENTIONS.md)
- Timeout: 1 second

### 3. Connection Handling
- When server socket is ready (POLLIN), accept new connection
- Create Client object, initialize MessageBuffer
- Add client FD to Poller for monitoring

### 4. Data Reception Pipeline
1. **recv()**: Read up to 4096 bytes from client socket
2. **MessageBuffer::append()**: Accumulate partial data
3. **MessageBuffer::extractMessages()**: Split on `\r\n` delimiters
4. **Parser::parse()**: Convert raw IRC message → Command struct
5. **CommandRegistry::execute()**: Dispatch to command handler

### 5. Registration Flow (Strict Order)
```
Step 0 (CONNECTED)
  ↓
[PASS] → Step 1 (HAS_PASS)
  ↓
[NICK] → Step 2 (HAS_NICK)
  ↓
[USER] → Step 3 (REGISTERED)
```

### 6. Command Handler Examples

- **PASS**: Verify password, advance registration
- **NICK**: Validate format, check uniqueness, advance registration
- **USER**: Complete registration, send welcome messages
- **JOIN**: Create channel if needed, add client, broadcast
- **PRIVMSG**: Route to channel members or direct client

### 7. Broadcasting
- Channel::broadcast() sends message to all channel members
- Excludes sender by comparing Client pointers
- Uses Server::sendToClient() for actual socket writes

### 8. Cleanup
- Disconnect broadcasts QUIT message to all channels
- Remove client from channels, close FD, delete objects
- Proper memory deallocation on shutdown

---

## Architecture Layers

| Layer | Components | Role |
|-------|-----------|------|
| **System I/O** | Poller, poll() syscall | File descriptor monitoring |
| **Socket I/O** | recv(), send() | Network communication |
| **Message Buffer** | MessageBuffer, Parser | Message assembly & parsing |
| **Protocol** | Command, CommandRegistry | IRC command dispatch |
| **Logic** | Client, Channel, Replies | Business rules & state |
| **Application** | Server, main() | Coordination & lifecycle |

---

## Error Handling Strategy

- **Validation errors**: Send IRC numeric replies (ERR_*)
- **Socket errors**: Disconnect client and log
- **Parser errors**: Silently ignore malformed messages
- **Signal handling**: Graceful shutdown on SIGINT/SIGTERM
- **Memory leaks**: Destructor cleans all dynamically allocated objects
