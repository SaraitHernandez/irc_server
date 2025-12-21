#ifndef MESSAGEBUFFER_HPP
#define MESSAGEBUFFER_HPP

#include <string>
#include <vector>

// MessageBuffer class - manages incomplete messages
// Accumulates data until complete messages (ending with CRLF) are found
class MessageBuffer {
public:
    // Constructor
    // MessageBuffer();
    
    // Destructor
    // ~MessageBuffer();
    
    // Append raw data to buffer
    // void append(const std::string& data);
    
    // Extract complete messages (ending with \r\n)
    // Returns vector of complete messages, leaves incomplete data in buffer
    // std::vector<std::string> extractMessages();
    
    // Get current buffer contents (for debugging)
    // const std::string& getBuffer() const;
    
    // Clear buffer
    // void clear();
    
    // Check if buffer is empty
    // bool isEmpty() const;
    
    // Get buffer size
    // size_t size() const;
    
private:
    // std::string buffer_;  // Accumulated data
    
    // Helper: find next complete message in buffer
    // size_t findMessageEnd(size_t startPos = 0) const;
};

#endif // MESSAGEBUFFER_HPP

