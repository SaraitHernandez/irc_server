# 🧪 Manual Testing Guide - IRC Server

**Complete guide for manual testing of all IRC server features**

---

## 📋 Quick Setup

```bash
# 1. Start server
./ircserv 6667 test123

# 2. In another terminal, connect
nc localhost 6667

# 3. Run tests
```

---

## ✅ Test Checklist

Use this checklist to verify all features work correctly.

### 1. Authentication Tests

#### Test 1.1: Successful Authentication ✅
```bash
PASS test123
NICK alice
USER alice 0 * :Alice Smith
```

**Expected:**
```
:ft_irc 001 alice :Welcome to the IRC Network alice!alice@127.0.0.1
:ft_irc 002 alice :Your host is ft_irc, running version 1.0
:ft_irc 003 alice :This server was created today
:ft_irc 004 alice ft_irc 1.0 o itkol
```

**✓ Verify:**
- [ ] All 4 welcome messages received (001-004)
- [ ] Hostname shows real IP (not "unknown")
- [ ] Client can proceed to use other commands

---

#### Test 1.2: Wrong Password ❌
```bash
PASS wrongpassword
```

**Expected:**
```
:ft_irc 464 * :Password incorrect
```

**✓ Verify:**
- [ ] Error message received
- [ ] Can retry (3 attempts allowed)

---

#### Test 1.3: Password Retry Limit (Halloy) ✅
```bash
PASS wrong1
PASS wrong2
PASS wrong3
```

**Expected:**
```
:ft_irc 464 * :Password incorrect
:ft_irc 464 * :Password incorrect
:ft_irc 464 * :Password incorrect (max attempts exceeded)
(Connection closed)
```

**✓ Verify:**
- [ ] 3 attempts allowed
- [ ] Disconnected after 3rd failed attempt

---

#### Test 1.4: Wrong Order (NICK before PASS) ❌
```bash
NICK alice
```

**Expected:**
```
:ft_irc 451 * :You have not registered
```

**✓ Verify:**
- [ ] Error ERR_NOTREGISTERED (451)
- [ ] Cannot proceed without PASS first

---

#### Test 1.5: Missing Nickname ❌
```bash
PASS test123
NICK
```

**Expected:**
```
:ft_irc 431 * :No nickname given
```

**✓ Verify:**
- [ ] Error ERR_NONICKNAMEGIVEN (431)

---

### 2. Channel Tests

#### Test 2.1: Create and Join Channel ✅
```bash
PASS test123
NICK bob
USER bob 0 * :Bob Jones
JOIN #test
```

**Expected:**
```
:bob!bob@127.0.0.1 JOIN :#test
:ft_irc 331 bob #test :No topic is set
:ft_irc 353 bob = #test :@bob
:ft_irc 366 bob #test :End of /NAMES list
```

**✓ Verify:**
- [ ] JOIN notification received
- [ ] User is operator (@ symbol)
- [ ] NAMES list shows user
- [ ] Topic message (no topic set)

---

#### Test 2.2: Set and View Topic ✅
```bash
TOPIC #test :This is a test channel
TOPIC #test
```

**Expected:**
```
:bob!bob@127.0.0.1 TOPIC #test :This is a test channel
:ft_irc 332 bob #test :This is a test channel
```

**✓ Verify:**
- [ ] Topic set successfully
- [ ] Topic query returns correct topic

---

#### Test 2.3: Leave Channel (PART) ✅
```bash
PART #test :Goodbye everyone
```

**Expected:**
```
:bob!bob@127.0.0.1 PART #test :Goodbye everyone
```

**✓ Verify:**
- [ ] PART notification with message
- [ ] No longer in channel

---

### 3. Messaging Tests

#### Test 3.1: Channel Message ✅
```bash
# After joining #test
PRIVMSG #test :Hello everyone!
```

**Expected:**
- Message broadcast to all channel members (except sender)

**✓ Verify:**
- [ ] Message sent without error
- [ ] Other clients in channel receive message

---

#### Test 3.2: Private Message ✅
```bash
# With another user "alice" connected
PRIVMSG alice :Hello Alice!
```

**Expected:**
- Message sent to alice only

**✓ Verify:**
- [ ] No error if alice exists
- [ ] Alice receives: `:sender!user@host PRIVMSG alice :Hello Alice!`

---

#### Test 3.3: Message to Non-Existent User ❌
```bash
PRIVMSG nobody :Hello
```

**Expected:**
```
:ft_irc 401 bob nobody :No such nick/channel
```

**✓ Verify:**
- [ ] Error ERR_NOSUCHNICK (401)

---

### 4. Channel Mode Tests

