// CommandRegistry implementation
// Registers and routes IRC commands to their handler functions

#include "irc/CommandRegistry.hpp"
#include "irc/Server.hpp"
#include "irc/Client.hpp"
#include "irc/Command.hpp"
#include "irc/Replies.hpp"
#include "irc/commands/Pass.hpp"
#include "irc/commands/Nick.hpp"
#include "irc/commands/User.hpp"
#include "irc/commands/Join.hpp"
#include "irc/commands/Part.hpp"
#include "irc/commands/Privmsg.hpp"
#include "irc/commands/Invite.hpp"
#include "irc/commands/Kick.hpp"
#include "irc/commands/Topic.hpp"
#include "irc/commands/Mode.hpp"
#include "irc/commands/Quit.hpp"
#include "irc/commands/Ping.hpp"
#include "irc/commands/Pong.hpp"
#include <cctype>

// TODO: Implement CommandRegistry::CommandRegistry()
// - Call initializeHandlers() to register all commands

// TODO: Implement CommandRegistry::~CommandRegistry()
// - Cleanup if needed

// TODO: Implement CommandRegistry::execute(Server& server, Client& client, const Command& cmd)
// - Convert command.command to uppercase
// - Look up handler in handlers_ map
// - If found, call handler(server, client, cmd)
// - If not found, send ERR_UNKNOWNCOMMAND
// - Return true if executed, false if not found

// TODO: Implement CommandRegistry::registerCommand(const std::string& command, CommandHandler handler)
// - Convert command to uppercase
// - Store in handlers_ map

// TODO: Implement CommandRegistry::hasCommand(const std::string& command) const
// - Convert to uppercase
// - Check if exists in handlers_ map

// TODO: Implement CommandRegistry::initializeHandlers()
// Register all command handlers:
// - registerCommand("PASS", handlePass);
// - registerCommand("NICK", handleNick);
// - registerCommand("USER", handleUser);
// - registerCommand("JOIN", handleJoin);
// - registerCommand("PART", handlePart);
// - registerCommand("PRIVMSG", handlePrivmsg);
// - registerCommand("NOTICE", handleNotice);  // Similar to PRIVMSG
// - registerCommand("INVITE", handleInvite);
// - registerCommand("KICK", handleKick);
// - registerCommand("TOPIC", handleTopic);
// - registerCommand("MODE", handleMode);
// - registerCommand("QUIT", handleQuit);
// - registerCommand("PING", handlePing);
// - registerCommand("PONG", handlePong);
// - Optional: WHO, WHOIS (low priority)

