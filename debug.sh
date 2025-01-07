#!/bin/bash
FILENAME=test

#Compile
gcc -Os -c -g *.c
retval=$?; if [ $retval -ne 0 ]; then exit; fi

#Link
gcc -Os *.o -o $FILENAME
retval=$?; if [ $retval -ne 0 ]; then exit; fi

#Sizes before running! Otherwise no way to see them if ie go into debug loop
#size *.o > sizes.txt
#./sizes.py

gdb -x gdb-startup.txt -q ./$FILENAME


