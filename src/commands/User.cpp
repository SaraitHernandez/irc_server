// USER command handler
// Sets username and realname for client
// Format: USER <username> <mode> <unused> <realname>

#include "irc/Server.hpp"
#include "irc/Client.hpp"
#include "irc/Command.hpp"
#include "irc/Replies.hpp"
#include "irc/commands/User.hpp"

// TODO: Implement handleUser(Server& server, Client& client, const Command& cmd)
// Function signature: void handleUser(Server& server, Client& client, const Command& cmd)
//
// Logic:
// 1. Check if client is already registered -> send ERR_ALREADYREGISTRED
// 2. Check if enough parameters (username, mode, unused, realname) -> send ERR_NEEDMOREPARAMS
// 3. Get username from params[0]
// 4. Get realname from trailing (or params[3] if no trailing)
// 5. Set username and realname using client.setUsername()
// 6. If client has PASS and NICK, register client
// 7. Send welcome messages if just registered (RPL_WELCOME, RPL_YOURHOST, RPL_CREATED, RPL_MYINFO)

