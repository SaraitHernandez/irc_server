---
name: Client Registration Flow
overview: Implementar la clase Client con soporte completo para el flujo de registro estricto (PASS -> NICK -> USER) siguiendo las convenciones de Halloy, incluyendo case-insensitive nicknames, password retry, y los tres comandos de registro.
todos:
  - id: utils
    content: "Implement Utils: toLower, toUpper, trim, isValidNickname, intToString"
    status: completed
  - id: client-hpp
    content: "Update Client.hpp: add private members (registrationStep_, passwordAttempts_, nicknameDisplay_) and method declarations"
    status: completed
  - id: client-cpp
    content: "Implement Client.cpp: constructor, getters, setters with case handling, registration state machine"
    status: completed
  - id: pass-cmd
    content: "Implement PASS command: step check, password validation, retry mechanism (max 3)"
    status: completed
  - id: nick-cmd
    content: "Implement NICK command: step check, validation, case-insensitive collision check, nick change broadcast"
    status: completed
  - id: user-cmd
    content: "Implement USER command: step check, set username/realname, complete registration, send welcome messages"
    status: completed
isProject: false
---

# Client and Registration Flow Implementation

## Overview

This plan implements the `Client` class and registration commands (PASS, NICK, USER) following the strict order required by Halloy client compatibility as documented in [TEAM_CONVENTIONS.md](docs/TEAM_CONVENTIONS.md).

## Dependencies Check

Before starting, verify these are available (provided by Alex and Artur):

- `Server::sendToClient(int fd, const std::string& msg)` - for sending responses
- `Server::getPassword()` - for PASS validation
- `Server::disconnectClient(int fd)` - for max password attempts
- `Server::getClientByNickname(const std::string& nick)` - for nick collision check
- `Replies` class - already implemented by Artur
- `Command` struct - for parsed commands

---

## Phase 1: Utils Functions

File: [src/Utils.cpp](src/Utils.cpp) and [include/irc/Utils.hpp](include/irc/Utils.hpp)

Implement these utility functions first (Client depends on them):

```cpp
std::string toLower(const std::string& str);    // For case-insensitive comparison
std::string toUpper(const std::string& str);    // For commands
std::string trim(const std::string& str);       // For parsing
bool isValidNickname(const std::string& nick);  // Validation
std::string intToString(int value);             // For numeric replies
```

**Nickname validation rules (simplified):**

- Length: 1-9 characters
- First char: letter or underscore
- Rest: letters, digits, underscore

---

## Phase 2: Client Class

Files: [include/irc/Client.hpp](include/irc/Client.hpp) and [src/Client.cpp](src/Client.cpp)

### 2.1 Private Members

```cpp
private:
    int fd_;
    
    // Identity (case handling for Halloy)
    std::string nickname_;         // lowercase for comparison
    std::string nicknameDisplay_;  // original case for display
    std::string username_;
    std::string hostname_;
    std::string realname_;
    
    // Registration state machine (STRICT ORDER)
    int registrationStep_;         // 0=PASS, 1=NICK, 2=USER, 3=done
    int passwordAttempts_;         // Max 3 attempts
    static const int MAX_PASSWORD_ATTEMPTS = 3;
    
    // Channel membership
    std::vector<std::string> channels_;
    
    // Buffer (delegate to MessageBuffer if Alex provides)
    std::string buffer_;
```

### 2.2 Constructor

```cpp
Client(int fd)
    : fd_(fd)
    , registrationStep_(0)
    , passwordAttempts_(0)
{}
```

### 2.3 Identity Methods

```cpp
// Getters
int getFd() const;
const std::string& getNickname() const;        // lowercase
const std::string& getNicknameDisplay() const; // original
const std::string& getUsername() const;
const std::string& getHostname() const;
const std::string& getRealname() const;

// Setters with case handling
void setNickname(const std::string& nick) {
    nicknameDisplay_ = nick;              // "BOB"
    nickname_ = Utils::toLower(nick);     // "bob"
}
void setUsername(const std::string& user, const std::string& real);
void setHostname(const std::string& host);
```

