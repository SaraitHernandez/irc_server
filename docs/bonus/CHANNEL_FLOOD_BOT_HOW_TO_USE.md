```
ft_irc/
├── ircserv                        ← main server binary (make re)
├── channel_flood_bot              ← bot binary (make bot)
├── Makefile
├── include/
│   └── commands/
├── src/
│   ├── Channel.cpp
│   ├── Server.cpp
│   ├── ...                        ← built as part of ircserv
│   └── bot/
│       └── channel_flood_bot.cpp  ← built only via make bot
└── tests/
    └── bonus/
        ├── flood_channel.sh       ← flood test (200 PRIVMSG)
        └── test_bot.sh            ← bot command verification

```

***

# Scenario VII + Bonus — Manual Test

> **Important:** open ALL terminals in the project root directory.
> ```bash
> cd ~/ft_irc_eval   # do this in every terminal before starting
> pwd                # must show the project root
> ```

### 0. Build

```bash
make re && make bot
```

***

## Terminal 1 — server (valgrind)
```bash
valgrind --leak-check=full --track-fds=yes ./ircserv 6667 testpass
```

## Terminal 2 — bot (valgrind)
```bash
./channel_flood_bot
```
Wait for `[*] joined #test — listening` → **Ctrl+Z** ← freeze bot
```
zsh: suspended
```
## Terminal 3 — client A
```bash
nc -C 127.0.0.1 6667
```
```
PASS testpass / NICK clientA / USER clientA 0 * :Client A / JOIN #test
```
Wait for `:ft_irc 366 ... End of /NAMES list` → **Ctrl+Z** ← freeze client A
```
zsh: suspended 
```
## Terminal 4 — flood
```bash
bash bonus/flood_channel.sh
```
Wait for script to finish.

## Terminal 3 — unfreeze client A
```bash
fg
```
✓ All 200 messages arrive → **Ctrl+C**

## Terminal 2 — unfreeze bot
```bash
fg
```
✓ `[<<SVR]` lines appear

## Terminal 5 — verify bot
```bash
bash bonus/test_bot.sh
```
✓ Expected in Terminal 2:
```
:flood_bot!... PRIVMSG #test :pong!
:flood_bot!... PRIVMSG #test :uptime: 0h Xm Xs
:flood_bot!... PRIVMSG #test :HELLO EVALUATOR
```

## Terminal 1 → stop server → valgrind
**Ctrl+C** → expect `0 bytes in 0 blocks`, `0 errors`

***

## Pass criteria

| Check | Expected |
|---|---|
| Server alive after flood | no crash, no hang |
| Client A got all 200 msgs | full burst after `fg` Terminal 3 |
| Bot survived freeze + flood | resumed after `fg` Terminal 2 |
| Bot responds to commands | pong / uptime / echo in Terminal 5 |
| Bot valgrind | 0 bytes, 0 errors |
| Server valgrind | 0 bytes, 0 errors |