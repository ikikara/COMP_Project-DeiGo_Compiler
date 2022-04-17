#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "symbol_table.h"
#include "syntax_tree.h"
#include "check_semantic.h"

/*

João Carlos Borges Silva Nº 2019216783
Pedro Afonso Ferreira Lopes Martins Nº 2019216826

*/

extern table* all;
extern int int_llvm;
extern int float32_llvm;
extern int string_llvm;
extern int bool_llvm;

extern int flagS;

//Função para meter primeira letra da palavra minuscula
char* tolow(char* string){
    char* type = (char*)malloc(8*sizeof(char));
    strcpy(type, string);
    type[0]+=32;

    return type;
}

//Função para procurar a tabela com nome igual ao name
table* search_table(char* name){
    table* aux = *&all;

    //Enquanto não chegar ao fim da lista de tabelas
    while(aux!=NULL){
        if(!strcmp(aux->name, name)){
            return aux;
        }
        aux = aux->next_table;
    }

    return NULL;
}

/*tipos de returns
1 - tem 1 filho, print, return
2 - tem 2 filhos, =, +, -, *, /, %, atoi, -(unario), +(unario)
3 - tem filhos variados, func(-)
4 - não tem filhos, string, int ou float32
5 - tem 2 filhos, &&, ||, !, ==, !=, <, >, <=, >=
6 - tem blocks, for, if, block
0 - não tem filhos, id
-1 - inválido
*/
int valid_operator(char* operator){
    if(!strcmp(operator, "Id")){
        return 0;
    } 
    else if(!strcmp(operator, "Print") || !strcmp(operator, "Return")){
        return 1;
    }
    else if(!strcmp(operator, "Assign") || !strcmp(operator, "Add") || !strcmp(operator, "Sub") || !strcmp(operator, "Mul") || !strcmp(operator, "Div") || !strcmp(operator, "Mod") || !strcmp(operator, "ParseArgs") || !strcmp(operator, "Minus") || !strcmp(operator, "Plus")){
        return 2;
    }
    else if(!strcmp(operator, "Call")){
        return 3;
    }
    else if(!strcmp(operator, "IntLit") || !strcmp(operator, "RealLit") || !strcmp(operator, "StrLit")){
        return 4;
    }
    else if(!strcmp(operator, "And") || !strcmp(operator, "Or") || !strcmp(operator, "Not") || !strcmp(operator, "Eq") || !strcmp(operator, "Ne") || !strcmp(operator, "Lt") || !strcmp(operator, "Gt") || !strcmp(operator, "Le") || !strcmp(operator, "Ge")){
        return 5;
    }
    else if(!strcmp(operator, "If") || !strcmp(operator, "For") || !strcmp(operator, "Block")){
        return 6;
    }
    else{
        return -1;
    }
}

int compare_types(ast_tree* node, char* type){
    ast_tree* aux = node;
    
    while(aux!=NULL){
        if(strcmp(aux->token->type, type)){
            return 0;
        }
        aux=aux->next_sibling;
    }

    return 1;
}


