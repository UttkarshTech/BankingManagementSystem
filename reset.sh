#!/bin/bash
touch loanID.dat
gcc dummyWL.c genutilsX.c -o a.out
./a.out
gcc dummyW.c
./a.out
gcc dummyR.c -o read.out
gcc dummyRT.c -o readt.out
rm -f txns.dat
touch txns.dat