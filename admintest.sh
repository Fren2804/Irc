#!/bin/bash

SERVER="localhost"
PORT=6667
NICK="ADMIN"

{
    echo "PASS 23"
    sleep 1
    echo "NICK $NICK"
    sleep 1
    echo "USER $NICK 0 * :$NICK"
    sleep 1
    echo "JOIN #madrid"
    sleep 1
    echo "JOIN #berlin"
    sleep 1
    echo "MODE #berlin +i"
    sleep 1
    echo "MODE #madrid +l 2"
    sleep 1
    echo "JOIN #moscu"
    sleep 1
    echo "MODE #moscu +k 24"
    sleep 1
} | nc $SERVER $PORT