int anotate_node_symbol(char* name, char* name_func, ast_tree *node){
    symbol* global = (*&all)->first_symbol;
    int flag_global = 0;
    symbol* possible;

    if(valid_operator(node->token->id)==0){
        while(global!=NULL){
            if(!strcmp(global->id, name) && global->is_func==node->token->is_func){
                node->token->type = tolow(global->type);
                flag_global=1;
                possible=*&global;
                break;
            }

            global=global->next_symbol;
        }


        symbol* local = search_table(name_func)->first_symbol;

        while(local!=NULL){
            if(!strcmp(local->id, name) && local->is_func==node->token->is_func){
                if (node->token->line > local->line ){
                    node->token->type = tolow(local->type);
                    local->used=1;
                    return 1;
                }
                if (node->token->line == local->line && node->token->column >= local->column){
                    node->token->type = tolow(local->type);
                    local->used=1;
                    return 1;
                }
            }

            local=local->next_symbol;
        }
        
        if(flag_global==1){
            possible->used=1;
            return 1;
        }
        else{
            node->token->type="undef";
            return 0;
        }
    }

    if(valid_operator(node->token->id)==1){
        ast_tree* aux2 = *&node->first_child;

        if(aux2!=NULL){
            anotate_node_symbol(aux2->value, name_func, aux2);
        }

        if(!strcmp(node->token->id, "Print")){
            if(!strcmp(aux2->token->type, "undef")){
                if(!strcmp(aux2->token->id, "Id")){
                    cannotFindSymbol(aux2, aux2->value);
                }
                incompatibleType(node);
                return 0;
            }
            char* type=aux2->token->type;

            if(!strcmp(type, "int")){
                int_llvm=1;
            }
            else if(!strcmp(type, "float32")){
                float32_llvm=1;
            }
            else if(!strcmp(type, "bool")){
                bool_llvm=1;
            }
            else if(!strcmp(type, "string")){
                string_llvm=1;
            }
            
        }

        if(!strcmp(node->token->id, "Return")){
            char* type = search_table(name_func)->type;
            if(type!=NULL){
                if(aux2!=NULL){
                    if(!strcmp(aux2->token->type, "undef") || strcmp(aux2->token->type, tolow(type))){
                        if(!strcmp(aux2->token->id, "Id")){
                            cannotFindSymbol(aux2, aux2->value);
                        }
                        incompatibleType(node);
                        return 0;
                    }
                }
                else{
                    incompatibleType(node);
                    return 0;
                }
            }
            else{
                if(aux2!=NULL){
                        if(!strcmp(aux2->token->id, "Id")){
                            cannotFindSymbol(aux2, aux2->value);
                        }
                        incompatibleType(node);
                        return 0;
                    
                }
            }
        }


        return 1;
    }

    if(valid_operator(node->token->id)==2){
        ast_tree* aux2 = *&node->first_child;

        while(aux2!=NULL){
            anotate_node_symbol(aux2->value, name_func, aux2);
            aux2=aux2->next_sibling;
        }

        char* type;
        type = node->first_child->token->type;
        ast_tree* errors = node->first_child;

        if(!strcmp(node->token->id, "Minus") || !strcmp(node->token->id, "Plus")){
            if(!strcmp(type, "int")){
                node->token->type=type;
                return 1;
            }
            else if(!strcmp(type, "float32")){
                node->token->type=type;
                return 1;
            }
            else{
                node->token->type="undef";
                operatorNotApplied(node);
                return 0;
            }
        } 
        else if(!strcmp(node->token->id, "Mod")){
            if(!compare_types(node->first_child,  "int") || (!strcmp(type, "undef") && !strcmp(node->first_child->next_sibling->token->type, "undef"))){
                node->token->type="undef";
                operatorNotApplied(node);
                
                while(errors!=NULL){
                    if(!strcmp(errors->token->id,"Id") && !strcmp(errors->token->type,"undef")){
                        cannotFindSymbol(errors, errors->value);
                    }
                    errors=errors->next_sibling;
                }
                return 0;
            }
            else{
                node->token->type=type;
                return 1;
            }
        }
        else if(!strcmp(node->token->id, "ParseArgs")){
            if(!strcmp(type, "int") && (!strcmp(node->first_child->next_sibling->token->type, "int") || !strcmp(node->first_child->next_sibling->token->type, "string"))){
                node->token->type="int";
                return 1;
            }
            else{
                node->token->type="undef";
                operatorNotApplied(node);
                
                while(errors!=NULL){
                    if(!strcmp(errors->token->id,"Id") && !strcmp(errors->token->type,"undef")){
                        cannotFindSymbol(errors, errors->value);
                    }
                    errors=errors->next_sibling;
                }
                return 0;
            }
        }
        else{
            if(!compare_types(node->first_child, type) || ((!strcmp(type, "undef") || !strcmp(node->first_child->next_sibling->token->type, "undef")))){
                node->token->type="undef";
                operatorNotApplied(node);
                
                while(errors!=NULL){
                    if(!strcmp(errors->token->id,"Id") && !strcmp(errors->token->type,"undef")){
                        cannotFindSymbol(errors, errors->value);
                    }
                    errors=errors->next_sibling;
                }
                return 0;
            }
            else if( (!strcmp(type, "string") || !strcmp(node->first_child->next_sibling->token->type, "string")) && strcmp(node->token->id, "Assign")!=0 && strcmp(node->token->id, "Add")!=0){
                node->token->type="undef";
                operatorNotApplied(node);

                while(errors!=NULL){
                    if(!strcmp(errors->token->id,"Id") && !strcmp(errors->token->type,"undef")){
                        cannotFindSymbol(errors, errors->value);
                    }
                    errors=errors->next_sibling;
                }

                return 0;
            }
            else if( (!strcmp(type, "bool") || !strcmp(node->first_child->next_sibling->token->type, "bool")) && strcmp(node->token->id, "Assign")!=0 ){
                node->token->type="undef";
                operatorNotApplied(node);

                while(errors!=NULL){
                    if(!strcmp(errors->token->id,"Id") && !strcmp(errors->token->type,"undef")){
                        cannotFindSymbol(errors, errors->value);
                    }
                    errors=errors->next_sibling;
                }

                return 0;
            }
            else{
                node->token->type=type;
                return 1;
            }
        }
        
        
    }

    if(valid_operator(node->token->id)==3){
        table* aux;

        ast_tree* principal_node = *&node->first_child;
        ast_tree* aux2 = *&node->first_child->next_sibling;
        int nrChilds = nrChilds_node(node)-1;
        char* parameters= (char*)malloc((nrChilds*9+2)*sizeof(char));
        int count=0;
        strcpy(parameters,"()");

        while(aux2!=NULL){
            anotate_node_symbol(aux2->value, name_func, aux2);
            
            if(nrChilds==1){
                strcpy(parameters, "(");
                strcat(parameters, aux2->token->type);
                strcat(parameters, ")");

            }
            else{
                if(count==0 || count==nrChilds-1){
                    if(count==0){
                        strcpy(parameters, "(");
                        strcat(parameters, aux2->token->type);
                        strcat(parameters, ",");

                    }
                    else{
                        strcat(parameters, aux2->token->type);
                        strcat(parameters, ")");

                    }
                }
                else{

                    strcat(parameters, aux2->token->type);
                    strcat(parameters, ",");
                }
            }
            //strcat(parameters, aux2->token->type);
            count++;
            aux2=aux2->next_sibling;
        }

        char* caseError=(char*)malloc(((nrChilds*9+2)*sizeof(char) + strlen(node->first_child->value) + 1)*sizeof(char));
        strcat(caseError, node->first_child->value);
        strcat(caseError, parameters);

        //se a função não existe
        if((aux=search_table(*&node->first_child->value))==NULL){
            node->token->type=node->first_child->token->type="undef";
            cannotFindSymbol(node->first_child, caseError);
            return 0;
        }
        //se a função existe
        else{
            char* real_parameters=(char*)malloc(1000*sizeof(char));
            sprintf(real_parameters, "(%s)", search_table(principal_node->value)->params);

            //se a função existe os parametros batem certo com os tipos
            if(!strcmp(parameters, real_parameters)){
                principal_node->token->type=real_parameters;

                if(aux->type == NULL){
                    node->token->type="none";
                }
                else{
                    node->token->type=tolow(aux->type);
                }
                return 1;
            }
            //se a função existe os parametros não batem certo com os tipos
            else{
                node->token->type=node->first_child->token->type="undef";
                cannotFindSymbol(node->first_child, caseError);
                return 0;
            }
        }
        
        
    }

    if(valid_operator(node->token->id)==4){
        if(!strcmp(node->token->id, "IntLit")){
            node->token->type = "int";
            return 1;
        }
        else if(!strcmp(node->token->id, "RealLit")){
            node->token->type = "float32";
            return 1;
        }
        else if(!strcmp(node->token->id, "StrLit")){
            node->token->type = "string";
            return 1;
        }
    }
    
    if(valid_operator(node->token->id)==5){
        ast_tree* aux2 = *&node->first_child;

        while(aux2!=NULL){
            anotate_node_symbol(aux2->value, name_func, aux2);
            
            aux2=aux2->next_sibling;
        }

        char* type;
        if(!strcmp(node->token->id, "And") || !strcmp(node->token->id, "Or") || !strcmp(node->token->id, "Not")){
            type = "bool";
        } 
        else{
            type = node->first_child->token->type;
        }

            
        ast_tree* errors = node->first_child;
        if((!strcmp(node->token->id, "Lt") || !strcmp(node->token->id, "Le") || !strcmp(node->token->id, "Gt") || !strcmp(node->token->id, "Ge"))){
            if( !strcmp(type, "bool") && !strcmp(node->first_child->next_sibling->token->type, "bool")){
                node->token->type="undef";
                operatorNotApplied(node);
                node->token->type="bool";

                return 0;
            }
            else{
                if(!compare_types(node->first_child, type) ){
                node->token->type="undef";
                operatorNotApplied(node);

                while(errors!=NULL){
                    if(!strcmp(errors->token->id,"Id") && !strcmp(errors->token->type,"undef")){
                        cannotFindSymbol(errors, errors->value);
                    }
                    errors=errors->next_sibling;
                }
                
                    node->token->type="bool";
                    return 0;
                }
                else{
                    node->token->type="bool";
                    return 1;
                }
            }
        }
        else{
            if(!compare_types(node->first_child, type) ){
                node->token->type="undef";
                operatorNotApplied(node);

                while(errors!=NULL){
                    if(!strcmp(errors->token->id,"Id") && !strcmp(errors->token->type,"undef")){
                        cannotFindSymbol(errors, errors->value);
                    }
                    errors=errors->next_sibling;
                }
                
                node->token->type="bool";
                return 0;
            }
            else{
                node->token->type="bool";
                return 1;
            }
        }
        
    }

    if(valid_operator(node->token->id)==6){
        ast_tree* aux2 = *&node->first_child;

        while(aux2!=NULL){
            if(!strcmp(aux2->value, "")){
                anotate_node_symbol("", name_func, aux2);
            }
            else{
                anotate_node_symbol(aux2->value, name_func, aux2);
            }
            
            aux2=aux2->next_sibling;
        }

        if(node->first_child){
            if((!strcmp(node->token->id, "For") || !strcmp(node->token->id, "If")) && strcmp(node->first_child->token->type, "bool") && strcmp(node->first_child->token->id, "Block")!=0){
                if(!strcmp(node->first_child->token->id, "Id") && !strcmp(node->first_child->token->type, "undef")){
                    cannotFindSymbol(node->first_child, node->first_child->value);
                }
                incompatibleType(node);
            } 
        }

        return 1;
    }

    return 0;
}


