#!/bin/sh

# run llvm and compile the resulting C analyser
./gocompiler < $1.txt > $1.ll
llc-7 $1.ll
clang-7 -o $1 -Wall -Wno-unused-function $1.s

# 'lex' and 'gcc' are commonly available too
