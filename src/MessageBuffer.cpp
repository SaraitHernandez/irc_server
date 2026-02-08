/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MessageBuffer.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: akacprzy <akacprzy@student.42warsaw.pl>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/18 22:00:02 by akacprzy          #+#    #+#             */
/*   Updated: 2026/02/08 14:50:12 by akacprzy         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "irc/MessageBuffer.hpp"

// Constructor - Initialize buffer_ as empty string
MessageBuffer::MessageBuffer() : buffer_("") {}

// Destructor - No cleanup needed (string handles it)
MessageBuffer::~MessageBuffer() {}

// Method - append() data to buffer_
void MessageBuffer::append(const std::string& data)
{
    buffer_ += data;
}

// Method - extractMessages()
// - Find all complete messages (ending with \r\n)
// - Extract each complete message
// - Remove extracted messages from buffer_
// - Return vector of complete message strings
// - Keep incomplete data in buffer_ for next append
std::vector<std::string> MessageBuffer::extractMessages()
{
    std::vector<std::string> messages;
    size_t pos;

    while ((pos = findMessageEnd()) != std::string::npos)
    {
        // Extract message with \r\n
        std::string msg = buffer_.substr(0, pos + 2);
        messages.push_back(msg);
        // Remove message with \r\n (pos points to \r, +2 covers \r\n)
        buffer_.erase(0, pos + 2); 
    }
    return messages;
}

// Method - getBuffer() - return const reference to buffer_
const std::string& MessageBuffer::getBuffer() const
{
    return buffer_;
}

// Method - clear() - clear buffer_ string
void MessageBuffer::clear() 
{
    buffer_.clear();
}

// Method - isEmpty() - return buffer_.empty()
bool MessageBuffer::isEmpty() const
{
    return buffer_.empty();
}

// Method - size() - return buffer_.size()
size_t MessageBuffer::size() const 
{
    return buffer_.size();
}

// Method - findMessageEnd(size_t startPos) - helper method to find next "\r\n" starting from startPos
size_t MessageBuffer::findMessageEnd(size_t startPos) const
{
    return buffer_.find("\r\n", startPos);
}
