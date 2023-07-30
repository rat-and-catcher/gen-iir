#!/bin/sh
gcc -O3 -Wall -DLINUX_OS src/gen-iir.c src/dspl/dspl.c -lm -o gen-iir
