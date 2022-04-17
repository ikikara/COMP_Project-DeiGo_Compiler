#!/bin/sh

# run lex and compile the resulting C analyser
lex $1.l
yacc -d -v  $1.y
gcc -o $1 -Wall -Wno-unused-function lex.yy.c y.tab.c syntax_tree.c symbol_table.c check_semantic.c LLVM.c

# 'lex' and 'gcc' are commonly available too