void anotate_node_func(ast_tree** node){
    ast_tree* params;
    if(!strcmp((*node)->first_child->first_child->next_sibling->token->id,"FuncParams")){
        params=(*node)->first_child->first_child->next_sibling->first_child;
    }
    else{
        params=(*node)->first_child->first_child->next_sibling->next_sibling->first_child;
    }
    ast_tree* func_body = (*node)->first_child->next_sibling->first_child;
    char* name_func = (*node)->first_child->first_child->value;

    while(params!=NULL){
        if(params->first_child->next_sibling->token->is_defined==0){
            symbolAlreadyDefined(params->first_child->next_sibling);
        }
        
        params=params->next_sibling;
    }

    while(func_body!=NULL){
        anotate_node_symbol("", name_func, func_body);
        if(!strcmp(func_body->token->id, "VarDecl") && func_body->first_child->next_sibling->token->is_defined==0){
            symbolAlreadyDefined(func_body->first_child->next_sibling);
        }

        func_body=func_body->next_sibling;
    }
}

void anotate_tree(ast_tree** node){
    ast_tree* aux = (*node)->first_child;

    while(aux!=NULL){
        if(!strcmp(aux->token->id,"FuncDecl")){
            if(aux->first_child->first_child->token->is_defined==0){
                symbolAlreadyDefined(aux->first_child->first_child);
            }
            else{
                anotate_node_func(&aux);
            }
            
        }
        if(!strcmp(aux->token->id, "VarDecl") && aux->first_child->next_sibling->token->is_defined==0){
            symbolAlreadyDefined(aux->first_child->next_sibling);
        }

        aux=aux->next_sibling;
    }
}



