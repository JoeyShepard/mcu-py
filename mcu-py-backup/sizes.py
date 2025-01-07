#!/usr/bin/env python3

with open("sizes.txt","rt") as f:
    i=0
    total=0
    for line in f.readlines():
        if i==0:
            print(line[:-1])
        else:
            vals=line.split()
            if vals[5]!="test.o":
                total+=int(vals[0])
                print(line[:-1])
        i+=1
    print("Total:")
    print(str(total).rjust(7),end="")
    print(f" ({int(10000*total/0x3000)/100}% of 12K)")
                

