#!/bin/bash

killall daemonServer;
gcc daemonServer.c -o daemonServer -l sqlite3;
./daemonServer;
echo "" > stdout.txt
