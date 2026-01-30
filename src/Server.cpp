// Server implementation
// Manages server socket, client connections, and coordinates I/O

#include "irc/Server.hpp"
#include "irc/Poller.hpp"
#include "irc/Client.hpp"
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


/*******5 SIGINT handler*/
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
		close(it->first);
		delete it->second;
	}
	if (serverSocketFd_ >= 0) {
		close(serverSocketFd_); // Server Socket here:)
	}
	delete poller_;
	std::cout << "[Server] Destroyed" << std::endl; // Debug message to see lifecycle
}

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

/*******5 SIGINT handler*/
static void	signalHandler(int signal) {
	if (signal == SIGINT) {
		std::cout << "\n[Server] Received SIGINT, shutting down..." << std::endl;
		Server::running_ = false;
	}
}

// DONE: Implement Server::run()
// - Main event loop:
//   while (running) {
//       poller.poll();
//       poller.processEvents();
//   }
void	Server::run() {
	/*******5 SIGINT handler*/
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);

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

// DONE: createServerSocket(): socket(), set socket options
void Server::createServerSocket() {
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

// DONE: setNonBlocking(int fd): fcntl() with O_NONBLOCK
void	Server::setNonBlocking(int fd) {
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0)
	{  // ← error check *******4
		std::cerr << "[Server] fcntl(F_GETFL) failed for fd=" << fd 
					<< ": " << strerror(errno) << std::endl;
		return;
	}
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
	{  // ← error check *******4
		std::cerr << "[Server] fcntl(F_SETFL) failed for fd=" << fd 
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

// Stub implementations for Network phase
// DOING: Accept new connection, create Client, add to Poller
void	Server::handleNewConnection() {
	struct sockaddr_in clientAddr;
	socklen_t clientLen = sizeof(clientAddr);

	// accept new connection
	int clientFd = accept(serverSocketFd_, 
							(struct sockaddr*)&clientAddr, 
							&clientLen);

	if (clientFd < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return; // there are no connections
		}
		std::cerr << "[Server] accept() failed: " 
					<< strerror(errno) << std::endl;
		return;
	}

	// set non blocking mode
	setNonBlocking(clientFd);

	// add to Poller
	poller_->addFd(clientFd, POLLIN);

	std::cout << "[Server] New connection fd=" << clientFd << std::endl;
}


// DOING: Read data, parse messages, execute commands
void	Server::handleClientInput(int fd) {
	std::cout << "\n=== DEBUG MSG: handleClientInput START fd=" << fd << " ===" << std::endl;

	char	buffer[512];
	ssize_t bytesRead = recv(fd, buffer, sizeof(buffer) - 1, 0);

	std::cout << "[Server] recv() returned: " << bytesRead << std::endl;
	std::cout << "[Server] errno: " << errno 
				<< " (" << strerror(errno) << ")" << std::endl;

	if (bytesRead < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			std::cout << "[Server] EAGAIN/EWOULDBLOCK - no data yet" << std::endl;
			return; // no data yet
		}
		std::cerr << "[Server] recv() error fd=" << fd << std::endl;
		disconnectClient(fd);
		return;
	}

	if (bytesRead == 0) {
		// client closed connection
		std::cout << "[Server] Client disconnected fd=" << fd << std::endl;
		disconnectClient(fd);
		std::cout << "=== handleClientInput DEBUG MSG: (disconnected 'Ctrl+C') ===" << std::endl;
		return;
	}
	//data received
	std::cout << "[Server] Case: bytesRead > 0 (" << bytesRead << " bytes)" << std::endl;
	// debug: output of data received
	buffer[bytesRead] = '\0';
	std::cout << "[Server] Received from fd=" << fd 
				<< ": " << buffer << std::endl;
	std::cout << "=== DEBUG MSG: handleClientInput ===" << std::endl;
}

// DOING: Remove client from all channels, close socket, delete Client
void	Server::disconnectClient(int fd) {
	std::cout << "[Server] Disconnecting fd=" << fd << std::endl;
	// remove from Poller
	poller_->removeFd(fd);
	// close socket
	close(fd);
	// TODO: remove Client object from clients_
}