### 2.4 Registration State Machine

```cpp
// State queries
int getRegistrationStep() const;
bool isRegistered() const;          // step == 3
bool hasPassword() const;           // step >= 1
bool hasNickname() const;           // step >= 2

// State transitions
void setPassword();                 // step 0 -> 1
void completeNickStep();            // step 1 -> 2
void registerClient();              // step 2 -> 3

// Password retry
int getPasswordAttempts() const;
void incrementPasswordAttempts();
bool hasExceededPasswordAttempts() const;  // >= MAX
```

### 2.5 Prefix Format

```cpp
std::string getPrefix() const {
    return nicknameDisplay_ + "!" + username_ + "@" + hostname_;
}
```

### 2.6 Channel Tracking

```cpp
void addChannel(const std::string& channel);
void removeChannel(const std::string& channel);
const std::vector<std::string>& getChannels() const;
bool isInChannel(const std::string& channel) const;
```

---

## Phase 3: PASS Command

File: [src/commands/Pass.cpp](src/commands/Pass.cpp)

```cpp
void handlePass(Server& server, Client& client, const Command& cmd) {
    int fd = client.getFd();
    
    // 1. Already past PASS stage?
    if (client.getRegistrationStep() > 1) {
        server.sendToClient(fd, Replies::numeric(
            Replies::ERR_ALREADYREGISTRED, 
            client.getNicknameDisplay().empty() ? "*" : client.getNicknameDisplay(),
            "", "You may not reregister"));
        return;
    }
    
    // 2. Missing parameter?
    if (cmd.params.empty() || cmd.getParam(0).empty()) {
        server.sendToClient(fd, Replies::numeric(
            Replies::ERR_NEEDMOREPARAMS, "*", "PASS", 
            "Not enough parameters"));
        return;
    }
    
    // 3. Check password
    if (cmd.getParam(0) != server.getPassword()) {
        client.incrementPasswordAttempts();
        if (client.hasExceededPasswordAttempts()) {
            server.sendToClient(fd, Replies::numeric(
                Replies::ERR_PASSWDMISMATCH, "*", "",
                "Password incorrect (max attempts exceeded)"));
            server.disconnectClient(fd);
        } else {
            server.sendToClient(fd, Replies::numeric(
                Replies::ERR_PASSWDMISMATCH, "*", "",
                "Password incorrect"));
        }
        return;
    }
    
    // 4. Success - move to step 1
    client.setPassword();
}
```

---

## Phase 4: NICK Command

File: [src/commands/Nick.cpp](src/commands/Nick.cpp)

```cpp
void handleNick(Server& server, Client& client, const Command& cmd) {
    int fd = client.getFd();
    std::string currentNick = client.getNicknameDisplay();
    if (currentNick.empty()) currentNick = "*";
    
    // 1. Must have PASS first (if not registered)
    if (!client.isRegistered() && client.getRegistrationStep() < 1) {
        server.sendToClient(fd, Replies::numeric(
            Replies::ERR_NOTREGISTERED, "*", "",
            "You have not registered"));
        return;
    }
    
    // 2. Missing parameter?
    if (cmd.params.empty() || cmd.getParam(0).empty()) {
        server.sendToClient(fd, Replies::numeric(
            Replies::ERR_NONICKNAMEGIVEN, currentNick, "",
            "No nickname given"));
        return;
    }
    
    std::string newNick = cmd.getParam(0);
    
    // 3. Valid format?
    if (!Utils::isValidNickname(newNick)) {
        server.sendToClient(fd, Replies::numeric(
            Replies::ERR_ERRONEUSNICKNAME, currentNick, newNick,
            "Erroneous nickname"));
        return;
    }
    
    // 4. Already in use? (case-insensitive check)
    Client* existing = server.getClientByNickname(newNick);
    if (existing && existing != &client) {
        server.sendToClient(fd, Replies::numeric(
            Replies::ERR_NICKNAMEINUSE, currentNick, newNick,
            "Nickname is already in use"));
        return;
    }
    
    // 5. If changing nick (already registered), broadcast change
    std::string oldPrefix = client.getPrefix();
    bool wasRegistered = client.isRegistered();
    bool hadNick = client.hasNickname();
    
    // 6. Set new nickname (stores both lowercase and display)
    client.setNickname(newNick);
    
    // 7. Advance registration if needed
    if (!wasRegistered && client.getRegistrationStep() == 1) {
        client.completeNickStep();  // step 1 -> 2
    }
    
    // 8. Broadcast nick change to channels
    if (hadNick) {
        std::string nickMsg = ":" + oldPrefix + " NICK :" + newNick + "\r\n";
        // Broadcast to all channels...
    }
}
```

