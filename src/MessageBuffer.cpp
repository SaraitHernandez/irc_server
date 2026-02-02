/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MessageBuffer.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: oostapen <oostapen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/18 22:00:02 by akacprzy          #+#    #+#             */
/*   Updated: 2026/02/02 22:20:52 by oostapen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "irc/MessageBuffer.hpp"

// Constructor - Initialize buffer_ as empty string
MessageBuffer::MessageBuffer() : buffer_("") {}

// Deconstructor - No cleanup needed (string handles it)
MessageBuffer::~MessageBuffer() {}

// Method - append() data to buffer_
void MessageBuffer::append(const std::string& data)
{
    buffer_ += data;
}


// 111******* DEBUG MULTIPLE MSGs from 2nd terminal
#include <iostream>
std::vector<std::string> MessageBuffer::extractMessages()
{
    std::vector<std::string> messages;
    size_t pos;

    // === DEBUG START ===
    std::cout << "[MessageBuffer] === extractMessages() START ===" << std::endl;
    std::cout << "[MessageBuffer] Buffer content: \"" << buffer_ << "\"" << std::endl;
    std::cout << "[MessageBuffer] Buffer length: " << buffer_.length() << std::endl;
    
    // output in HEX to check
    std::cout << "[MessageBuffer] Buffer HEX: ";
    for (size_t i = 0; i < buffer_.length(); i++) {
        std::cout << std::hex << (int)(unsigned char)buffer_[i] << " ";
    }
    std::cout << std::dec << std::endl;
    // === DEBUG END ===

    while ((pos = findMessageEnd()) != std::string::npos)
    {
        std::cout << "[MessageBuffer] Found \\r\\n at position: " << pos << std::endl;
        
        std::string msg = buffer_.substr(0, pos);
        
        std::cout << "[MessageBuffer] Extracted message: \"" << msg << "\"" << std::endl;
        
        messages.push_back(msg);
        buffer_.erase(0, pos + 2);
        
        std::cout << "[MessageBuffer] Remaining buffer: \"" << buffer_ << "\"" << std::endl;
    }
    
    std::cout << "[MessageBuffer] Total extracted: " << messages.size() << " messages" << std::endl;
    std::cout << "[MessageBuffer] === extractMessages() END ===" << std::endl;
    
    return messages;
}


/*
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
        messages.push_back(buffer_.substr(0, pos));
		//110******* DEBUG MULTIPLE MSGs(2nd terminal)
		// ←  +2 \r\n (DELETED TO TEST)
        buffer_.erase(0, pos + 2);// ←  +2 \r\n(STAYED)
    }
    return messages;
}
*/

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
