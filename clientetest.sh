#!/bin/bash

SERVER="localhost"
PORT=6667
NICK="ClienteTest"
CANAL1="#roma"
CANAL2="#madrid"
CANAL3="#berlin"
CANAL4="#lisboa"
CANAL5="#moscu"

{
    echo ""
    sleep 1
    echo "PAss"
    sleep 1
    # Si hay problema con el comando pass cierra conexion
    #echo "PASS "
    #sleep 1
    echo "USER $NICK"
    sleep 1
    echo "NICK $NICK"
    sleep 1
    echo "USER $NICK * * $NICK"
    sleep 1
    echo "USER $NICK * * :$NICK"
    sleep 1
    echo "JOIN $CANAL1"
    sleep 1
    echo "KICK"
    sleep 1
    echo "PART"
    sleep 1
    echo "PRIVMSG"
    sleep 1
    echo "TOPIC"
    sleep 1
    echo "MODE"
    sleep 1
    #Sale
    #echo "QUIT :adios"
    echo "no se que poner esto es una prueba"
    sleep 1
    # Si hay problema con el comando pass cierra conexion
    #echo "PASS 23 45"
    #sleep 1
    #echo "PASS 24"
    #sleep 1
    echo "PASS 23"
    sleep 1
    echo "PASS 24"
    sleep 1
    echo "PASS 23"
    sleep 1
    echo "No se que poner"
    sleep 1
    echo "JOIN $CANAL1"
    sleep 1
    echo "KICK"
    sleep 1
    echo "PART"
    sleep 1
    echo "PRIVMSG"
    sleep 1
    echo "TOPIC"
    sleep 1
    echo "MODE"
    sleep 1
    echo "USER"
    sleep 1
    echo "NICK"
    sleep 1
    echo "NICK $NICK $NICK"
    sleep 1
    echo "NICK $NICK"
    sleep 1
    echo "JOIN $CANAL1"
    sleep 1
    echo "NICK $NICK"
    sleep 1
    echo "USER * * *"
    sleep 1
    echo "USER $NICK * * $NICK $NICK"
    sleep 1
    echo "USER $NICK * * :$NICK $NICK $NICK $NICK $NICK $NICK $NICK"
    sleep 1
    echo "USER $NICK * * :"
    sleep 1
    echo "USER $NICK * * :$NICK $NICK"
    sleep 1
    echo "JOIN"
    sleep 1
    echo "JOIN roma"
    sleep 1
    echo "JOIN $CANAL1 $CANAL1"
    sleep 1
    echo "JOIN $CANAL1"
    sleep 1
    #Control canal flags
    echo "JOIN $CANAL2"
    sleep 1
    echo "JOIN $CANAL3"
    sleep 1
    #Control canal flags
    echo "JOIN $CANAL4"
    sleep 1
    echo "JOIN $CANAL5 25"
    sleep 1
    echo "JOIN $CANAL5 24"
    sleep 1
} | nc $SERVER $PORT