---

## Phase 5: USER Command

File: [src/commands/User.cpp](src/commands/User.cpp)

```cpp
void handleUser(Server& server, Client& client, const Command& cmd) {
    int fd = client.getFd();
    std::string nick = client.getNicknameDisplay();
    if (nick.empty()) nick = "*";
    
    // 1. Already registered?
    if (client.isRegistered()) {
        server.sendToClient(fd, Replies::numeric(
            Replies::ERR_ALREADYREGISTRED, nick, "",
            "You may not reregister"));
        return;
    }
    
    // 2. Must have PASS and NICK first
    if (client.getRegistrationStep() < 2) {
        server.sendToClient(fd, Replies::numeric(
            Replies::ERR_NOTREGISTERED, nick, "",
            "You have not registered"));
        return;
    }
    
    // 3. Need at least username (params[0])
    if (cmd.params.empty()) {
        server.sendToClient(fd, Replies::numeric(
            Replies::ERR_NEEDMOREPARAMS, nick, "USER",
            "Not enough parameters"));
        return;
    }
    
    // 4. Extract username and realname
    std::string username = cmd.getParam(0);
    std::string realname = cmd.trailing.empty() 
        ? cmd.getParam(3) 
        : cmd.trailing;
    
    // 5. Set user info
    client.setUsername(username, realname);
    
    // 6. Complete registration
    client.registerClient();  // step 2 -> 3
    
    // 7. Send welcome messages (001-004)
    sendWelcomeMessages(server, client);
}

void sendWelcomeMessages(Server& server, Client& client) {
    int fd = client.getFd();
    std::string nick = client.getNicknameDisplay();
    
    // RPL_WELCOME (001)
    server.sendToClient(fd, Replies::numeric(
        Replies::RPL_WELCOME, nick, "",
        "Welcome to the IRC Network " + client.getPrefix()));
    
    // RPL_YOURHOST (002)
    server.sendToClient(fd, Replies::numeric(
        Replies::RPL_YOURHOST, nick, "",
        "Your host is " + Replies::formatServerName() + ", running version 1.0"));
    
    // RPL_CREATED (003)
    server.sendToClient(fd, Replies::numeric(
        Replies::RPL_CREATED, nick, "",
        "This server was created today"));
    
    // RPL_MYINFO (004)
    server.sendToClient(fd, Replies::numeric(
        Replies::RPL_MYINFO, nick,
        Replies::formatServerName() + " 1.0 o itkol", ""));
}
```

---

## Testing Checklist

After implementation, verify:

- NICK before PASS returns ERR_NOTREGISTERED (451)
- USER before NICK returns ERR_NOTREGISTERED (451)
- Wrong password allows retry (up to 3 times)
- 4th wrong password disconnects
- `NICK BOB` and `NICK bob` are treated as same user
- Display shows original case (`BOB`)
- Welcome messages (001-004) sent after USER
- NICK change after registration broadcasts to channels