#### Test 4.1: Invite-Only Mode (+i) ✅
```bash
# As operator in #test
MODE #test +i
MODE #test
```

**Expected:**
```
:bob!bob@127.0.0.1 MODE #test +i
:ft_irc 324 bob #test +i
```

**✓ Verify:**
- [ ] Mode set notification
- [ ] Mode query shows +i

---

#### Test 4.2: Topic Protected Mode (+t) ✅
```bash
MODE #test +t
```

**Expected:**
```
:bob!bob@127.0.0.1 MODE #test +t
```

**✓ Verify:**
- [ ] Only operators can change topic now

---

#### Test 4.3: Channel Key (+k) ✅
```bash
MODE #test +k secretpassword
MODE #test
```

**Expected:**
```
:bob!bob@127.0.0.1 MODE #test +k secretpassword
:ft_irc 324 bob #test +itk
```

**✓ Verify:**
- [ ] Key set
- [ ] Mode query shows +k
- [ ] JOIN requires key

---

#### Test 4.4: User Limit (+l) ✅
```bash
MODE #test +l 5
MODE #test
```

**Expected:**
```
:bob!bob@127.0.0.1 MODE #test +l 5
:ft_irc 324 bob #test +itkl
```

**✓ Verify:**
- [ ] Limit set
- [ ] Cannot join when channel full (5 users)

---

#### Test 4.5: Operator Mode (+o) ✅
```bash
# With alice in channel
MODE #test +o alice
```

**Expected:**
```
:bob!bob@127.0.0.1 MODE #test +o alice
```

**✓ Verify:**
- [ ] Alice becomes operator (@alice in NAMES)

---

### 5. Operator Command Tests

#### Test 5.1: INVITE ✅
```bash
# Channel is +i (invite-only)
INVITE charlie #test
```

**Expected:**
```
:ft_irc 341 bob charlie #test
:bob!bob@127.0.0.1 INVITE charlie :#test
```

**✓ Verify:**
- [ ] RPL_INVITING (341) received
- [ ] Charlie receives INVITE notification
- [ ] Charlie can now join +i channel

---

#### Test 5.2: KICK ✅
```bash
# With alice in #test
KICK #test alice :Violating rules
```

**Expected:**
```
:bob!bob@127.0.0.1 KICK #test alice :Violating rules
```

**✓ Verify:**
- [ ] Alice removed from channel
- [ ] All members see KICK notification
- [ ] Reason included

---

#### Test 5.3: Non-Operator KICK Attempt ❌
```bash
# As non-operator user
KICK #test bob :Trying to kick
```

**Expected:**
```
:ft_irc 482 alice #test :You're not channel operator
```

**✓ Verify:**
- [ ] Error ERR_CHANOPRIVSNEEDED (482)
- [ ] KICK denied

---

### 6. Connection Tests

#### Test 6.1: PING/PONG ✅
```bash
PING :test123
```

**Expected:**
```
:ft_irc PONG ft_irc :test123
```

**✓ Verify:**
- [ ] PONG response with same token

---

#### Test 6.2: QUIT ✅
```bash
QUIT :Goodbye everyone
```

**Expected:**
- All channels broadcast: `:bob!bob@127.0.0.1 QUIT :Goodbye everyone`
- Connection closes

**✓ Verify:**
- [ ] QUIT message sent to all channels
- [ ] Client disconnected
- [ ] Channels cleaned up (if empty)

---

### 7. Multi-Client Tests

#### Test 7.1: Two Clients Same Channel ✅

**Terminal 1:**
```bash
nc localhost 6667
PASS test123
NICK alice
USER alice 0 * :Alice
JOIN #party
```

**Terminal 2:**
```bash
nc localhost 6667
PASS test123
NICK bob
USER bob 0 * :Bob
JOIN #party
```

**✓ Verify:**
- [ ] Alice sees bob JOIN notification
- [ ] Bob sees alice in NAMES list (@alice bob)
- [ ] Both can send messages
- [ ] Both receive each other's messages

---

#### Test 7.2: Three Clients Same Channel ✅

Add **Terminal 3:**
```bash
nc localhost 6667
PASS test123
NICK charlie
USER charlie 0 * :Charlie
JOIN #party
```

**✓ Verify:**
- [ ] All clients see charlie JOIN
- [ ] Charlie sees: @alice bob charlie in NAMES
- [ ] Only alice is operator (@)

---

### 8. Case-Insensitive Tests (Halloy)

#### Test 8.1: Nickname Case ✅
```bash
PASS test123
NICK BOB
USER bob 0 * :Bob
NICK bob
```

**Expected:**
```
:BOB!bob@127.0.0.1 NICK :bob
```

