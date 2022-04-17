#ifndef LLVM_H
#define LLVM_H

#include <stdio.h>
#include "symbol_table.h"
#include "syntax_tree.h"

/*

João Carlos Borges Silva Nº 2019216783
Pedro Afonso Ferreira Lopes Martins Nº 2019216826

*/

typedef struct var_ var;
struct var_{
    char *id;
    char *type;
    char *value;
    int count;
    var* next_var;
};


void generateLLVM(ast_tree *node);
void varDeclLLVM(ast_tree *node, char* tipo, int reg);
void funcDeclLLVM(ast_tree *node, char* type);
char* typeLLVM(char* type);
int getFuncParams(ast_tree *node, int start);
void declarationsLLVM(ast_tree *node);

void funcBodyLLVM(ast_tree *node, table *tabela);
int assignLLVM(ast_tree *node, char* name_func, int reg_atual);
void ifLLVM(ast_tree *node);
void forLLVM(ast_tree *node);
int printLLVM(ast_tree *node, char* name_func, int reg);
int callLLVM(ast_tree *node, char* name_func, int reg, int main);

void allocaLLVM(ast_tree **node, int reg, int reg2);
void storeLLVM(ast_tree *what, ast_tree* where, int reg, int reg2);

char* operatorLLVM(ast_tree* node);
int operationLLVM(ast_tree *node, char* name_func, int reg_atual);
int generateDecl(ast_tree* node, char* name_func, int reg);
void returnLLVM(ast_tree *node, char* name_func, int reg);

#endif
