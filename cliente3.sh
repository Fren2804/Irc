#!/bin/bash

SERVER="localhost"
PORT=6667
NICK="Cliente3"

{
    echo "PASS 23"
    sleep 1
    echo "NICK $NICK"
    sleep 1
    echo "USER $NICK 0 * :$NICK"
    sleep 1
    echo "JOIN #roma"
    sleep 1
    echo "PART #roma me voy"
    sleep 1
    echo "PART #roma :me voy"
    sleep 1
    echo "PRIVMSG #roma :hola a todos"
    sleep 1
    echo "JOIN #roma"
    sleep 1
    echo "PRIVMSG #roma :hola a todos"
    sleep 1
    echo "PRIVMSG #roma hola a todos"
    sleep 1
    echo "PRIVMSG Cliente1 :que tal"
    sleep 1
    echo "PRIVMSG Cliente1 que tal"
    sleep 1
    echo "QUIT :adios"
} | nc $SERVER $PORT
