#!/bin/bash

SERVER="localhost"
PORT=6667
NICK="ClienteC"

{
    echo -n "PAS"
    sleep 1
    echo -n "S 2"
    sleep 1
    echo "3"
    sleep 1
    echo -e "USER $NICK 0 * :$NICK\r\nNICK $NICK"
    sleep 1
    echo "JOIN #roma"
    sleep 1
    echo "JOIN #madrid"
    sleep 1
} | nc -C $SERVER $PORT