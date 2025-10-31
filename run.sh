#!/bin/bash
clear
gcc -c *.c
gcc server.o *X.o -o server.out
gcc client.o *X.o -o client.out
touch txns.dat users.dat
./reset.sh
./server.out
