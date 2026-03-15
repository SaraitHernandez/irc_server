#!/bin/bash

# Multi-client interaction test
# Usage: ./test_multi_clients.sh [port] [password]

PORT=${1:-8080}
PASS=${2:-test123}
PASS_COUNT=0
FAIL_COUNT=0
TOTAL=0

check() {
    TOTAL=$((TOTAL + 1))
    local label="$1"
    local file="$2"
    local pattern="$3"
    if grep -q "$pattern" "$file" 2>/dev/null; then
        echo "  ✅ $label"
        PASS_COUNT=$((PASS_COUNT + 1))
    else
        echo "  ❌ $label"
        FAIL_COUNT=$((FAIL_COUNT + 1))
    fi
}

echo "=== MULTI-CLIENT INTERACTION TEST ==="
echo "Server: localhost:$PORT"
echo ""

# ──────────────────────────────────────────────
# TEST 1: Two clients chat in the same channel
# ──────────────────────────────────────────────
echo "--- Test 1: Two clients in a channel ---"

(
    printf "PASS %s\r\n" "$PASS"
    printf "NICK alice\r\n"
    printf "USER alice 0 * :Alice\r\n"
    sleep 0.5
    printf "JOIN #chat\r\n"
    sleep 1.5
    printf "QUIT\r\n"
) | nc localhost $PORT > /tmp/mc_alice.log 2>&1 &
PID_ALICE=$!

sleep 0.3

(
    printf "PASS %s\r\n" "$PASS"
    printf "NICK bob\r\n"
    printf "USER bob 0 * :Bob\r\n"
    sleep 0.5
    printf "JOIN #chat\r\n"
    sleep 0.3
    printf "PRIVMSG #chat :Hello Alice!\r\n"
    sleep 0.5
    printf "QUIT\r\n"
) | nc localhost $PORT > /tmp/mc_bob.log 2>&1 &
PID_BOB=$!

wait $PID_ALICE $PID_BOB 2>/dev/null

check "Alice registered"             /tmp/mc_alice.log "001.*Welcome"
check "Bob registered"               /tmp/mc_bob.log   "001.*Welcome"
check "Alice saw Bob join"           /tmp/mc_alice.log  "bob.*JOIN"
check "Alice received Bob's message" /tmp/mc_alice.log  "Hello Alice"

echo ""

# ──────────────────────────────────────────────
# TEST 2: Invite-only channel blocks non-invited
# ──────────────────────────────────────────────
echo "--- Test 2: Invite-only channel (+i) ---"

(
    printf "PASS %s\r\n" "$PASS"
    printf "NICK charlie\r\n"
    printf "USER charlie 0 * :Charlie\r\n"
    sleep 0.3
    printf "JOIN #secret\r\n"
    sleep 0.3
    printf "MODE #secret +i\r\n"
    sleep 1.5
    printf "QUIT\r\n"
) | nc localhost $PORT > /tmp/mc_charlie.log 2>&1 &
PID_CHARLIE=$!

sleep 1

(
    printf "PASS %s\r\n" "$PASS"
    printf "NICK diana\r\n"
    printf "USER diana 0 * :Diana\r\n"
    sleep 0.3
    printf "JOIN #secret\r\n"
    sleep 0.5
    printf "QUIT\r\n"
) | nc localhost $PORT > /tmp/mc_diana.log 2>&1 &
PID_DIANA=$!

wait $PID_CHARLIE $PID_DIANA 2>/dev/null

check "Charlie created #secret"      /tmp/mc_charlie.log "JOIN.*#secret"
check "Charlie set +i"               /tmp/mc_charlie.log "MODE.*#secret.*+i"
check "Diana blocked from #secret"   /tmp/mc_diana.log   "473"

echo ""

# ──────────────────────────────────────────────
# TEST 3: Operator can kick a user
# ──────────────────────────────────────────────
echo "--- Test 3: Operator kick ---"

