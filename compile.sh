#!/bin/bash
FILENAME=test

#Compile
gcc -Os -c -g *.c
retval=$?; if [ $retval -ne 0 ]; then exit; fi

#Link
gcc -Os *.o -o $FILENAME


