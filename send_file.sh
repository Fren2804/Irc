#!/bin/bash

SERVER="localhost"
PORT=6667
SENDER="alice"
TARGET="Cliente1"
FILE="$1"

if [ -z "$FILE" ]; then
    echo "Uso: ./send_file.sh archivo.bin"
    exit 1
fi

SIZE=$(stat -c%s "$FILE")

echo "Enviando archivo '$FILE' ($SIZE bytes) a $TARGET"

{
	echo "PASS 23"
	sleep 1
    # Registro básico
    echo "NICK $SENDER"
    echo "USER $SENDER 0 * :$SENDER"
    sleep 1

    # Comando FILE
    echo "FILE $TARGET $(basename $FILE) $SIZE"
    sleep 1

    # Enviar los bytes CRUDOS del archivo
    cat "$FILE"

    sleep 1
} | nc -C $SERVER $PORT
