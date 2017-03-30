#!/bin/bash

killall daemonServer;
rm resultat;
gcc daemonServer.c -o daemonServer -l sqlite3;
./daemonServer;
echo "" > stdout.txt
