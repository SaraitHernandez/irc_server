// PART command handler
// Client leaves a channel
// Format: PART <channel>{,<channel>} [<reason>]

#include "irc/Server.hpp"
#include "irc/Client.hpp"
#include "irc/Channel.hpp"
#include "irc/Command.hpp"
#include "irc/Replies.hpp"
#include "irc/commands/Part.hpp"

// TODO: Implement handlePart(Server& server, Client& client, const Command& cmd)
// Function signature: void handlePart(Server& server, Client& client, const Command& cmd)
//
// Logic:
// 1. Check if client is registered -> send ERR_NOTREGISTERED
// 2. Check if channel parameter is provided -> send ERR_NEEDMOREPARAMS
// 3. Handle multiple channels (comma-separated)
// 4. For each channel:
//    - Get channel -> send ERR_NOSUCHCHANNEL if not found
//    - Check if client is in channel -> send ERR_NOTONCHANNEL if not
//    - Broadcast PART message: ":nick!user@host PART #channel :reason"
//    - Remove client from channel
//    - If channel becomes empty, delete channel
//    - Remove client from channel's operator list if present
//    - Send PART confirmation to client

