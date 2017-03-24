#!/bin/bash

killall daemonServer;
rm resultat;
gcc daemonServer.c -o daemonServer;
./daemonServer;
