// TOPIC command handler
// Sets or gets channel topic
// Format: TOPIC <channel> [<topic>]

#include "irc/Server.hpp"
#include "irc/Client.hpp"
#include "irc/Channel.hpp"
#include "irc/Command.hpp"
#include "irc/Replies.hpp"
#include "irc/commands/Topic.hpp"

// TODO: Implement handleTopic(Server& server, Client& client, const Command& cmd)
// Function signature: void handleTopic(Server& server, Client& client, const Command& cmd)
//
// Logic:
// 1. Check if client is registered -> send ERR_NOTREGISTERED
// 2. Check if channel parameter is provided -> send ERR_NEEDMOREPARAMS
// 3. Get channel -> send ERR_NOSUCHCHANNEL if not found
// 4. Check if client is in channel -> send ERR_NOTONCHANNEL if not
// 5. If no topic provided (GET):
//    - If channel has topic: send RPL_TOPIC and RPL_TOPICWHOTIME
//    - If channel has no topic: send RPL_NOTOPIC
// 6. If topic provided (SET):
//    - If channel mode 't' (topic-protected): check if client is operator
//      -> send ERR_CHANOPRIVSNEEDED if not operator
//    - Set topic using channel.setTopic()
//    - Broadcast TOPIC change: ":nick!user@host TOPIC #channel :newtopic"

