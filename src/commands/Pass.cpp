// PASS command handler
// Sets password for client connection
// Format: PASS <password>

#include "irc/Server.hpp"
#include "irc/Client.hpp"
#include "irc/Command.hpp"
#include "irc/Replies.hpp"
#include "irc/commands/Pass.hpp"

// TODO: Implement handlePass(Server& server, Client& client, const Command& cmd)
// Function signature: void handlePass(Server& server, Client& client, const Command& cmd)
//
// Logic:
// 1. Check if client is already registered -> send ERR_ALREADYREGISTRED
// 2. Check if password parameter is provided -> send ERR_NEEDMOREPARAMS
// 3. Get server password from Config
// 4. Compare provided password with server password
// 5. If match: set password on client using client.setPassword()
// 6. If no match: send ERR_PASSWDMISMATCH (or just ignore, depends on spec)
// 7. After PASS is set, check if client can register (has NICK and USER)
//    - If yes, call client.registerClient() and send welcome messages