//Função para criar uma estrutura simbolo
symbol* new_symbol(char *id, char *type, char* whatisit){
    symbol* new_symb = (symbol *)malloc(sizeof(symbol));

    new_symb->id = id;
    new_symb->type = type;
    new_symb->whatisit = whatisit;
    if(!strcmp(whatisit, "func")){
        new_symb->is_func=1;
    }
    else{
        new_symb->is_func=0;
    }
    new_symb->next_symbol = NULL;
    new_symb->used = 0;
    new_symb->line = 0;
    new_symb->column = 0;
    new_symb->is_defined = 0;
    new_symb->register_tab = -1;

    return new_symb;
}

//Função para adicionar variaveis locais (e parametros se existirem)
//list -> sitio onde se vai adicionar as variaveis locais (e parametros se existirem)
//node -> sitio de onde vêm os parametros, se existirem
//node2 -> sitio de onde vêm as variáveis locais, se existirem
void add_local_symbols(symbol** list, ast_tree** node, ast_tree** node2, char* name_func){
    symbol *aux = *list;
    ast_tree *aux2 = *node;
    ast_tree *aux3 = *node2;
    //percorrer os parametros
    while(aux2!=NULL){
        if(existince(aux2->first_child->next_sibling->value, name_func, 0)==0){
            aux->next_symbol=new_symbol(aux2->first_child->next_sibling->value, aux2->first_child->token->id, "param");
            aux->next_symbol->line=aux2->first_child->next_sibling->token->line;
            aux->next_symbol->column=aux2->first_child->next_sibling->token->column;
            
            aux->next_symbol->is_defined=1;
            aux2->first_child->next_sibling->token->is_defined=1;

            aux=aux->next_symbol;
        }
        else{
            aux2->first_child->next_sibling->token->is_defined=0;
        }
        aux2=aux2->next_sibling;
    }
    
    //percorrer as variaveis locais
    while(aux3!=NULL){
        if(!strcmp(aux3->token->id,"VarDecl") ){
            if(existince(aux3->first_child->next_sibling->value, name_func, 0)==0){
                aux->next_symbol=new_symbol(aux3->first_child->next_sibling->value, aux3->first_child->token->id, "var");
                aux->next_symbol->line=aux3->first_child->next_sibling->token->line;
                aux->next_symbol->column=aux3->first_child->next_sibling->token->column;

                aux->next_symbol->is_defined=1;
                aux3->first_child->next_sibling->token->is_defined=1;
                aux=aux->next_symbol;
            }
            else{
                aux3->first_child->next_sibling->token->is_defined=0;
            }
        }

        aux3=aux3->next_sibling;
    }
}

