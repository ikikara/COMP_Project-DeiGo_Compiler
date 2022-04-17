//Funcao auxiliar para traduzir o tipo de operador em questao
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "symbol_table.h"
#include "syntax_tree.h"

/*

João Carlos Borges Silva Nº 2019216783
Pedro Afonso Ferreira Lopes Martins Nº 2019216826

*/

extern table* all;
extern int flagS;

char* getOperator(char* op){
    if(strcmp(op,"Add")==0){
        return "+";
    }
    else if(strcmp(op,"Sub")==0){
        return "-";
    }
    else if(strcmp(op,"Mul")==0){
        return "*";
    }
    else if(strcmp(op,"Div")==0){
        return "/";
    }
    else if(strcmp(op,"Mod")==0){
        return "%";
    }
    else if(strcmp(op,"And")==0){
        return "&&";
    }
    else if(strcmp(op,"Or")==0){
        return "||";
    }
    else if(strcmp(op,"Eq")==0){
        return "==";
    }
    else if(strcmp(op,"Ne")==0){
        return "!=";
    }
    else if(strcmp(op,"Lt")==0){
        return "<";
    }
    else if(strcmp(op,"Gt")==0){
        return ">";
    }
    else if(strcmp(op,"Le")==0){
        return "<=";
    }
    else if(strcmp(op,"Ge")==0){
        return ">=";
    }
    else if(strcmp(op,"Not")==0){
        return "!";
    }
    else if(strcmp(op,"Minus")==0){
        return "-";
    }
    else if(strcmp(op,"Plus")==0){
        return "+";
    }
    else if(strcmp(op,"Assign")==0){
        return "=";   
    }
    else if(strcmp(op,"ParseArgs")==0){
        return "strconv.Atoi";
    }
    else if(strcmp(op,"Print")==0){
        return "fmt.Println";
    } 
    else{
        return NULL;
    }
}

int existince(char* name, char* name_func, int global_f){
    symbol* global = (*&all)->first_symbol;

    if(global_f==1){
        while(global!=NULL){
            if(!strcmp(global->id, name)){
                return 1;
            }

            global=global->next_symbol;
        }

        return 0;
    }

    if(global_f==0){
        symbol* local = search_table(name_func)->first_symbol;

        while(local!=NULL){
            if(!strcmp(local->id, name)){
                return 1;
            }

            local=local->next_symbol;
        }
        
        return 0;
    }

    return 0;
}

void symbolAlreadyDefined(ast_tree *node){
    flagS=1;
    
    printf("Line %d, column %d: Symbol %s already defined\n", node->token->line, node->token->column, node->value);                         
}

void cannotFindSymbol(ast_tree *node, char* toprint){
    flagS=1;
    
    if (!strcmp(node->token->type,"undef")){
        printf("Line %d, column %d: Cannot find symbol %s\n", node->token->line, node->token->column, toprint);
    }
}

void operatorNotApplied(ast_tree *node){
    flagS=1;
    char* error;

    if (!strcmp(node->token->type,"undef")){
        if(nrChilds_node(node)==2){
            error = (char*)malloc((100+3+3+2)*sizeof(char*) + sizeof(node->first_child->token->type) +  sizeof(node->first_child->next_sibling->token->type));
            int line=node->token->line;
            int column=node->token->column;
            char* operator=getOperator(node->token->id);

            if(!strcmp(node->first_child->token->type,"")){
                node->first_child->token->type="undef";
            }
            if(!strcmp(node->first_child->next_sibling->token->type,"")){
                node->first_child->next_sibling->token->type="undef";
            }

            if(!strcmp(operator, "strconv.Atoi")){
                line=node->first_child->next_sibling->token->line;
                column=node->first_child->next_sibling->token->column-21;
            }


            sprintf(error, "Line %d, column %d: Operator %s cannot be applied to types %s, %s", line, column, operator, node->first_child->token->type, node->first_child->next_sibling->token->type);
        }
        else{
            error = (char*)malloc((100+3+3+2)*sizeof(char*) + sizeof(node->first_child->token->type));
            if(!strcmp(node->first_child->token->type,"")){
                node->first_child->token->type="undef";
            }

            sprintf(error, "Line %d, column %d: Operator %s cannot be applied to type %s", node->token->line, node->token->column, getOperator(node->token->id), node->first_child->token->type);
        }
        puts(error);
        free(error);
    }
    
}

void incompatibleType(ast_tree *node){
    flagS=1;
    
    if(!strcmp(node->token->id, "Print")){
        printf("Line %d, column %d: Incompatible type %s in %s statement\n", node->first_child->token->line, node->first_child->token->column, node->first_child->token->type, getOperator(node->token->id));
    }
    else if(!strcmp(node->token->id, "Return") && node->first_child==NULL){
        printf("Line %d, column %d: Incompatible type none in %s statement\n", node->token->line, node->token->column, tolow(node->token->id));
    }

    else{
        printf("Line %d, column %d: Incompatible type %s in %s statement\n", node->first_child->token->line, node->first_child->token->column, node->first_child->token->type, tolow(node->token->id));
    }
}

void symbolNeverUsed(){
    //Tabela de simbolos global
    table* case_errors= *&all->next_table;

    while(case_errors!=NULL){
        symbol* firstSymb = case_errors->first_symbol;

        while(firstSymb!=NULL){
            if(firstSymb->used==0 && !strcmp(firstSymb->whatisit,"var")){
                flagS=1;
                printf("Line %d, column %d: Symbol %s declared but never used\n", firstSymb->line, firstSymb->column, firstSymb->id);
            }

            firstSymb=firstSymb->next_symbol;
        }
        
        case_errors=case_errors->next_table;
    }

}
