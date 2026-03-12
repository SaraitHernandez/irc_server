#!/bin/bash

# Stress Test: 10 simultaneous clients

PORT=6669
PASS="test123"

echo "=== STRESS TEST: 10 SIMULTANEOUS CLIENTS ==="
echo "Starting clients..."

for i in {1..10}; do
    (
        echo "PASS $PASS"
        echo "NICK user$i"
        echo "USER user$i 0 * :User $i"
        echo "JOIN #stress"
        echo "PRIVMSG #stress :Hello from user$i"
        sleep 1
        echo "QUIT :Stress test done"
    ) | nc localhost $PORT > /tmp/stress_client_$i.log 2>&1 &
done

# Wait for all clients
wait

echo "All clients finished"
echo ""
echo "=== RESULTS ==="
for i in {1..10}; do
    if grep -q "Welcome" /tmp/stress_client_$i.log; then
        echo "✅ Client $i: SUCCESS"
    else
        echo "❌ Client $i: FAILED"
        cat /tmp/stress_client_$i.log
    fi
done

# Cleanup
rm -f /tmp/stress_client_*.log
