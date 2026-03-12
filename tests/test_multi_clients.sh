#!/bin/bash

# Test script for multiple IRC clients

PORT=6668
PASS="test123"

echo "=== TESTING MULTIPLE CLIENTS ==="

# Client 1: Charlie joins and creates channel
echo ""
echo "CLIENT 1 (Charlie): Join and create #party"
(
  sleep 0.5
  echo "PASS $PASS"
  sleep 0.2
  echo "NICK charlie"
  sleep 0.2
  echo "USER charlie 0 * :Charlie Chan"
  sleep 0.5
  echo "JOIN #party"
  sleep 0.5
  echo "PRIVMSG #party :Party starting!"
  sleep 0.5
  echo "MODE #party +i"
  sleep 1
) | nc localhost $PORT &

CLIENT1_PID=$!

# Client 2: Diana joins the party
echo ""
echo "CLIENT 2 (Diana): Try to join +i channel"
(
  sleep 1
  echo "PASS $PASS"
  sleep 0.2
  echo "NICK diana"
  sleep 0.2
  echo "USER diana 0 * :Diana Davis"
  sleep 0.5
  echo "JOIN #party"
  sleep 1
) | nc localhost $PORT &

CLIENT2_PID=$!

# Wait for both clients
wait $CLIENT1_PID
wait $CLIENT2_PID

echo ""
echo "=== TEST COMPLETE ==="
