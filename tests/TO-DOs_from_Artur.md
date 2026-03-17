# TO-DOs

## Presentation
Who: Sara
What: Add 2 slide to the presentation
Where: https://gamma.app/docs/IRC-Server-A-Real-Time-Chat-System-5if21zxcoixvklp?mode=doc
Between "Non-Blocking I/O: Handling Many Users" and "Authentication: The Three-Step Entry"

### Slide 1
Title: Partial messages
Text: The messages can arrive in parts.
Example - when using ctrl+D you send the command in several parts: ’com’, then ’man’, then ’d\n’.
In order to process a command, we have to first aggregate the received packets in order to rebuild full command

### Slide 2
Title: The protocol
Text: 
- Standardized Message Format: Every IRC message is a single line of text ending with a CRLF structured into an optional prefix, a command, and a list of parameters.
- Synchronous Numeric Replies: The server communicates state and errors back to the client using standardized three-digit numeric codes (e.g., 001 for Welcome or 401 for Error) followed by a descriptive string.
- Text-Based Parsing: Because the protocol is strictly string-oriented, the core implementation revolves around a robust parser capable of tokenizing incoming buffers into commands and their respective arguments.

## Code

### PRIVMSG for Halloy
Who: Sara
What: PRIVMSG command - Halloy does not add ":" when a message does not contain spaces or the ":" itself.
Our server receives then e.g.: "PRIVMSG #channel message" and not "PRIVMSG #channel :message"
When the ":" is not added to the message, our server does not send it to other users in the channel, but it should in my opinion to work with Halloy
I think it is related to trailing part being only treated as message by our solution.

### Point 4.2
Who: Sara
What: 
Manual testing guide expects: `464 :Password incorrect`
What we are getting: `:ft_irc 464 * :Password incorrect`
Question: what is the '*' character for here?

Another example:
Expected: `451 :You have not registered`
Received: `:ft_irc 451 * :You have not registered`

### Point 8.15
Who: Sara
What: `MODE #test` does not show the number of users the channel is limited to.

### Point 10.2
Who: Sara
What: I'm not sure how to send PING from server to user
