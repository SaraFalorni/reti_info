#!/bin/bash

echo "compilazione server.."
gcc -Wall -pthread \
	server.c \
	serverUtility/gameFunctions.c \
	serverUtility/generalFunctions.c \
	serverUtility/PlayersFunctions.c \
	serverUtility/QAfunctions.c \
	serverUtility/rankingsFunctions.c \
	-o server
	
if [ $? -ne 0 ]; then
	echo "errore nella compilazione del server"
	exit 1
fi

echo "compilazione client.."
gcc -Wall \
	client.c \
	utility/clientUtility.c \
	-o client
	
if [ $? -ne 0 ]; then
	echo "errore nella compilazione del client"
	exit 1
fi

echo "compilazione completata"
	