//Função para adicionar variaveis globais
//list -> sitio onde se vai adicionar as variaveis globais
//node -> sitio de onde vêm as variáveis globais 
void add_global_symbols(symbol** list, ast_tree** node){
    symbol *aux = *list;
    ast_tree *aux2 = *node;

    //percorrer as variaveis globais
    while(aux2!=NULL){
        //se for uma função
        if(!strcmp(aux2->token->id,"FuncDecl")){
            if(existince(aux2->first_child->first_child->value, "", 1)==0){
                //se a função tiver tipo
                if(nrChilds_node(aux2->first_child)==3){
                    aux->next_symbol=new_symbol(aux2->first_child->first_child->value, aux2->first_child->first_child->next_sibling->token->id, "func");
                }
                //se a função não tiver tipo
                else{
                    aux->next_symbol=new_symbol(aux2->first_child->first_child->value, "None", "func");
                }
            
                aux->next_symbol->line= aux2->first_child->first_child->token->line;
                aux->next_symbol->column= aux2->first_child->first_child->token->column;
                aux->next_symbol->is_defined=1;
                
                aux2->first_child->first_child->token->is_defined=1;

                //aux->next_symbol->params= search_table(aux2->first_child->first_child->value)->params;
    
                aux=aux->next_symbol;
            }     
            else{
                aux2->first_child->next_sibling->token->is_defined=0;
            }       
        }

        //se for uma variavel
        else{
            /*if(!strcmp(aux2->token->id,"Assign")){
                cannotFindSymbol(tablita, "Global", aux2->first_child);
            }*/
            if(existince(aux2->first_child->next_sibling->value, "", 1)==0){
                aux->next_symbol=new_symbol(aux2->first_child->next_sibling->value, aux2->first_child->token->id, "var");
                aux->next_symbol->line= aux2->first_child->next_sibling->token->line;
                aux->next_symbol->column= aux2->first_child->next_sibling->token->column;
                aux->next_symbol->is_defined=1;
                aux2->first_child->next_sibling->token->is_defined=1;
                
                aux=aux->next_symbol;
            }
            else{
                aux2->first_child->next_sibling->token->is_defined=0;
            }

        }

        aux2=aux2->next_sibling;
    }
}


