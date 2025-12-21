// NICK command handler
// Sets or changes client nickname
// Format: NICK <nickname>

#include "irc/Server.hpp"
#include "irc/Client.hpp"
#include "irc/Command.hpp"
#include "irc/Replies.hpp"
#include "irc/Utils.hpp"
#include "irc/commands/Nick.hpp"

// TODO: Implement handleNick(Server& server, Client& client, const Command& cmd)
// Function signature: void handleNick(Server& server, Client& client, const Command& cmd)
//
// Logic:
// 1. Check if nickname parameter is provided -> send ERR_NONICKNAMEGIVEN
// 2. Validate nickname format using Utils::isValidNickname() -> send ERR_ERRONEUSNICKNAME
// 3. Check if nickname is already in use by another client -> send ERR_NICKNAMEINUSE
// 4. If client already has nickname, notify old channels of nickname change
// 5. Set new nickname using client.setNickname()
// 6. If this is first NICK and client has PASS and USER, register client
// 7. Send welcome messages if just registered
// 8. If changing nickname, broadcast to all channels: ":oldnick!user@host NICK :newnick"

