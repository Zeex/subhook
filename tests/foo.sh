#!/bin/sh

yasm -f elf64 foo_64_unix.asm
gcc -c foo_main.c -o foo_main.o
gcc foo_64_unix.o foo_main.o -o foo

./foo
