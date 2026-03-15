#!/bin/bash

# Stress Test: 10 simultaneous clients
# Usage: ./stress_test_10_clients.sh [port] [password]

PORT=${1:-8080}
PASS=${2:-test123}

echo "=== STRESS TEST: 10 SIMULTANEOUS CLIENTS ==="
echo "Server: localhost:$PORT"
echo "Starting clients..."

for i in {1..10}; do
    (
        printf "PASS %s\r\n" "$PASS"
        printf "NICK user%s\r\n" "$i"
        printf "USER user%s 0 * :User %s\r\n" "$i" "$i"
        printf "JOIN #stress\r\n"
        printf "PRIVMSG #stress :Hello from user%s\r\n" "$i"
        sleep 1
        printf "QUIT :Stress test done\r\n"
    ) | nc localhost $PORT > /tmp/stress_client_$i.log 2>&1 &
done

wait
echo "All clients finished"
echo ""

PASS_COUNT=0
FAIL_COUNT=0

echo "=== RESULTS ==="
for i in {1..10}; do
    if grep -q "Welcome" /tmp/stress_client_$i.log; then
        echo "✅ Client $i: SUCCESS"
        PASS_COUNT=$((PASS_COUNT + 1))
    else
        echo "❌ Client $i: FAILED"
        FAIL_COUNT=$((FAIL_COUNT + 1))
    fi
done

echo ""
echo "=== SUMMARY ==="
echo "Passed: $PASS_COUNT/10"
echo "Failed: $FAIL_COUNT/10"

rm -f /tmp/stress_client_*.log
