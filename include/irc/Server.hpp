#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <map>
#include <csignal>		// sig_atomic_t, signal(), SIGINT
#include "Client.hpp"	// Client*
#include "Poller.hpp"	// poller_
#include "Config.hpp"	// config_.getPort()
#include "irc/MessageBuffer.hpp" //100******* MessageBuffer

// Main server class - manages socket, connections, and I/O
// Coordinates between Poller, Parser, and Command handlers
class	Server {

private:
	int serverSocketFd_;
	Config config_;
	Poller* poller_;

	// Client and channel storage
	std::map<int, Client*> clients_;           // fd -> Client*
	// std::map<std::string, Channel*> channels_; // channel name -> Channel*
	//101******* MessageBuffer
	std::map<int, MessageBuffer*> buffers_;    // fd -> MessageBuffer*

	// Helper methods
	void createServerSocket();
	void bindSocket();
	void listenSocket();
	void setNonBlocking(int fd);
public:
	// Constructor: initialize server with configuration
	Server(const Config& config);

	// Destructor: cleanup resources
	~Server();

	// Start the server: create socket, bind, listen
	void start();

	// Main server loop (calls Poller::poll())
	void run();

	// Handle new client connection (called by Poller)
	void handleNewConnection();

	// Handle incoming data from client (called by Poller)
	void handleClientInput(int clientFd);

	// Handle client disconnection
	void disconnectClient(int clientFd);

	// Send data to a specific client (PRIMARY METHOD - see TEAM_CONVENTIONS.md)
	void sendToClient(int clientFd, const std::string& message);

	// Convenience method for sending formatted IRC response
	void sendResponse(int clientFd, const std::string& numeric,
						const std::string& params, const std::string& trailing);

	// Broadcast message to all clients in a channel
	// void broadcastToChannel(const std::string& channelName, const std::string& message, int excludeFd = -1);

	// Client management
	Client* getClient(int fd);
	// Client* getClientByNickname(const std::string& nickname);
	// void addClient(int fd);
	// void removeClient(int fd);

	/*******5 server::run() SIGINT for Ctrl+C */
	static volatile	sig_atomic_t running_;
	int getServerFd() const;
	Poller* getPoller() const { return poller_; }
	//103******* MessageBuffer
	MessageBuffer* getBuffer(int fd);
};

#endif

