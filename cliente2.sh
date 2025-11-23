#!/bin/bash

SERVER="localhost"
PORT=6667
NICK="Cliente2"

{
    echo "PASS 23"
    sleep 1
    echo "NICK $NICK"
    sleep 1
    echo "USER $NICK 0 * :$NICK"
    sleep 1
    echo "JOIN #roma"
    sleep 1
    echo "JOIN #roma"
    sleep 4
    echo "PART #romA :me voy"
    sleep 1
    echo "PART #roma"
    sleep 1
} | nc $SERVER $PORT
