// MessageBuffer implementation
// Manages incomplete messages, extracts complete messages ending with CRLF

#include "irc/MessageBuffer.hpp"

// TODO: Implement MessageBuffer::MessageBuffer()
// - Initialize buffer_ as empty string

// TODO: Implement MessageBuffer::~MessageBuffer()
// - No cleanup needed (string handles it)

// TODO: Implement MessageBuffer::append(const std::string& data)
// - Append data to buffer_
// - Handle binary data safely

// TODO: Implement MessageBuffer::extractMessages()
// - Find all complete messages (ending with \r\n)
// - Extract each complete message
// - Remove extracted messages from buffer_
// - Return vector of complete message strings
// - Keep incomplete data in buffer_ for next append
//
// Algorithm:
//   while (buffer_ contains "\r\n") {
//       find position of "\r\n"
//       extract substring from start to position+2
//       add to result vector
//       remove from buffer_
//   }

// TODO: Implement MessageBuffer::getBuffer() const
// - Return const reference to buffer_

// TODO: Implement MessageBuffer::clear()
// - Clear buffer_ string

// TODO: Implement MessageBuffer::isEmpty() const
// - Return buffer_.empty()

// TODO: Implement MessageBuffer::size() const
// - Return buffer_.size()

// TODO: Implement MessageBuffer::findMessageEnd(size_t startPos) const
// - Helper method to find next "\r\n" starting from startPos
// - Return position or std::string::npos if not found

