#!/bin/bash
gcc dummyWL.c genutilsX.c -o a.out
./a.out
gcc dummyW.c
./a.out
gcc dummyR.c -o read.out
gcc dummyRT.c -o readt.out