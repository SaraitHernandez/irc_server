# How to implement `Server::broadcastToChannel()`

**File to edit:** `src/Server.cpp`  
**Declaration (already in `Server.hpp`):**
```cpp
void broadcastToChannel(const std::string& channelName,
                        const std::string& message,
                        int excludeFd = -1);
```

---

## What it should do

Send `message` to every client in a channel, optionally skipping one client (identified by `excludeFd`).  
Used for things like:
- `PRIVMSG #channel :hello` → broadcast to all members except sender
- `JOIN` notification → broadcast to all members including the joiner (`excludeFd = -1`)
- `NICK` change → broadcast to all members

---

## How to implement it

`Channel` already has `broadcast()` in [include/irc/Channel.hpp](include/irc/Channel.hpp#L81):
```cpp
void broadcast(Server* server, const std::string& message, Client* exclude);
```

So `Server::broadcastToChannel()` is just a **thin wrapper** around it:

```cpp
void Server::broadcastToChannel(const std::string& channelName,
                                const std::string& message,
                                int excludeFd)
{
    // 1. Find the channel (case-insensitive)
    Channel* channel = getChannel(channelName);
    if (!channel)
        return;  // channel doesn't exist, nothing to do

    // 2. Resolve excludeFd to a Client* (NULL means "exclude nobody")
    Client* exclude = NULL;
    if (excludeFd >= 0)
        exclude = getClient(excludeFd);

    // 3. Delegate to Channel::broadcast()
    channel->broadcast(this, message, exclude);
}
```

**Where to put it:** Add after `sendToClient()` at the bottom of `src/Server.cpp`.

---

## How `Channel::broadcast()` works internally

It iterates over `clients_` map (all members of the channel) and calls `server->sendToClient(fd, message)` for each member that is not `exclude`. So by providing `this` as the `Server*`, the channel can call back into the server to actually send the data.

---

## Who calls `broadcastToChannel()`

Commands call it when they need to send a message to an entire channel.  
Example usage from a command handler:

```cpp
// In Privmsg.cpp — broadcast message to channel
server.broadcastToChannel(channelName, ":" + client.getPrefix() + " PRIVMSG " + channelName + " :" + text, client.getFd());
```

```cpp
// In Nick.cpp — notify all channels of nick change
const std::vector<std::string>& channels = client.getChannels();
for (size_t i = 0; i < channels.size(); ++i) {
    server.broadcastToChannel(channels[i], ":" + oldPrefix + " NICK :" + newNick, -1);
}
```

---

## Summary

| Step | What to do |
|------|-----------|
| 1 | Copy the implementation block above into `src/Server.cpp` |
| 2 | Place it after `sendToClient()` at the bottom of the file |
| 3 | No changes needed to `Server.hpp` — declaration already there |
| 4 | No new includes needed — `Channel.hpp` and `Client.hpp` already included |
