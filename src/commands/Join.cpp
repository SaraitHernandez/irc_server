// JOIN command handler
// Client joins a channel
// Format: JOIN <channel>{,<channel>} [<key>{,<key>}]

#include "irc/Server.hpp"
#include "irc/Client.hpp"
#include "irc/Channel.hpp"
#include "irc/Command.hpp"
#include "irc/Replies.hpp"
#include "irc/Utils.hpp"
#include "irc/commands/Join.hpp"

// TODO: Implement handleJoin(Server& server, Client& client, const Command& cmd)
// Function signature: void handleJoin(Server& server, Client& client, const Command& cmd)
//
// Logic:
// 1. Check if client is registered -> send ERR_NOTREGISTERED
// 2. Check if channel parameter is provided -> send ERR_NEEDMOREPARAMS
// 3. Validate channel name using Utils::isValidChannelName() -> send ERR_NOSUCHCHANNEL
// 4. Handle multiple channels (comma-separated)
// 5. For each channel:
//    - Get or create channel
//    - Check if channel has key (mode 'k') -> send ERR_BADCHANNELKEY if wrong
//    - Check if channel is invite-only (mode 'i') -> send ERR_INVITEONLYCHAN if not invited
//    - Check if channel is full (mode 'l') -> send ERR_CHANNELISFULL
//    - Add client to channel
//    - If first member, make client operator
//    - Send JOIN confirmation to client: ":nick!user@host JOIN :#channel"
//    - Send channel topic if exists (RPL_TOPIC)
//    - Send channel member list (RPL_NAMREPLY, RPL_ENDOFNAMES)
//    - Broadcast JOIN to all channel members

