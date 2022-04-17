#ifndef SYNTAX_TREE_H
#define SYNTAX_TREE_H

/*

João Carlos Borges Silva Nº 2019216783
Pedro Afonso Ferreira Lopes Martins Nº 2019216826

*/

typedef struct token_ token;
struct token_{
    char* id;
    char* type;
    int line;
    int column;
    int global;
    int is_defined;
    int is_func;
    int used;
};

typedef struct ast_node ast_tree;
struct ast_node{
    token* token;
    char* value;
    ast_tree* first_child;
    ast_tree* next_sibling;
};

int nrChilds_node (ast_tree* node);
int nrChilds_blocks (ast_tree* node);
token* new_token(char* string, int line, int column);
ast_tree *ast_node(char* name, token* id, char* value);
void add_childs(ast_tree *root, int nargs, ...);
void add_siblings(ast_tree *root, int nargs, ...);
void print_tree(ast_tree *node, int level);
void free_tree(ast_tree *node);
ast_tree *null_check(ast_tree *node);

#endif