(
    printf "PASS %s\r\n" "$PASS"
    printf "NICK op_user\r\n"
    printf "USER op_user 0 * :Operator\r\n"
    sleep 0.3
    printf "JOIN #moderated\r\n"
    sleep 1
    printf "KICK #moderated target :Bye!\r\n"
    sleep 0.5
    printf "QUIT\r\n"
) | nc localhost $PORT > /tmp/mc_op.log 2>&1 &
PID_OP=$!

sleep 0.5

(
    printf "PASS %s\r\n" "$PASS"
    printf "NICK target\r\n"
    printf "USER target 0 * :Target\r\n"
    sleep 0.3
    printf "JOIN #moderated\r\n"
    sleep 1.5
    printf "QUIT\r\n"
) | nc localhost $PORT > /tmp/mc_target.log 2>&1 &
PID_TARGET=$!

wait $PID_OP $PID_TARGET 2>/dev/null

check "Op created #moderated"    /tmp/mc_op.log     "JOIN.*#moderated"
check "Target joined"            /tmp/mc_target.log  "JOIN.*#moderated"
check "Target was kicked"        /tmp/mc_target.log  "KICK.*#moderated"

echo ""

# ──────────────────────────────────────────────
# TEST 4: Private messages between two users
# ──────────────────────────────────────────────
echo "--- Test 4: Private message ---"

(
    printf "PASS %s\r\n" "$PASS"
    printf "NICK sender\r\n"
    printf "USER sender 0 * :Sender\r\n"
    sleep 0.8
    printf "PRIVMSG receiver :Secret message\r\n"
    sleep 0.5
    printf "QUIT\r\n"
) | nc localhost $PORT > /tmp/mc_sender.log 2>&1 &
PID_SENDER=$!

sleep 0.3

(
    printf "PASS %s\r\n" "$PASS"
    printf "NICK receiver\r\n"
    printf "USER receiver 0 * :Receiver\r\n"
    sleep 1.5
    printf "QUIT\r\n"
) | nc localhost $PORT > /tmp/mc_receiver.log 2>&1 &
PID_RECEIVER=$!

wait $PID_SENDER $PID_RECEIVER 2>/dev/null

check "Sender registered"            /tmp/mc_sender.log   "001.*Welcome"
check "Receiver got private message"  /tmp/mc_receiver.log "Secret message"

echo ""

# ──────────────────────────────────────────────
# TEST 5: Channel with password (+k)
# ──────────────────────────────────────────────
echo "--- Test 5: Channel password (+k) ---"

(
    printf "PASS %s\r\n" "$PASS"
    printf "NICK eve\r\n"
    printf "USER eve 0 * :Eve\r\n"
    sleep 0.3
    printf "JOIN #locked\r\n"
    sleep 0.3
    printf "MODE #locked +k secretkey\r\n"
    sleep 1.5
    printf "QUIT\r\n"
) | nc localhost $PORT > /tmp/mc_eve.log 2>&1 &
PID_EVE=$!

sleep 1

(
    printf "PASS %s\r\n" "$PASS"
    printf "NICK frank\r\n"
    printf "USER frank 0 * :Frank\r\n"
    sleep 0.3
    printf "JOIN #locked wrongkey\r\n"
    sleep 0.3
    printf "JOIN #locked secretkey\r\n"
    sleep 0.5
    printf "QUIT\r\n"
) | nc localhost $PORT > /tmp/mc_frank.log 2>&1 &
PID_FRANK=$!

wait $PID_EVE $PID_FRANK 2>/dev/null

check "Eve created #locked"           /tmp/mc_eve.log   "JOIN.*#locked"
check "Frank rejected with wrong key"  /tmp/mc_frank.log "475"
check "Frank joined with correct key"  /tmp/mc_frank.log "JOIN.*#locked"

echo ""

# ──────────────────────────────────────────────
# SUMMARY
# ──────────────────────────────────────────────
echo "=== SUMMARY ==="
echo "Passed: $PASS_COUNT/$TOTAL"
echo "Failed: $FAIL_COUNT/$TOTAL"
echo ""
if [ "$FAIL_COUNT" -eq 0 ]; then
    echo "🎉 ALL TESTS PASSED"
else
    echo "⚠️  Some tests failed - check output above"
fi

rm -f /tmp/mc_*.log
