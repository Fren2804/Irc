#!/bin/bash

SERVER="localhost"
PORT=6667
NICK="Cliente1"

{
    echo "PASS 23"
    sleep 1
    echo "NICK $NICK"
    sleep 1
    echo "USER $NICK 0 * :$NICK"
    sleep 1
    echo "JOIN #roma"
    sleep 1
    echo "JOIN #madrid"
    sleep 1
} | nc $SERVER $PORT


#sleep 5
#echo "KICK #roma Cliente4 :no insultar"
#sleep 1
#echo "KICK #roma Cliente2 :no insultar"
#sleep 1
#echo "KICK #madrid Cliente5 :no insultar"
