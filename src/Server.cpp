// Server implementation
// Manages server socket, client connections, and coordinates I/O

#include "irc/Server.hpp"
#include "irc/Poller.hpp"
#include "irc/Client.hpp"
#include "irc/Channel.hpp"
#include "irc/Utils.hpp"
#include "irc/MessageBuffer.hpp"
#include "irc/Config.hpp"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <cerrno>	// errno, strerror() — for errors recv/accept
#include <csignal> // sig_atomic_t, signal(), SIGINT
#include <arpa/inet.h>  // inet_ntoa()

// SIGINT handler
volatile	sig_atomic_t Server::running_ = true;

// DONE: Implement Server::Server(const Config& config)
// - Store config
// - Initialize clients_ map
Server::Server(const Config& config)
	: serverSocketFd_(-1), config_(config), poller_(NULL) {
	std::cout << "[Server] Created with port=" << config.getPort() << std::endl;
}

// DONE: Implement Server::~Server()
// - Close server socket
// - Delete all clients

Server::~Server() {
    for (std::map<int, Client*>::iterator it = clients_.begin();
            it != clients_.end(); ++it) {
        close(it->first);	//closing fd OS Level
        delete it->second;	//deletes object Client frees memory
    }
    for (std::map<int, MessageBuffer*>::iterator it = buffers_.begin();
            it != buffers_.end(); ++it) {
        delete it->second;
    }	//cleans buffer of messages DYNAMICALLY ALLOCATED WITH new
    for (std::map<std::string, Channel*>::iterator it = channels_.begin();
            it != channels_.end(); ++it) {
        delete it->second;
    }	//cleans channels chat rooms 
    if (serverSocketFd_ >= 0) {
        close(serverSocketFd_);
    }				//DYNAMICALLY ALLOCATED poller_
    delete poller_;	//monitors events on file descriptor closed here
}


// Server::~Server() {
// 	for (std::map<int, Client*>::iterator it = clients_.begin();
// 			it != clients_.end(); ++it) {
// 		close(it->first);
// 		delete it->second;
// 	}
// 	if (serverSocketFd_ >= 0) {
// 		close(serverSocketFd_); // Server Socket here:)
// 	}
// 	delete poller_;
// }

// - Delete all channels (logic Layer)

// DONE: Implement Server::start()
// - Call createServerSocket()
// - Call bindSocket()
// - Call listenSocket()
// - Add server socket to Poller
// - Set server socket to non-blocking
void	Server::start() {
	createServerSocket();
	bindSocket();
	listenSocket();
	setNonBlocking(serverSocketFd_);

	poller_ = new Poller(this);
	poller_->addFd(serverSocketFd_, POLLIN);

	std::cout << "[Server] Listening on port " << config_.getPort() << std::endl;
}

//SIGINT handler
static void	signalHandler(int sig) {
	(void)sig;
	std::cout << "\n[Server] Signal received, shutting down..." << std::endl;
		Server::running_ = false; // works for SIGINT, SIGTERM, SIGQUIT
}

// DONE: Implement Server::run()
// - Main event loop:
//   while (running) {
//       poller.poll();
//       poller.processEvents();
//   }
void	Server::run() {
	//SIGINT handler
	signal(SIGINT, signalHandler); // Ctrl+C
	signal(SIGTERM, signalHandler);// kill <pid>
	signal(SIGQUIT, signalHandler);// Ctrl+\ — prevent core dump
	signal(SIGPIPE, SIG_IGN);      // prevent crash on broken client send()
	std::cout << "[Server] Running event loop..." << std::endl;

	while (running_) {
		int ready = poller_->poll(1000);  // 1 sec timeout
		if (ready > 0) {
			poller_->processEvents();
		}
	}
	std::cout << "[Server] Event loop stopped" << std::endl;
}

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

// TODO: Implement Server::(int clientFd, const std::string& message)
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

// DONE: createServerSocket(): socket(), set socket options
void	Server::createServerSocket() {
	serverSocketFd_ = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocketFd_ < 0) throw std::runtime_error("socket failed");

	int opt = 1;
	setsockopt(serverSocketFd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}

