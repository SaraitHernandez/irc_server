---
name: Complete IRC Commands Implementation
overview: Implementación completa de todos los comandos IRC restantes (Channel class y comandos de canal/operador), siguiendo TEAM_CONVENTIONS.md, IRC_LOGIC_AND_DATA_STRUCTURE.md, arquitectura Buffer Variant 3, y requisitos de compatibilidad Halloy.
todos:
  - id: channel-class
    content: "Implement Channel class: constructor, getters, setters, membership, operators, invite list, topic, modes, broadcast"
    status: completed
  - id: join-cmd
    content: "Implement JOIN command: validation, mode checks, auto-creation, operator assignment, topic/names"
    status: completed
  - id: part-cmd
    content: "Implement PART command: membership check, broadcast, removal, channel cleanup"
    status: completed
  - id: quit-cmd
    content: "Implement QUIT command: multi-channel broadcast, disconnection"
    status: in_progress
  - id: privmsg-cmd
    content: "Implement PRIVMSG command: target detection (channel vs user), routing, broadcast"
    status: completed
  - id: kick-cmd
    content: "Implement KICK command: operator check, target validation, broadcast, removal"
    status: completed
  - id: invite-cmd
    content: "Implement INVITE command: permission check, invite list management, notifications"
    status: completed
  - id: topic-cmd
    content: "Implement TOPIC command: query vs set, +t mode check, broadcast"
    status: completed
  - id: mode-cmd
    content: "Implement MODE command: one mode per command, parameter parsing, broadcast"
    status: completed
  - id: ping-pong-cmd
    content: "Implement PING and PONG commands: token echo, keep-alive"
    status: completed
isProject: false
---

# Complete IRC Commands Implementation Plan

## Overview

This plan covers the implementation of all remaining IRC commands for Sara (Dev C - Logic Layer), starting with the Channel class and proceeding through channel operations, messaging, and operator commands.

**Already Complete:**

- Client class with Halloy compatibility (case-insensitive nicknames, password retry, strict registration order)
- Utils class (validation, string manipulation)
- PASS, NICK, USER commands (registration flow)

**To Implement:**

- Channel class
- Channel commands (JOIN, PART, QUIT)
- Messaging commands (PRIVMSG, PING, PONG)
- Operator commands (KICK, INVITE, TOPIC, MODE)

---

## Architecture Reference

Following **Buffer Variant 3** architecture where:

- Alex (Network Layer): Handles sockets, uses MessageBuffer to feed Parser
- Artur (Parser Layer): Parses IRC messages into Command structs
- Sara (Logic Layer): Implements Client, Channel, and all command handlers

**Critical Dependencies (Alex must implement first):**

- `Server::getPassword()` - for PASS validation
- `Server::sendToClient(int fd, const std::string& msg)` - for sending responses
- `Server::getClientByNickname(const std::string& nick)` - case-insensitive lookup
- `Server::getChannel(const std::string& name)` - case-insensitive lookup
- `Server::createChannel(const std::string& name)` - auto-create channels
- `Server::removeChannel(const std::string& name)` - cleanup empty channels
- `Server::disconnectClient(int fd)` - for cleanup

---

## Phase 1: Channel Class Implementation

File: [include/irc/Channel.hpp](include/irc/Channel.hpp) and [src/Channel.cpp](src/Channel.cpp)

### 1.1 Private Members

```cpp
private:
    // Identity (case handling for Halloy)
    std::string name_;         // "#test" - lowercase for comparison
    std::string nameDisplay_;  // "#Test" - original case for display
    
    // Topic
    std::string topic_;
    std::string topicSetter_;  // nickname who set it
    time_t topicTime_;         // when it was set
    
    // Membership
    std::map<int, Client*> clients_;    // fd -> Client* (quick lookup)
    std::vector<Client*> operators_;    // operators list
    std::set<std::string> inviteList_;  // lowercase nicknames
    
    // Channel modes
    bool inviteOnly_;      // +i mode
    bool topicProtected_;  // +t mode (only ops can change topic)
    std::string channelKey_;  // +k mode (password)
    int userLimit_;        // +l mode (0 = no limit)
```

### 1.2 Constructor

```cpp
Channel(const std::string& name)
    : nameDisplay_(name)
    , name_(Utils::toLower(name))
    , topicTime_(0)
    , inviteOnly_(false)
    , topicProtected_(false)
    , userLimit_(0)
{}
```