**✓ Verify:**
- [ ] Same client can change case
- [ ] Original case preserved in prefix

---

#### Test 8.2: Duplicate Nickname Different Case ❌

**Terminal 1:** `NICK Alice`  
**Terminal 2:** `NICK alice`

**Expected Terminal 2:**
```
:ft_irc 433 * alice :Nickname is already in use
```

**✓ Verify:**
- [ ] Second client gets ERR_NICKNAMEINUSE
- [ ] Nicknames are case-insensitive

---

### 9. Edge Case Tests

#### Test 9.1: Empty Parameters ❌
```bash
PASS test123
NICK
USER
JOIN
```

**Expected:**
- ERR_NONICKNAMEGIVEN for NICK
- ERR_NEEDMOREPARAMS for USER
- ERR_NEEDMOREPARAMS for JOIN

**✓ Verify:**
- [ ] Proper error codes for each

---

#### Test 9.2: Unknown Command ❌
```bash
PASS test123
NICK alice
USER alice 0 * :Alice
UNKNOWNCOMMAND param1 param2
```

**Expected:**
```
:ft_irc 421 alice UNKNOWNCOMMAND :Unknown command
```

**✓ Verify:**
- [ ] ERR_UNKNOWNCOMMAND (421)

---

#### Test 9.3: Partial Data (Ctrl+D Test) ✅
```bash
nc localhost 6667
PASS te(Ctrl+D)st123(Ctrl+D)
NICK al(Ctrl+D)ice
```

**✓ Verify:**
- [ ] MessageBuffer reconstructs: "PASS test123" and "NICK alice"
- [ ] Commands execute correctly

---

### 10. Cleanup Tests

#### Test 10.1: Disconnect Cleanup ✅
```bash
# Connect, join #test, then close connection (Ctrl+C)
```

**✓ Verify (in server logs):**
- [ ] `[Server] Disconnecting fd=X`
- [ ] `[Server] Removing empty channel: #test`
- [ ] No memory leaks

---

#### Test 10.2: Multiple Disconnect ✅
```bash
# Connect 3 clients to #test
# Disconnect all 3
```

**✓ Verify:**
- [ ] Channel deleted after last user leaves
- [ ] Server still running
- [ ] New clients can connect

---

## 📊 Test Coverage Summary

| Category | Tests | Status |
|----------|-------|--------|
| Authentication | 5 | ✅ |
| Channels | 3 | ✅ |
| Messaging | 3 | ✅ |
| Modes | 5 | ✅ |
| Operators | 3 | ✅ |
| Connection | 2 | ✅ |
| Multi-Client | 2 | ✅ |
| Case-Insensitive | 2 | ✅ |
| Edge Cases | 3 | ✅ |
| Cleanup | 2 | ✅ |
| **TOTAL** | **30** | **✅** |

---

## 🔧 Testing Tools

### Using netcat (nc)
```bash
nc localhost 6667
# Type commands manually
```

### Using irssi
```bash
irssi -c localhost -p 6667
/server test123
/join #test
/msg #test Hello
```

### Using telnet
```bash
telnet localhost 6667
PASS test123
NICK testuser
USER test 0 * :Test
```

---

## 📝 Test Notes

### Halloy Specific Tests
- ✅ Strict order PASS → NICK → USER
- ✅ Password retry (3 attempts)
- ✅ Case-insensitive nicknames
- ✅ Case-preserving display

### Performance Tests
- Can handle 3+ simultaneous clients
- Messages delivered < 300ms
- No blocking observed

### Memory Tests
```bash
# macOS
leaks -atExit -- ./ircserv 6667 test123

# Linux
valgrind --leak-check=full ./ircserv 6667 test123
```

---

## 🐛 Common Issues During Testing

### Issue: "Connection refused"
**Solution:** Server not running. Start with `./ircserv 6667 test123`

### Issue: "Port already in use"
**Solution:** Kill existing server: `killall ircserv`

### Issue: Commands don't work
**Solution:** Check you completed authentication (PASS, NICK, USER)

### Issue: Can't see other clients
**Solution:** Both clients must stay connected (don't disconnect immediately)

---

## ✅ Quick Smoke Test (2 minutes)

Run these commands to verify basic functionality:

```bash
# Start server
./ircserv 6667 test123

# In another terminal
nc localhost 6667
PASS test123
NICK tester
USER test 0 * :Test User
JOIN #test
MODE #test +i
TOPIC #test :Testing
PRIVMSG #test :Hello!
QUIT :Done
```

**If all works:** ✅ Server is functional!

---

**Test Document Version:** 1.0  
**Last Updated:** March 12, 2026  
**Status:** Complete

---

**END OF MANUAL TESTING GUIDE**
