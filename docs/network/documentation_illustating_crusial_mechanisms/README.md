# Network Mechanisms & Architecture Documentation

Comprehensive documentation of crucial IRC server mechanisms and internals.

---

## Contents

### [01_WORKFLOW_PIPELINE.md](01_WORKFLOW_PIPELINE.md)
Complete ASCII workflow diagram showing the entire call chain and program flow from `main()` to shutdown. Includes:
- Server initialization & socket setup
- Main event loop architecture
- Connection handling
- Message reception pipeline
- Command parsing & dispatch
- Registration flow (PASS → NICK → USER)
- Command handlers (PASS, NICK, USER, JOIN, PRIVMSG)
- Broadcasting mechanism
- Cleanup & resource deallocation

### [02_SOCKET_QUEUE_BACKLOG.md](02_SOCKET_QUEUE_BACKLOG.md)
Explanation of the `backlog` parameter in `listen()`:
- What backlog means
- Connection acceptance flow
- Queue behavior
- Recommended values for different scenarios

---

## Purpose

This directory contains detailed documentation of IRC server mechanisms that are essential for understanding and debugging the system. Topics focus on:
- **Architecture**: Layered design (I/O, Protocol, Logic, Application)
- **Flow**: Control flow through major subsystems
- **Mechanisms**: How specific features work internally
- **Concepts**: Networking concepts relevant to IRC protocol

---

## Audience

- Team members learning the codebase
- Contributors implementing new features
- Debuggers investigating issues
- Future maintainers

---

## Related Files

- Main architecture overview: [../INDEX.md](../INDEX.md)
- Team conventions: [../TEAM_CONVENTIONS.md](../TEAM_CONVENTIONS.md)
- Protocol documentation: [../IRC_LOGIC_AND_DATA_STRUCTURE.md](../IRC_LOGIC_AND_DATA_STRUCTURE.md)