### 1.3 Core Methods

**Identity:**

```cpp
const std::string& getName() const;        // lowercase
const std::string& getNameDisplay() const; // original
```

**Membership:**

```cpp
bool hasClient(Client* client) const;
void addClient(Client* client);
void removeClient(Client* client);
std::vector<Client*> getClients() const;
size_t getClientCount() const;
bool isEmpty() const { return clients_.empty(); }
```

**Operators:**

```cpp
bool isOperator(Client* client) const;
void addOperator(Client* client);
void removeOperator(Client* client);
std::vector<Client*> getOperators() const;
```

**Invite List:**

```cpp
bool isInvited(const std::string& nickname) const;  // case-insensitive
void addToInviteList(const std::string& nickname);
void removeFromInviteList(const std::string& nickname);
```

**Topic:**

```cpp
const std::string& getTopic() const;
void setTopic(const std::string& topic, const std::string& setBy);
const std::string& getTopicSetter() const;
time_t getTopicTime() const;
bool hasTopic() const { return !topic_.empty(); }
```

**Modes:**

```cpp
bool hasMode(char mode) const;  // i, t, k, l
void setMode(char mode, bool value);
std::string getModeString() const;  // "+itk"
const std::string& getChannelKey() const;
void setChannelKey(const std::string& key);
int getUserLimit() const;
void setUserLimit(int limit);
```

**Broadcasting (CRITICAL):**

```cpp
// Signature from TEAM_CONVENTIONS.md section 12:
void broadcast(Server* server, const std::string& message, Client* exclude);
```

Implementation:

```cpp
void Channel::broadcast(Server* server, const std::string& message, 
                        Client* exclude) {
    for (std::map<int, Client*>::iterator it = clients_.begin();
         it != clients_.end(); ++it) {
        if (it->second != exclude) {
            server->sendToClient(it->first, message);
        }
    }
}
```

---

## Phase 2: JOIN Command

File: [src/commands/Join.cpp](src/commands/Join.cpp)

### Logic Flow:

1. **Registration check:**
  ```cpp
   if (!client.isRegistered()) {
       // ERR_NOTREGISTERED (451)
       return;
   }
  ```
2. **Parameter validation:**
  ```cpp
   if (cmd.params.empty() || cmd.getParam(0).empty()) {
       // ERR_NEEDMOREPARAMS (461)
       return;
   }
  ```
3. **Parse channel name and key:**
  ```cpp
   std::string channelName = cmd.getParam(0);
   std::string key = cmd.getParam(1);  // may be empty

   // Validate format
   if (!Utils::isValidChannelName(channelName)) {
       // ERR_BADCHANMASK (476)
       return;
   }
  ```
4. **Get or create channel (case-insensitive):**
  ```cpp
   Channel* channel = server.getChannel(channelName);
   bool isNew = (channel == NULL);

   if (isNew) {
       channel = server.createChannel(channelName);
   }
  ```
5. **Check if already in channel:**
  ```cpp
   if (channel->hasClient(&client)) {
       // Already in, silently ignore or send notice
       return;
   }
  ```
6. **Mode checks:**
  ```cpp
   // +k: key required
   if (channel->hasMode('k')) {
       if (key != channel->getChannelKey()) {
           // ERR_BADCHANNELKEY (475)
           return;
       }
   }

   // +i: invite only
   if (channel->hasMode('i')) {
       if (!channel->isInvited(client.getNickname())) {
           // ERR_INVITEONLYCHAN (473)
           return;
       }
   }

   // +l: user limit
   if (channel->hasMode('l')) {
       if (channel->getClientCount() >= (size_t)channel->getUserLimit()) {
           // ERR_CHANNELISFULL (471)
           return;
       }
   }
  ```
7. **Add client to channel:**
  ```cpp
   channel->addClient(&client);
   client.addChannel(channel->getName());

   // First member becomes operator
   if (isNew || channel->getClientCount() == 1) {
       channel->addOperator(&client);
   }
  ```
8. **Send JOIN confirmation:**
  ```cpp
   std::string joinMsg = ":" + client.getPrefix() + " JOIN :" + 
                         channel->getNameDisplay() + "\r\n";

   // To all members (including joiner)
   server.sendToClient(client.getFd(), joinMsg);
   channel->broadcast(&server, joinMsg, &client);
  ```
