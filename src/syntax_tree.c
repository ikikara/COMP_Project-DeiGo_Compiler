#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "syntax_tree.h"
//#include "check_semantic.h"

/*

João Carlos Borges Silva Nº 2019216783
Pedro Afonso Ferreira Lopes Martins Nº 2019216826

*/

char* haveValue(char* op){
    if(strcmp(op,"Add")==0){
        return "+";
    }else if(strcmp(op,"Sub")==0){
        return "-";
    }else if(strcmp(op,"Mul")==0){
        return "*";
    }else if(strcmp(op,"Div")==0){
        return "/";
    }else if(strcmp(op,"Mod")==0){
        return "%%";
    }else if(strcmp(op,"And")==0){
        return "&&";
    }else if(strcmp(op,"Or")==0){
        return "||";
    }else if(strcmp(op,"Eq")==0){
        return "==";
    }else if(strcmp(op,"Ne")==0){
        return "!=";
    }else if(strcmp(op,"Lt")==0){
        return "<";
    }else if(strcmp(op,"Gt")==0){
        return ">";
    }else if(strcmp(op,"Le")==0){
        return "<=";
    }else if(strcmp(op,"Ge")==0){
        return ">=";
    }else if(strcmp(op,"Not")==0){
        return "!";
    }else if(strcmp(op,"Minus")==0){
        return "-";
    }else if(strcmp(op,"Plus")==0){
        return "+";
    }else if(strcmp(op,"Assign")==0){
        return "=";   
    }else if(strcmp(op,"Return")==0){
        return "return";   
    } else{
        return NULL;
    }
}

int nrChilds_node (ast_tree* node){
    int count=0;
    ast_tree* root = node;

    if(root->first_child==NULL || root ==NULL){
        return 0;
    }
    else{
        count++;
        root=root->first_child;
        while (root->next_sibling != NULL) {
            count++;
            root = root->next_sibling;
        }
        return count;
    }
}

int nrChilds_blocks (ast_tree* node){
    int count=1;
    ast_tree* root = node;

    
    while (root->next_sibling != NULL) {
        count++;
        root = root->next_sibling;
    }
    return count;
}

token* new_token(char* string, int line, int column){
    token *token_struct = (token *)malloc(sizeof(token));

    token_struct->id = string;
    token_struct->line=line;
    token_struct->column=column;
    token_struct->type="";
    token_struct->is_func=0;

    return token_struct;
}

ast_tree *ast_node(char* name, token* id, char* value) {
    ast_tree *node = (ast_tree *)malloc(sizeof(ast_tree));

    if(!strcmp(value,"")){
        node->token=new_token(name, 0, 0);
        node->value="";
    }
    else{ 
        node->token=id;
        node->value=node->token->id;
        node->token->id=name;
    }

    node->first_child = NULL;
    node->next_sibling = NULL;
    
    return node;
}

void add_childs(ast_tree *root, int nargs, ...){
    va_list args;                                                       //define uma lista de argumentos
    va_start(args, nargs);                                              //começa a lista de argumentos dos ...
    ast_tree* current = root->first_child = va_arg(args, ast_tree*);    //define que o first_child do nó dado passado como parametro 
                                                                        //tem o valor do 1º argumento dos ... e define o nó onde se encontra atualmente
    
    

    ast_tree* child = NULL;                                             //cria um nó auxiliar para criar o child

    for(int i=0; i<nargs-1; i++){
        child = va_arg(args, ast_tree*);                                //percorrer os proximos argumentos dos ...
        for(ast_tree* c = child; c; c = c->next_sibling){
            current->next_sibling = c;
            current = current->next_sibling;
        }
    }

    va_end(args);
}

void add_siblings(ast_tree *root, int nargs, ...){
    va_list args;
    va_start(args, nargs);
    ast_tree* sibling;
    ast_tree* current = root;

    while (current->next_sibling != NULL) {
        current = current->next_sibling;
    }

    for(int i=0; i<nargs; ++i){
        sibling = va_arg(args, ast_tree*);

        current->next_sibling = sibling;
        current = current->next_sibling;
    }
    
    va_end(args);
}

void print_tree(ast_tree *node, int level){
    for (int i = 0; i < level; i++){
        printf("..");
    }
    

    if (strcmp(node->value,"") ){
        if(haveValue(node->token->id)==NULL){
            printf("%s(%s)", node->token->id, node->value);
        }
        else{
            printf("%s", node->token->id);
        }
    } 
    else{
        printf("%s", node->token->id);
    }

    if (strcmp(node->token->type,"") && strcmp(node->token->type,"none") ){
        printf(" - %s\n", node->token->type);
    }
    else{
        printf("\n");
    }
    
    if (node->first_child != NULL){
        print_tree(node->first_child, level + 1);
    }
    if (node->next_sibling != NULL){
        print_tree(node->next_sibling, level);
    }
}

void free_tree(ast_tree *node){
    if (node != NULL) {
        if (node->first_child != NULL){
            free_tree(node->first_child);
        }
        if (node->next_sibling != NULL){
            free_tree(node->next_sibling);
        }

        
        free(node->token);
        free(node);
    }
    
}

