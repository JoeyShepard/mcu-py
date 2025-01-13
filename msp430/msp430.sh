/opt/msp430-gcc/bin/msp430-elf-gcc -I /opt/msp430-gcc/include -mmcu=MSP430G2553 -O2 -Wall -g -c -o execute.o execute.c
/opt/msp430-gcc/bin/msp430-elf-gcc -I /opt/msp430-gcc/include -mmcu=MSP430G2553 -O2 -Wall -g -L /opt/msp430-gcc/include -Wl,--gc-sections  execute.o -o mcu-py.out
