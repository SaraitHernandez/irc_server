// Poller implementation
// ONLY class that calls poll() system call - see TEAM_CONVENTIONS.md
// Manages file descriptor polling and event notification

#include "irc/Poller.hpp"
#include "irc/Server.hpp"
#include <algorithm>
#include <poll.h>

// TODO: Implement Poller::Poller(Server* server)
// - Store server pointer
// - Initialize pollfds_ vector

// TODO: Implement Poller::~Poller()
// - Cleanup if needed

// TODO: Implement Poller::addFd(int fd, short events)
// - Create pollfd structure
// - Add to pollfds_ vector
// - Set fd, events, and revents (initially 0)

// TODO: Implement Poller::removeFd(int fd)
// - Find fd in pollfds_ vector
// - Remove from vector
// - Update indices if needed

// TODO: Implement Poller::poll(int timeout)
// ONLY PLACE poll() IS CALLED - see TEAM_CONVENTIONS.md
// - Call ::poll(pollfds_.data(), pollfds_.size(), timeout)
// - Handle errors
// - Return number of ready file descriptors

// TODO: Implement Poller::processEvents()
// - Iterate through pollfds_
// - For each fd with events:
//     - If POLLIN: check if server socket or client socket
//       - Server socket: call server->handleNewConnection()
//       - Client socket: call server->handleClientInput(fd)
//     - If POLLOUT: handle write events (if needed)
//     - If POLLERR/POLLHUP: call server->disconnectClient(fd)

// TODO: Implement Poller::hasEvent(int fd, short event) const
// - Find fd in pollfds_
// - Check if revents has the specified event

// TODO: Implement Poller::findFdIndex(int fd) const
// - Linear search through pollfds_
// - Return index or -1 if not found

// TODO: Implement Poller::updatePollfds()
// - Rebuild pollfds_ if needed (usually not necessary)

