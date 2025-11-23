#!/bin/bash

SERVER="localhost"
PORT=6667
NICK="Cliente6"

{
    echo "PASS 23"
    sleep 1
    echo "NICK $NICK"
    sleep 1
    echo "USER $NICK 0 * :$NICK"
    sleep 1
    echo "JOIN #berlin"
    sleep 1
} | nc $SERVER $PORT
