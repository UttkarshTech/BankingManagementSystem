#!/bin/bash
clear
./cleanup.sh
touch users.dat txns.dat feedbacks.dat loans.dat loanid.dat
gcc -c *.c
gcc server.o *X.o -o server.out
gcc client.o *X.o -o client.out
./reset.sh
./server.out