#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "syntax_tree.h"
#include <stdbool.h>


/*

João Carlos Borges Silva Nº 2019216783
Pedro Afonso Ferreira Lopes Martins Nº 2019216826

*/

typedef struct symbol_ symbol;
struct symbol_{
    char* id;
    char* type;
    char* whatisit;
    char* params;
    int line;
    int column;
    bool used;
    bool is_defined;
    bool is_func;
    int register_tab;
    symbol* next_symbol;
};

typedef struct table_ table;
struct table_{
    char* name;
    char* nametoprint;
    char* type;
    char* params;
    symbol* first_symbol;
    table* next_table;
};

char* tolow(char* string);
void anotate_tree(ast_tree** node);
table* search_table(char* name);
table* new_table(char *name, char *name2, char* name3, ast_tree** node);
void create_tables(ast_tree *node, table* tablita, int level);
void print_tables(table* first_table);
void free_table(table* first_table);


#endif
