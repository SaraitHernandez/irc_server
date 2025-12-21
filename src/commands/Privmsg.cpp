// PRIVMSG command handler
// Sends a message to a channel or user
// Format: PRIVMSG <target>{,<target>} :<message>

#include "irc/Server.hpp"
#include "irc/Client.hpp"
#include "irc/Channel.hpp"
#include "irc/Command.hpp"
#include "irc/Replies.hpp"
#include "irc/Utils.hpp"
#include "irc/commands/Privmsg.hpp"

// TODO: Implement handlePrivmsg(Server& server, Client& client, const Command& cmd)
// Function signature: void handlePrivmsg(Server& server, Client& client, const Command& cmd)
//
// Logic:
// 1. Check if client is registered -> send ERR_NOTREGISTERED
// 2. Check if target and message are provided -> send ERR_NEEDMOREPARAMS
// 3. Handle multiple targets (comma-separated)
// 4. For each target:
//    - If target is channel (starts with # or &):
//      * Get channel -> send ERR_NOSUCHCHANNEL if not found
//      * Check if client is in channel -> send ERR_CANNOTSENDTOCHAN if not
//      * Format message: ":nick!user@host PRIVMSG #channel :message"
//      * Broadcast to all channel members (excluding sender)
//    - If target is user (nickname):
//      * Get client by nickname -> send ERR_NOSUCHNICK if not found
//      * Format message: ":nick!user@host PRIVMSG targetnick :message"
//      * Send to target user