9. **Send topic (if exists):**
  ```cpp
   if (channel->hasTopic()) {
       // RPL_TOPIC (332)
       server.sendToClient(fd, Replies::numeric(
           Replies::RPL_TOPIC, nick, channel->getNameDisplay(),
           channel->getTopic()));
   } else {
       // RPL_NOTOPIC (331)
       server.sendToClient(fd, Replies::numeric(
           Replies::RPL_NOTOPIC, nick, channel->getNameDisplay(),
           "No topic is set"));
   }
  ```
10. **Send NAMES list:**
  ```cpp
    std::vector<Client*> members = channel->getClients();
    std::string namesList;

    for (size_t i = 0; i < members.size(); ++i) {
        if (channel->isOperator(members[i])) {
            namesList += "@";
        }
        namesList += members[i]->getNicknameDisplay();
        if (i < members.size() - 1) {
            namesList += " ";
        }
    }

    // RPL_NAMREPLY (353)
    server.sendToClient(fd, Replies::numeric(
        Replies::RPL_NAMREPLY, nick, 
        "= " + channel->getNameDisplay(), namesList));

    // RPL_ENDOFNAMES (366)
    server.sendToClient(fd, Replies::numeric(
        Replies::RPL_ENDOFNAMES, nick, channel->getNameDisplay(),
        "End of /NAMES list"));
  ```

---

## Phase 3: PART Command

File: [src/commands/Part.cpp](src/commands/Part.cpp)

### Logic Flow:

1. **Registration check**
2. **Parameter validation:**
  ```cpp
   if (cmd.params.empty()) {
       // ERR_NEEDMOREPARAMS (461)
       return;
   }
  ```
3. **Get channel:**
  ```cpp
   std::string channelName = cmd.getParam(0);
   Channel* channel = server.getChannel(channelName);

   if (!channel) {
       // ERR_NOSUCHCHANNEL (403)
       return;
   }
  ```
4. **Check membership:**
  ```cpp
   if (!channel->hasClient(&client)) {
       // ERR_NOTONCHANNEL (442)
       return;
   }
  ```
5. **Broadcast PART:**
  ```cpp
   std::string reason = cmd.trailing.empty() ? "Leaving" : cmd.trailing;
   std::string partMsg = ":" + client.getPrefix() + " PART " + 
                         channel->getNameDisplay() + " :" + reason + "\r\n";

   // To all members (including leaver)
   server.sendToClient(client.getFd(), partMsg);
   channel->broadcast(&server, partMsg, &client);
  ```
6. **Remove client:**
  ```cpp
   channel->removeClient(&client);
   client.removeChannel(channel->getName());

   // If channel empty, delete it
   if (channel->isEmpty()) {
       server.removeChannel(channel->getName());
   }
  ```

---

## Phase 4: QUIT Command

File: [src/commands/Quit.cpp](src/commands/Quit.cpp)

### Logic Flow:

1. **Build QUIT message:**
  ```cpp
   std::string reason = cmd.trailing.empty() ? "Leaving" : cmd.trailing;
   std::string quitMsg = ":" + client.getPrefix() + " QUIT :" + 
                         reason + "\r\n";
  ```
2. **Broadcast to all channels:**
  ```cpp
   const std::vector<std::string>& channels = client.getChannels();

   for (size_t i = 0; i < channels.size(); ++i) {
       Channel* chan = server.getChannel(channels[i]);
       if (chan) {
           chan->broadcast(&server, quitMsg, &client);
       }
   }
  ```
