#!/bin/bash

for i in *.c;
do
    if [ "$i" != "debug.c" ]; then
        if [ "$i" != "test.c" ]; then
            /opt/msp430-gcc/bin/msp430-elf-gcc -I /opt/msp430-gcc/include -mmcu=MSP430G2553 -Os -Werror -g -c -o temp-msp430/`basename $i .c`.o $i;
        fi
    fi
done

cd temp-msp430
msp430-elf-size *.o > sizes.txt
./sizes.py
