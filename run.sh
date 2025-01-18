#!/bin/bash
FILENAME=test

#Compile
./compile.sh
retval=$?; if [ $retval -ne 0 ]; then exit; fi

#Sizes before running! Otherwise no way to see them if ie go into debug loop
size *.o > sizes.txt
./sizes.py

./$FILENAME