3. **Disconnect client (Alex's responsibility):**
  ```cpp
   server.disconnectClient(client.getFd());
   // disconnectClient() will:
   // - Remove from all channels
   // - Remove from clients_ map
   // - Close socket
   // - Delete Client object
  ```

---

## Phase 5: PRIVMSG Command

File: [src/commands/Privmsg.cpp](src/commands/Privmsg.cpp)

### Logic Flow:

1. **Registration check**
2. **Parameter validation:**
  ```cpp
   if (cmd.params.empty() || cmd.trailing.empty()) {
       // ERR_NEEDMOREPARAMS (461)
       return;
   }
  ```
3. **Determine target type:**
  ```cpp
   std::string target = cmd.getParam(0);
   std::string message = cmd.trailing;

   if (Utils::isChannelName(target)) {
       // Channel message
       handleChannelMessage(server, client, target, message);
   } else {
       // Private message
       handlePrivateMessage(server, client, target, message);
   }
  ```
4. **Channel message handler:**
  ```cpp
   void handleChannelMessage(...) {
       Channel* channel = server.getChannel(target);

       if (!channel) {
           // ERR_NOSUCHCHANNEL (403)
           return;
       }

       if (!channel->hasClient(&client)) {
           // ERR_CANNOTSENDTOCHAN (404)
           return;
       }

       // Format and broadcast
       std::string msg = ":" + client.getPrefix() + " PRIVMSG " + 
                         channel->getNameDisplay() + " :" + message + "\r\n";

       channel->broadcast(&server, msg, &client);  // Exclude sender
   }
  ```
5. **Private message handler:**
  ```cpp
   void handlePrivateMessage(...) {
       Client* targetClient = server.getClientByNickname(target);

       if (!targetClient) {
           // ERR_NOSUCHNICK (401)
           return;
       }

       // Format and send
       std::string msg = ":" + client.getPrefix() + " PRIVMSG " + 
                         targetClient->getNicknameDisplay() + " :" + 
                         message + "\r\n";

       server.sendToClient(targetClient->getFd(), msg);
   }
  ```

---

## Phase 6: KICK Command

File: [src/commands/Kick.cpp](src/commands/Kick.cpp)

### Logic Flow:

1. **Registration check**
2. **Parameter validation:**
  ```cpp
   if (cmd.params.size() < 2) {
       // ERR_NEEDMOREPARAMS (461)
       return;
   }
  ```
3. **Get channel and verify membership:**
  ```cpp
   std::string channelName = cmd.getParam(0);
   std::string targetNick = cmd.getParam(1);
   std::string reason = cmd.trailing.empty() ? 
                        client.getNicknameDisplay() : cmd.trailing;

   Channel* channel = server.getChannel(channelName);
   if (!channel) {
       // ERR_NOSUCHCHANNEL (403)
       return;
   }

   if (!channel->hasClient(&client)) {
       // ERR_NOTONCHANNEL (442)
       return;
   }
  ```
4. **Check operator status:**
  ```cpp
   if (!channel->isOperator(&client)) {
       // ERR_CHANOPRIVSNEEDED (482)
       return;
   }
  ```
5. **Get target client:**
  ```cpp
   Client* target = server.getClientByNickname(targetNick);
   if (!target) {
       // ERR_NOSUCHNICK (401)
       return;
   }

   if (!channel->hasClient(target)) {
       // ERR_USERNOTINCHANNEL (441)
       return;
   }
  ```
6. **Broadcast KICK and remove:**
  ```cpp
   std::string kickMsg = ":" + client.getPrefix() + " KICK " + 
                         channel->getNameDisplay() + " " + 
                         target->getNicknameDisplay() + " :" + 
                         reason + "\r\n";

   // To all members (including kicked user)
   server.sendToClient(target->getFd(), kickMsg);
   channel->broadcast(&server, kickMsg, target);

   // Remove from channel
   channel->removeClient(target);
   target->removeChannel(channel->getName());

   // Cleanup if empty
   if (channel->isEmpty()) {
       server.removeChannel(channel->getName());
   }
  ```

---

## Phase 7: INVITE Command

File: [src/commands/Invite.cpp](src/commands/Invite.cpp)

### Logic Flow:

1. **Registration check**
2. **Parameter validation:**
  ```cpp
   if (cmd.params.size() < 2) {
       // ERR_NEEDMOREPARAMS (461)
       return;
   }
  ```
3. **Get target and channel:**
  ```cpp
   std::string targetNick = cmd.getParam(0);
   std::string channelName = cmd.getParam(1);

   Client* target = server.getClientByNickname(targetNick);
   if (!target) {
       // ERR_NOSUCHNICK (401)
       return;
   }

   Channel* channel = server.getChannel(channelName);
   if (!channel) {
       // ERR_NOSUCHCHANNEL (403)
       return;
   }
  ```
4. **Verify inviter is in channel:**
  ```cpp
   if (!channel->hasClient(&client)) {
       // ERR_NOTONCHANNEL (442)
       return;
   }
  ```
5. **Check if target already in channel:**
  ```cpp
   if (channel->hasClient(target)) {
       // ERR_USERONCHANNEL (443)
       return;
   }
  ```
6. **Check operator requirement (only for +i channels):**
  ```cpp
   if (channel->hasMode('i') && !channel->isOperator(&client)) {
       // ERR_CHANOPRIVSNEEDED (482)
       return;
   }
  ```
7. **Add to invite list and notify:**
  ```cpp
   channel->addToInviteList(target->getNickname());

   // RPL_INVITING (341) to inviter
   server.sendToClient(client.getFd(), Replies::numeric(
       Replies::RPL_INVITING, client.getNicknameDisplay(),
       target->getNicknameDisplay() + " " + channel->getNameDisplay(), ""));

   // INVITE message to target
   std::string inviteMsg = ":" + client.getPrefix() + " INVITE " + 
                           target->getNicknameDisplay() + " :" + 
                           channel->getNameDisplay() + "\r\n";
   server.sendToClient(target->getFd(), inviteMsg);
  ```

---

## Phase 8: TOPIC Command

File: [src/commands/Topic.cpp](src/commands/Topic.cpp)

### Logic Flow:

1. **Registration check**
2. **Parameter validation:**
  ```cpp
   if (cmd.params.empty()) {
       // ERR_NEEDMOREPARAMS (461)
       return;
   }
  ```
3. **Get channel:**
  ```cpp
   std::string channelName = cmd.getParam(0);
   Channel* channel = server.getChannel(channelName);

   if (!channel) {
       // ERR_NOSUCHCHANNEL (403)
       return;
   }

   if (!channel->hasClient(&client)) {
       // ERR_NOTONCHANNEL (442)
       return;
   }
  ```
4. **Query vs. Set:**
  ```cpp
   if (cmd.trailing.empty() && cmd.params.size() == 1) {
       // Query topic
       if (channel->hasTopic()) {
           // RPL_TOPIC (332)
           server.sendToClient(fd, Replies::numeric(
               Replies::RPL_TOPIC, nick, channel->getNameDisplay(),
               channel->getTopic()));
       } else {
           // RPL_NOTOPIC (331)
           server.sendToClient(fd, Replies::numeric(
               Replies::RPL_NOTOPIC, nick, channel->getNameDisplay(),
               "No topic is set"));
       }
       return;
   }
  ```
5. **Set topic (with permission check):**
  ```cpp
   // Check +t mode
   if (channel->hasMode('t') && !channel->isOperator(&client)) {
       // ERR_CHANOPRIVSNEEDED (482)
       return;
   }

   std::string newTopic = cmd.trailing;
   channel->setTopic(newTopic, client.getNicknameDisplay());

   // Broadcast TOPIC change
   std::string topicMsg = ":" + client.getPrefix() + " TOPIC " + 
                          channel->getNameDisplay() + " :" + 
                          newTopic + "\r\n";

   server.sendToClient(client.getFd(), topicMsg);
   channel->broadcast(&server, topicMsg, &client);
  ```

---

## Phase 9: MODE Command (SIMPLIFIED)

File: [src/commands/Mode.cpp](src/commands/Mode.cpp)

**IMPORTANT:** Only ONE mode per command per TEAM_CONVENTIONS.md

### Logic Flow:

1. **Registration check**
2. **Parameter validation:**
  ```cpp
   if (cmd.params.empty()) {
       // ERR_NEEDMOREPARAMS (461)
       return;
   }
  ```
3. **Get channel:**
  ```cpp
   std::string channelName = cmd.getParam(0);
   Channel* channel = server.getChannel(channelName);

   if (!channel) {
       // ERR_NOSUCHCHANNEL (403)
       return;
   }

   if (!channel->hasClient(&client)) {
       // ERR_NOTONCHANNEL (442)
       return;
   }
  ```
4. **Query vs. Set:**
  ```cpp
   if (cmd.params.size() == 1) {
       // Query modes
       std::string modes = channel->getModeString();

       // RPL_CHANNELMODEIS (324)
       server.sendToClient(fd, Replies::numeric(
           Replies::RPL_CHANNELMODEIS, nick,
           channel->getNameDisplay() + " " + modes, ""));
       return;
   }
  ```
5. **Set mode (operator required):**
  ```cpp
   if (!channel->isOperator(&client)) {
       // ERR_CHANOPRIVSNEEDED (482)
       return;
   }

   std::string modeStr = cmd.getParam(1);
   if (modeStr.length() < 2) {
       return;  // Invalid format
   }

   char sign = modeStr[0];  // '+' or '-'
   char mode = modeStr[1];  // 'i', 't', 'k', 'o', 'l'

   if (sign != '+' && sign != '-') {
       return;  // Invalid
   }
  ```
6. **Apply mode based on type:**
  ```cpp
   switch (mode) {
       case 'i':  // invite-only
       case 't':  // topic-protected
           channel->setMode(mode, sign == '+');
           break;

       case 'k':  // key
           if (sign == '+') {
               std::string key = cmd.getParam(2);
               if (key.empty()) {
                   // ERR_NEEDMOREPARAMS
                   return;
               }
               channel->setChannelKey(key);
               channel->setMode('k', true);
           } else {
               channel->setChannelKey("");
               channel->setMode('k', false);
           }
           break;

       case 'o':  // operator
           {
               std::string targetNick = cmd.getParam(2);
               if (targetNick.empty()) {
                   // ERR_NEEDMOREPARAMS
                   return;
               }

               Client* target = server.getClientByNickname(targetNick);
               if (!target || !channel->hasClient(target)) {
                   // ERR_NOSUCHNICK or ERR_USERNOTINCHANNEL
                   return;
               }

               if (sign == '+') {
                   channel->addOperator(target);
               } else {
                   channel->removeOperator(target);
               }
           }
           break;

       case 'l':  // user limit
           if (sign == '+') {
               std::string limitStr = cmd.getParam(2);
               if (limitStr.empty()) {
                   // ERR_NEEDMOREPARAMS
                   return;
               }
               int limit = Utils::stringToInt(limitStr);
               channel->setUserLimit(limit);
               channel->setMode('l', true);
           } else {
               channel->setUserLimit(0);
               channel->setMode('l', false);
           }
           break;

       default:
           // ERR_UNKNOWNMODE (472)
           return;
   }
  ```
7. **Broadcast MODE change:**
  ```cpp
   std::string modeMsg = ":" + client.getPrefix() + " MODE " + 
                         channel->getNameDisplay() + " " + modeStr;

   // Add parameter for +k, +o, +l
   if ((mode == 'k' || mode == 'o' || mode == 'l') && sign == '+') {
       modeMsg += " " + cmd.getParam(2);
   }

   modeMsg += "\r\n";

   server.sendToClient(client.getFd(), modeMsg);
   channel->broadcast(&server, modeMsg, &client);
  ```

---

## Phase 10: PING and PONG Commands

Files: [src/commands/Ping.cpp](src/commands/Ping.cpp) and [src/commands/Pong.cpp](src/commands/Pong.cpp)

### PING Logic:

```cpp
void handlePing(Server& server, Client& client, const Command& cmd) {
    std::string token = cmd.params.empty() ? cmd.trailing : cmd.getParam(0);
    
    if (token.empty()) {
        // ERR_NEEDMOREPARAMS
        return;
    }
    
    // Send PONG with same token
    std::string pongMsg = ":" + Replies::formatServerName() + 
                          " PONG " + Replies::formatServerName() + 
                          " :" + token + "\r\n";
    
    server.sendToClient(client.getFd(), pongMsg);
}
```

### PONG Logic:

```cpp
void handlePong(Server& server, Client& client, const Command& cmd) {
    // Simply acknowledge - client responded to our PING
    // Update last activity timestamp if implementing timeout
    (void)server;
    (void)client;
    (void)cmd;
}
```

---

## Implementation Order Summary

1. **Phase 1:** Channel class (foundation)
2. **Phase 2:** JOIN (most complex, tests Channel thoroughly)
3. **Phase 3:** PART (tests removal logic)
4. **Phase 4:** QUIT (tests multi-channel cleanup)
5. **Phase 5:** PRIVMSG (tests messaging)
6. **Phase 6:** KICK (operator privilege check)
7. **Phase 7:** INVITE (invite list management)
8. **Phase 8:** TOPIC (conditional operator check)
9. **Phase 9:** MODE (most complex, one mode at a time)
10. **Phase 10:** PING/PONG (simplest, for testing)

---

## Testing Checklist per Halloy Requirements

After each phase, test:

- Case-insensitive channel names: `#Test` and `#test` are same channel
- Case-insensitive nicknames: `BOB` can't kick `bob` (same user)
- Empty parameter handling in all commands
- Operator privilege checks work correctly
- Broadcast excludes sender correctly
- Channel auto-creation on first JOIN
- First joiner becomes operator
- Empty channel deletion
- Mode +i requires INVITE
- Mode +t protects TOPIC
- Mode +k requires correct key
- Mode +l enforces limit
- Mode +o grants/removes operator status