void add_symbols(table** tablita, ast_tree** node, int level){
    table *aux = *tablita;
    ast_tree *aux2 = *node;

    ast_tree *list_params;
    ast_tree *list_vars; 
    
    if(level==0){
        aux2=aux2->first_child;

        if(!strcmp(aux2->token->id,"FuncDecl")){
            if(nrChilds_node(aux2->first_child)==3){
                aux->first_symbol=new_symbol(aux2->first_child->first_child->value, aux2->first_child->first_child->next_sibling->token->id, "func");
            }
            else{
                aux->first_symbol=new_symbol(aux2->first_child->first_child->value, "None", "func");
            }
            aux->first_symbol->line= aux2->first_child->first_child->token->line;
            aux->first_symbol->column= aux2->first_child->first_child->token->column;

            //aux->first_symbol->params= search_table(aux2->first_child->first_child->value)->params;

            aux2->first_child->first_child->token->is_defined=1;
        }
        else{   
            aux->first_symbol=new_symbol(aux2->first_child->next_sibling->value, aux2->first_child->token->id, "var");
            aux->first_symbol->line= aux2->first_child->next_sibling->token->line;
            aux->first_symbol->column= aux2->first_child->next_sibling->token->column;

            aux2->first_child->next_sibling->token->is_defined=1;
        }

        aux->first_symbol->is_defined=1;

        add_global_symbols(&aux->first_symbol, &aux2->next_sibling);
    }
    else if(level==1){
        if(nrChilds_node(aux2->first_child)==3){
            list_params= *&aux2->first_child->first_child->next_sibling->next_sibling;

            list_vars = *&aux2->first_child->next_sibling->first_child;

            if(list_params->first_child!=NULL){
                list_params=list_params->first_child;
                aux->first_symbol=new_symbol(list_params->first_child->next_sibling->value, list_params->first_child->token->id, "param");
                aux->first_symbol->line=list_params->first_child->next_sibling->token->line;
                aux->first_symbol->column=list_params->first_child->next_sibling->token->column;            
                aux->first_symbol->is_defined=1;
                list_params->first_child->next_sibling->token->is_defined=1;

                add_local_symbols(&aux->first_symbol, &list_params->next_sibling, &list_vars, aux2->first_child->first_child->value);
            }
            else{
                if(list_vars!=NULL){
                    while(list_vars!=NULL){
                        if(strcmp(list_vars->token->id,"VarDecl")==0){
                            aux->first_symbol=new_symbol(list_vars->first_child->next_sibling->value, list_vars->first_child->token->id, "var");
                            aux->first_symbol->line=list_vars->first_child->next_sibling->token->line;
                            aux->first_symbol->column=list_vars->first_child->next_sibling->token->column;
                            aux->first_symbol->is_defined=1;
                            list_vars->first_child->next_sibling->token->is_defined=1;

                            add_local_symbols(&aux->first_symbol, &list_params->first_child, &list_vars->next_sibling, aux2->first_child->first_child->value);
                            break;
                        }
                        list_vars=list_vars->next_sibling;
                    }
                    
                    
                }
            }
        }
        else if(nrChilds_node(aux2->first_child)==2){
            list_params= *&aux2->first_child->first_child->next_sibling;
            
            list_vars = *&aux2->first_child->next_sibling->first_child;
            
            if(list_params->first_child!=NULL){
                list_params=list_params->first_child;
                aux->first_symbol=new_symbol(list_params->first_child->next_sibling->value, list_params->first_child->token->id, "param");
                aux->first_symbol->line=list_params->first_child->next_sibling->token->line;
                aux->first_symbol->column=list_params->first_child->next_sibling->token->column;  
                aux->first_symbol->is_defined=1;
                list_params->first_child->next_sibling->token->is_defined=1;
                
                add_local_symbols(&aux->first_symbol, &list_params->next_sibling, &list_vars, aux2->first_child->first_child->value);
            }
            else{
                if(list_vars!=NULL){
                    while(list_vars!=NULL){
                        if(strcmp(list_vars->token->id,"VarDecl")==0){
                            aux->first_symbol=new_symbol(list_vars->first_child->next_sibling->value, list_vars->first_child->token->id, "var");
                            aux->first_symbol->line=list_vars->first_child->next_sibling->token->line;
                            aux->first_symbol->column=list_vars->first_child->next_sibling->token->column;
                            aux->first_symbol->is_defined=1;
                            list_vars->first_child->next_sibling->token->is_defined=1;

                            add_local_symbols(&aux->first_symbol, &list_params->first_child, &list_vars->next_sibling, aux2->first_child->first_child->value);
                            break;
                        }
                        list_vars=list_vars->next_sibling;
                    }
                    
                }
            }
        }
    }
}


