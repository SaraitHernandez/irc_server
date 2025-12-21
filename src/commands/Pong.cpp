// PONG command handler
// Client responds to server's PING with PONG
// Format: PONG <server> [<server2>]

#include "irc/Server.hpp"
#include "irc/Client.hpp"
#include "irc/Command.hpp"
#include "irc/Replies.hpp"
#include "irc/commands/Pong.hpp"

// TODO: Implement handlePong(Server& server, Client& client, const Command& cmd)
// Function signature: void handlePong(Server& server, Client& client, const Command& cmd)
//
// Logic:
// 1. Get server name from parameter (or from trailing)
// 2. Update client's last activity timestamp (if tracking)
// 3. PONG is acknowledgment of PING - typically no response needed
// 4. If server sent PING and client responds with PONG, connection is alive
// 5. Can be used to reset timeout counter

