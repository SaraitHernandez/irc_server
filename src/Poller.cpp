// Poller implementation
// ONLY class that calls poll() system call - see TEAM_CONVENTIONS.md
// Manages file descriptor polling and event notification

#include "irc/Poller.hpp"
#include "irc/Server.hpp"
#include <algorithm>
#include <poll.h>
#include <iostream>
#include <cerrno>
#include <cstring>

// DONE: Implement Poller::Poller(Server* server)
// - Store server pointer
// - Initialize pollfds_ vector
Poller::Poller(Server* server) : server_(server) {
	std::cout << "[Poller] Initialized" << std::endl;
}

// DONE: Implement Poller::~Poller()
// - Cleanup if needed
Poller::~Poller() {
	std::cout << "[Poller] Destroyed" << std::endl;
}

// DONE: Implement Poller::addFd(int fd, short events)
// - Create pollfd structure
// - Add to pollfds_ vector
// - Set fd, events, and revents (initially 0)
void	Poller::addFd(int fd, short events) {
	struct pollfd pfd;
	pfd.fd = fd;
	pfd.events = events;
	pfd.revents = 0;
	pollfds_.push_back(pfd);

    std::cout << "[Poller] Added fd=" << fd << " events=0x" << std::hex << events << std::dec << std::endl;
}

// DONE: Implement Poller::removeFd(int fd)
// - Find fd in pollfds_ vector
// - Remove from vector
// - Update indices if needed
void	Poller::removeFd(int fd) {
	for (std::vector<struct pollfd>::iterator it = pollfds_.begin();
			it != pollfds_.end(); ++it) {
		if (it->fd == fd) {
			std::cout << "[Poller] Removed fd=" << fd << std::endl;
			pollfds_.erase(it);
			return;
		}
	}
}

// DONE: Implement Poller::poll(int timeout)
// ONLY PLACE poll() IS CALLED - see TEAM_CONVENTIONS.md
// - Call ::poll(pollfds_.data(), pollfds_.size(), timeout)
// - Handle errors
// - Return number of ready file descriptors
int Poller::poll(int timeout) {
	if (pollfds_.empty()) {
		return 0;
	}

	int ready = ::poll(&pollfds_[0], pollfds_.size(), timeout);

	if (ready < 0) {
		if (errno == EINTR) {
			return 0;  // Signal interrupt - normal
		}
		std::cerr << "[Poller] poll() error: " << strerror(errno) << std::endl;
		return 0;
	}
	return ready;
}

// DONE: Implement Poller::processEvents()
// - Iterate through pollfds_
// - For each fd with events:
//   - If POLLIN: check if server socket or client socket
//     - Server socket: call server->handleNewConnection()
//     - Client socket: call server->handleClientInput(fd)
//   - If POLLOUT: handle write events (if needed)
//   - If POLLERR/POLLHUP: call server->disconnectClient(fd)
void	Poller::processEvents() {
	int	serverFd = server_->getServerFd();

	for (size_t i = 0; i < pollfds_.size(); ++i) {
		int fd = pollfds_[i].fd;
		short revents = pollfds_[i].revents;
		
		if (revents == 0) {
			continue;
		}
		
		std::cout << "[Poller] fd=" << fd << " revents=0x" << std::hex << revents << std::dec << std::endl;
		
		// POLLIN: data ready
		if (revents & POLLIN) {
			if (fd == serverFd) {
				// New connection
				server_->handleNewConnection();
			} else {
				// Client data
				server_->handleClientInput(fd);
			}
		}
		
		// POLLERR/POLLHUP: error/disconnect
		if (revents & (POLLERR | POLLHUP)) {
			if (fd != serverFd) {
				std::cout << "[Poller] Error/HUP on fd=" << fd << std::endl;
				server_->disconnectClient(fd);
			}
		}
	}
}

// TODO: Implement Poller::hasEvent(int fd, short event) const
// - Find fd in pollfds_
// - Check if revents has the specified event

// TODO: Implement Poller::findFdIndex(int fd) const
// - Linear search through pollfds_
// - Return index or -1 if not found

// TODO: Implement Poller::updatePollfds()
// - Rebuild pollfds_ if needed (usually not necessary)