table* new_table(char *name, char *name2, char* name3, ast_tree** node){
    table *new_table = (table *)malloc(sizeof(table));
    char* string;
    char* parameters;
    
    int count;

    if(node!=NULL){
        ast_tree* aux = *node;
        if(nrChilds_node(aux->first_child)==3){
            count = nrChilds_node(aux->first_child->first_child->next_sibling->next_sibling);
            parameters = (char*)malloc((count*9)*sizeof(char));
            
            aux=aux->first_child->first_child->next_sibling->next_sibling->first_child;

            if(aux!=NULL){
                strcpy(parameters, tolow(aux->first_child->token->id));
                
                aux=aux->next_sibling;

                while(aux!=NULL){
                    strcat(parameters, ",");
                    strcat(parameters, tolow(aux->first_child->token->id));
                    aux=aux->next_sibling;
                }
            }
            else{
                parameters="";
            }

        }
        else{
            count = nrChilds_node(aux->first_child->first_child->next_sibling);
            parameters = (char*)malloc((count*9)*sizeof(char));
            
            aux=aux->first_child->first_child->next_sibling->first_child;
            
            if(aux!=NULL){
                strcpy(parameters, tolow(aux->first_child->token->id));
                
                aux=aux->next_sibling;

                while(aux!=NULL){
                    strcat(parameters, ",");
                    strcat(parameters, tolow(aux->first_child->token->id));
                    aux=aux->next_sibling;
                }
            }
            else{
                parameters="";
            }
        }
    }

    if(name2==NULL && name3==NULL){ 
        new_table->name = name;

        new_table->nametoprint = name;
    }
    else{
        const int size = strlen(name2) + strlen(name);
        string = (char*)(malloc(((1+count)*9 + size)*sizeof(char)));
        sprintf(string, "%s %s(", name, name2);

        new_table->params = parameters;
        strcat(string, parameters);
        strcat(string, ")");
        new_table->nametoprint = string;

        new_table->type = name3; 
        
        new_table->name = name2;
    }  

    new_table->next_table=NULL;
    new_table->first_symbol=NULL;

    return new_table;
}

