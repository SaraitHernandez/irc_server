// Server implementation
// Manages server socket, client connections, and coordinates I/O

#include "irc/Server.hpp"
#include "irc/Poller.hpp"
#include "irc/Parser.hpp"
#include "irc/CommandRegistry.hpp"
#include "irc/Client.hpp"
#include "irc/Channel.hpp"
#include "irc/MessageBuffer.hpp"
#include "irc/Config.hpp"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

// TODO: Implement Server::Server(const Config& config)
// - Store config
// - Initialize clients_ and channels_ maps
// - Create Poller instance

// TODO: Implement Server::~Server()
// - Close server socket
// - Delete all clients
// - Delete all channels

// TODO: Implement Server::start()
// - Call createServerSocket()
// - Call bindSocket()
// - Call listenSocket()
// - Add server socket to Poller
// - Set server socket to non-blocking

// TODO: Implement Server::run()
// - Main event loop:
//   while (running) {
//       poller.poll();
//       poller.processEvents();
//   }

// TODO: Implement Server::handleNewConnection()
// - Accept new connection using accept()
// - Set socket to non-blocking
// - Create new Client object
// - Add client to clients_ map
// - Add client fd to Poller

// TODO: Implement Server::handleClientInput(int clientFd)
// - Read data from socket (use recv())
// - Append data to client's MessageBuffer
// - Extract complete messages from buffer
// - For each complete message:
//     - Parse message using Parser
//     - Execute command using CommandRegistry
// - Handle errors (disconnect on error)

// TODO: Implement Server::disconnectClient(int clientFd)
// - Remove client from all channels
// - Remove client from clients_ map
// - Remove fd from Poller
// - Close socket
// - Delete Client object

// TODO: Implement Server::sendToClient(int clientFd, const std::string& message)
// PRIMARY METHOD FOR SENDING DATA - see TEAM_CONVENTIONS.md
// - Find client by fd
// - Use send() to write message to socket
// - Handle errors (EPIPE, EAGAIN, etc.)
// - Add \r\n if not already present?

// TODO: Implement Server::sendResponse(int clientFd, ...)
// - Format message using Replies class
// - Call sendToClient()

// TODO: Implement Server::broadcastToChannel(...)
// - Get channel by name
// - Call channel->broadcast()

// TODO: Implement Client management methods
// - getClient(int fd)
// - getClientByNickname(const std::string& nickname)
// - addClient(int fd)
// - removeClient(int fd)

// TODO: Implement Channel management methods
// - getChannel(const std::string& name)
// - createChannel(const std::string& name)
// - removeChannel(const std::string& name)

// TODO: Implement helper methods
// - createServerSocket(): socket(), set socket options
// - bindSocket(): bind() with config port
// - listenSocket(): listen() with backlog
// - setNonBlocking(int fd): fcntl() with O_NONBLOCK