// DONE: bindSocket(): bind() with config port
void Server::bindSocket() {
	struct sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(config_.getPort());
	addr.sin_addr.s_addr = INADDR_ANY;  // 0.0.0.0

	if (bind(serverSocketFd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		close(serverSocketFd_);
		throw std::runtime_error("bind failed");
	}
	std::cout << "[Server] Bound to port " << config_.getPort() << std::endl;
}

// DONE: listenSocket(): listen() with backlog
void Server::listenSocket() {
	if (listen(serverSocketFd_, 10) < 0) {
		close(serverSocketFd_);
		throw std::runtime_error("listen failed");
	}

	std::cout << "[Server] Listening backlog=10" << std::endl;
}

// DONE: setNonBlocking(int fd): fcntl() with O_NONBLOCK (without F_GETFL)
void	Server::setNonBlocking(int fd) {
	if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
	{
		std::cerr << "[Server] fcntl(F_SETFL, O_NONBLOCK) failed for fd=" << fd
					<< ": " << strerror(errno) << std::endl;
	}
}

// DONE: getServerFd() for Poller::processEvents()
int	Server::getServerFd() const {
	return serverSocketFd_;
}

// DONE: getClient(int fd)
Client*	Server::getClient(int fd) {
	std::map<int, Client*>::iterator it = clients_.find(fd);
	return (it != clients_.end()) ? it->second : NULL;
}

// DONE: getBuffer(int fd)
MessageBuffer* Server::getBuffer(int fd) {
	std::map<int, MessageBuffer*>::iterator it = buffers_.find(fd);
	return (it != buffers_.end()) ? it->second : NULL;
}

std::string Server::getPassword() const {
return config_.getPassword();
}


// DONE: Accept new connection, create Client, add to Poller
void Server::handleNewConnection()
{
	struct sockaddr_in clientAddr;
	socklen_t clientLen = sizeof(clientAddr);

	int clientFd = accept(serverSocketFd_,
							(struct sockaddr*)&clientAddr,
							&clientLen);
	if (clientFd < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return;
		std::cerr << "[Server] accept() failed: " << strerror(errno) << std::endl;
		return;
	}

	setNonBlocking(clientFd);

	// get real client IP and set as hostname
	std::string hostname = inet_ntoa(clientAddr.sin_addr);

	Client* client = new Client(clientFd);
	client->setHostname(hostname);
	clients_[clientFd] = client;

	MessageBuffer* buffer = new MessageBuffer();
	buffers_[clientFd] = buffer;

	poller_->addFd(clientFd, POLLIN);

	std::cout << "[Server] New connection fd=" << clientFd
				<< " from " << hostname << std::endl;
}

// DOING: Read data, parse messages, execute commands
void	Server::handleClientInput(int fd)
{
	char	buffer[4096];
	ssize_t bytesRead = recv(fd, buffer, sizeof(buffer) - 1, 0);

	std::cout << "[Server] recv() returned: " << bytesRead << std::endl;

	if (bytesRead < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
		{
			std::cout << "[Server] EAGAIN/EWOULDBLOCK - no data yet" << std::endl;
			return; // no data yet
		}
		std::cerr << "[Server] recv() error fd=" << fd << std::endl;
		disconnectClient(fd);
		return;
	}
	if (bytesRead == 0)
	{
		// client closed connection
		std::cout << "[Server] Client disconnected fd=" << fd << std::endl;
		disconnectClient(fd);
		return;
	}
	// data received
	std::cout << "[Server] Received " << bytesRead << " bytes from fd=" << fd << std::endl;

	// Get client and buffer
	Client* client = getClient(fd);
	MessageBuffer* msgBuffer = getBuffer(fd);

	if (!client || !msgBuffer)
	{
		std::cerr << "[Server] ERROR: client or buffer not found for fd=" << fd << std::endl;
		return;
	}

	// Append to MessageBuffer
	msgBuffer->append(std::string(buffer, bytesRead));

	// Extract complete messages
	std::vector<std::string> messages = msgBuffer->extractMessages();

	// Process each complete message
	for (size_t i = 0; i < messages.size(); ++i)
	{
		std::cout << "[Server] Complete message: " << messages[i] << std::endl;
		
		// TODO (Issue 1.3): Parser integration
		Command cmd;
		if (parser_.parse(messages[i], cmd))
			registry_.execute(*this, *client, cmd);
	}
}

// DONE:Remove client from all channels, close socket, delete Client
void	Server::disconnectClient(int fd) {
	std::cout << "[Server] Disconnecting fd=" << fd << std::endl;

	// 1) find client
	Client* client = getClient(fd);
	if (!client) {
		std::cerr << "[Server] ERROR: client fd=" << fd << " not found!" << std::endl;
		// clean socket anyway
		poller_->removeFd(fd);
		close(fd);
		return;
	}


	// 2) Remove from channels (Dev C - Logic Layer)
	std::vector<std::string> channels = client->getChannels();
	for (size_t i = 0; i < channels.size(); ++i) {
		Channel* chan = getChannel(channels[i]);
		if (chan) {
			chan->removeClient(client);
			if (chan->isEmpty()) {
				removeChannel(channels[i]);
			}
		}
	}

	// 3) remove from clients_ map
	clients_.erase(fd);

	// 3.5) remove MessageBuffer
	MessageBuffer* buffer = getBuffer(fd);
	if (buffer) {
		delete buffer;
		buffers_.erase(fd);
	}

	// 4) remove from Poller
	poller_->removeFd(fd);

	// 5) close socket
	close(fd);

	// 6) clean memory
	delete client;

	std::cout << "[Server] Client fd=" << fd << " disconnected and cleaned up" << std::endl;
}

// ============================================================================
// Client management
// ============================================================================

Client* Server::getClientByNickname(const std::string& nickname) {
	std::string lowerNick = Utils::toLower(nickname);
	std::map<int, Client*>::iterator it;
	for (it = clients_.begin(); it != clients_.end(); ++it) {
		if (it->second->getNickname() == lowerNick)
			return it->second;
	}
	return NULL;
}

void	Server::sendResponse(int clientFd, const std::string& numeric,
							const std::string& params, const std::string& trailing)
{
	std::string msg = ":" + config_.getServerName() + " " + numeric
					+ " " + params + " :" + trailing;
	sendToClient(clientFd, msg);
}

// ============================================================================
// Channel management
// ============================================================================

// Find channel by name (case-insensitive). Returns NULL if not found.
Channel* Server::getChannel(const std::string& name) {
	std::string lower = Utils::toLower(name);
	std::map<std::string, Channel*>::iterator it = channels_.find(lower);
	if (it != channels_.end())
		return it->second;
	return NULL;
}

// Create and store a new channel. Returns existing channel if it already exists.
Channel* Server::createChannel(const std::string& name) {
	Channel* existing = getChannel(name);
	if (existing)
		return existing;
	std::string lower = Utils::toLower(name);
	Channel* channel = new Channel(name);
	channels_[lower] = channel;
	std::cout << "[Server] Created channel: " << name << std::endl;
	return channel;
}

// Remove channel if empty. Does nothing if channel not found or not empty.
void Server::removeChannel(const std::string& name) {
	std::string lower = Utils::toLower(name);
	std::map<std::string, Channel*>::iterator it = channels_.find(lower);
	if (it == channels_.end())
		return;
	if (it->second->isEmpty()) {
		std::cout << "[Server] Removing empty channel: " << name << std::endl;
		delete it->second;
		channels_.erase(it);
	}
}

// ============================================================================
// sendToClient
// ============================================================================

void Server::sendToClient(int clientFd, const std::string& message)
{
	// Ensure message ends with \r\n
	std::string msg = message;
	if (msg.size() < 2 || msg.substr(msg.size() - 2) != "\r\n")
		msg += "\r\n";

	ssize_t sent = send(clientFd, msg.c_str(), msg.size(), 0);
	if (sent < 0)
	{
		if (errno == EPIPE || errno == ECONNRESET)
			disconnectClient(clientFd);
		else if (errno != EAGAIN && errno != EWOULDBLOCK)
			std::cerr << "[Server] send() error fd=" << clientFd
						<< ": " << strerror(errno) << std::endl;
	}
}

void Server::broadcastToChannel(const std::string& channelName,
                                const std::string& message,
                                int excludeFd)
{
	// 1. Find the channel (case-insensitive)
	Channel* channel = getChannel(channelName);
	if (!channel)
		return;  // channel doesn't exist, nothing to do

	// 2. Resolve excludeFd to a Client* (NULL means "exclude nobody")
	Client* exclude = NULL;
	if (excludeFd >= 0)
		exclude = getClient(excludeFd);

	// 3. Delegate to Channel::broadcast()
	channel->broadcast(this, message, exclude);
}