void add_tables(table** tablita, ast_tree** node){
    table *aux = *tablita;
    ast_tree *aux2 = *node;

    if(!strcmp(aux2->first_child->token->id, "FuncDecl") && aux2->first_child!=NULL){
        if(strcmp(aux2->first_child->first_child->first_child->next_sibling->token->id, "FuncParams")){
            aux->next_table=new_table("Function", aux2->first_child->first_child->first_child->value, aux2->first_child->first_child->first_child->next_sibling->token->id, &aux2->first_child);
        }
        else{
            aux->next_table=new_table("Function", aux2->first_child->first_child->first_child->value, NULL, &aux2->first_child);
        }

        aux = aux->next_table;
    }
    aux2 = aux2->first_child;


    while(aux2->next_sibling != NULL){
        if(!strcmp(aux2->next_sibling->token->id, "FuncDecl") && aux2->next_sibling->first_child->first_child->token->is_defined){
            if(strcmp(aux2->next_sibling->first_child->first_child->next_sibling->token->id, "FuncParams")){
                aux->next_table=new_table("Function", aux2->next_sibling->first_child->first_child->value, aux2->next_sibling->first_child->first_child->next_sibling->token->id, &aux2->next_sibling);
            }
            else{
                aux->next_table=new_table("Function", aux2->next_sibling->first_child->first_child->value, NULL, &aux2->next_sibling);
            }
            
            aux = aux->next_table;
        }
        aux2 = aux2->next_sibling;
    }
}

void create_tables(ast_tree *node, table* tablita, int level){
    if(level==0){
        if(node->first_child!=NULL){
            add_symbols(&tablita, &node, 0);
            add_tables(&tablita, &node);
            table* aux= *&tablita->next_table;
            symbol* aux_list = *&tablita->first_symbol;
            ast_tree* aux2= *&node->first_child;

            while(aux2!=NULL){
                if(!strcmp(aux2->token->id, "FuncDecl")){
                    if(aux2->first_child->first_child->token->is_defined){
                       create_tables(aux2, aux, 1);
                       aux=aux->next_table;
                    }
                }
                aux2=aux2->next_sibling;
            }   

            while(aux_list!=NULL){
                if(!strcmp(aux_list->whatisit, "func")){
                    aux_list->params= search_table(aux_list->id)->params;
                }
                aux_list=aux_list->next_symbol;
            }
        }

    }
    else if(level==1){
        add_symbols(&tablita, &node, 1);
    }
}


void print_symbol(symbol* symb){
    printf("%s\t",  symb->id);
    if(symb->whatisit!=NULL && !(strcmp(symb->whatisit, "func"))){
        printf("(");
        if(symb->params!=NULL){
            printf("%s", symb->params);
        }
        printf(")");
    }
    char* type = tolow(symb->type);
    
    printf("\t%s", type);
    if(symb->whatisit!=NULL && !(strcmp(symb->whatisit, "param"))){
        printf("\t%s", symb->whatisit);
    }
    printf("\n");
}

void print_tables(table* first_table){
    if(flagS==1){
        puts("");
    }

    while(first_table!=NULL){
        printf("===== %s Symbol Table =====\n", first_table->nametoprint);
        if(strcmp(first_table->nametoprint,"Global")){
            if(first_table->type==NULL){
                printf("return\t\tnone\n");
            }
            else{
                printf("return\t\t%s\n", tolow(first_table->type));
            }
        }
        if(first_table->first_symbol!=NULL){
            print_symbol(first_table->first_symbol);

            while(first_table->first_symbol->next_symbol!=NULL){
                print_symbol(first_table->first_symbol->next_symbol);
                first_table->first_symbol->next_symbol=first_table->first_symbol->next_symbol->next_symbol;
            }
        }
        
        printf("\n");
        first_table=first_table->next_table;
    }
}


void free_table(table* first_table){
    if(first_table!=NULL){
        free_table(first_table->next_table);
    }
    
    free(first_table);    
}
