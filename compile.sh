#!/bin/bash
FILENAME=test

#Compile
#gcc -Werror -Os -c -Wa,-aghlns=combined.s *.c
gcc -Werror -Os -c -g src/*.c
retval=$?; if [ $retval -ne 0 ]; then exit $retval; fi

#Link
gcc -Os *.o -o $FILENAME
retval=$?; if [ $retval -ne 0 ]; then exit $retval; fi

exit 0

