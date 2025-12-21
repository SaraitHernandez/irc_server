// Channel implementation
// Manages channel members, operators, topic, and modes

#include "irc/Channel.hpp"
#include "irc/Client.hpp"
#include <algorithm>
#include <ctime>

// TODO: Implement Channel::Channel(const std::string& name)
// - Store name_
// - Initialize topic_ as empty
// - Initialize topicTime_ to 0
// - Initialize all mode flags to false
// - Initialize clients_ map

// TODO: Implement Channel::~Channel()
// - Cleanup - clients are not owned by Channel, just references

// TODO: Implement Channel::getName() const

// TODO: Implement membership methods
// - hasClient(Client* client) const - check if in clients_ map
// - addClient(Client* client) - add to clients_ map
// - removeClient(Client* client) - remove from clients_ map and operators_
// - getClients() const - return vector of all clients
// - getClientCount() const - return clients_.size()

// TODO: Implement operator methods
// - isOperator(Client* client) const - check in operators_ vector
// - addOperator(Client* client) - add to operators_ if not already
// - removeOperator(Client* client) - remove from operators_
// - getOperators() const - return operators_ vector

// TODO: Implement topic methods
// - getTopic() const
// - setTopic(const std::string& topic, const std::string& setBy)
//   - Update topic_, topicSetter_, topicTime_ (current time)
// - getTopicSetter() const
// - getTopicTime() const

// TODO: Implement mode methods
// - hasMode(char mode) const - check mode flags (i, t, k, o, l)
// - setMode(char mode, bool value) - set appropriate flag
// - getModeString() const - return string like "+itk"
// - getChannelKey() const
// - setChannelKey(const std::string& key)
// - getUserLimit() const
// - setUserLimit(int limit)

// TODO: Implement Channel::broadcast(const std::string& message, Client* exclude)
// - Iterate through clients_ map
// - For each client != exclude, send message
// - Note: Actual sending should be done via Server::sendToClient()
//   This method should accept a callback or return list of fds, or
//   be called from Server which handles the sending

// TODO: Implement Channel::broadcastFrom(...)
// - Similar to broadcast but includes sender prefix in message

// TODO: Implement Channel::isEmpty() const
// - Return true if clients_.empty()

