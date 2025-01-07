#!/bin/bash
FILENAME=test

rm -f $FILENAME
gcc -Os *.c -c
gcc *.o -o $FILENAME
./$FILENAME

size *.o > sizes.txt
./sizes.py

