#!/bin/bash
clear
gcc -c *.c
gcc server.o *X.o -o server.out
gcc client.o *X.o -o client.out
./reset.sh
rm -f txns.dat
touch txns.dat
./server.out
