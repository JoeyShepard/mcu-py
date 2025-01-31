#!/bin/bash
FILENAME=$1

#Compile
gcc -Werror -Os -c -g $FILENAME
retval=$?; if [ $retval -ne 0 ]; then exit $retval; fi

size "${FILENAME%%.*}.o"
