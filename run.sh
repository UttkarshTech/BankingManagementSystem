#!/bin/bash
clear
gcc -c *.c
gcc server.o *X.o -o server.out
gcc client.o *X.o -o client.out
touch users.dat
touch feedbacks.dat
./reset.sh
./server.out
