// PING command handler
// Server sends PING to check if client is alive
// Client responds with PONG
// Format: PING <server1> [<server2>]

#include "irc/Server.hpp"
#include "irc/Client.hpp"
#include "irc/Command.hpp"
#include "irc/Replies.hpp"
#include "irc/commands/Ping.hpp"

// TODO: Implement handlePing(Server& server, Client& client, const Command& cmd)
// Function signature: void handlePing(Server& server, Client& client, const Command& cmd)
//
// Logic:
// 1. Check if server parameter is provided -> send ERR_NEEDMOREPARAMS
// 2. Get server name from parameter (or from trailing)
// 3. Send PONG response: "PONG :servername"
//    Format: "PONG <server> [<server2>]"
//    Usually echo back what was sent: "PONG :<server>"
// 4. PING/PONG is used for keep-alive - no error needed if client not registered

