// QUIT command handler
// Disconnects client from server
// Format: QUIT [<reason>]

#include "irc/Server.hpp"
#include "irc/Client.hpp"
#include "irc/Channel.hpp"
#include "irc/Command.hpp"
#include "irc/Replies.hpp"
#include "irc/commands/Quit.hpp"

// TODO: Implement handleQuit(Server& server, Client& client, const Command& cmd)
// Function signature: void handleQuit(Server& server, Client& client, const Command& cmd)
//
// Logic:
// 1. Get quit reason from trailing (default: "Client quit")
// 2. Get all channels client is in
// 3. For each channel:
//    - Broadcast QUIT message: ":nick!user@host QUIT :reason"
//    - Remove client from channel
//    - Remove client from channel's operator list if present
//    - Delete channel if it becomes empty (optional)
// 4. Remove client from server's client list
// 5. Close client socket (or mark for disconnection, let Server handle it)
// 6. Note: Server should call disconnectClient() to clean up

