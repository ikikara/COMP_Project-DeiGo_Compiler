#include "LLVM.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//Var localvars[1024];
//Var globalvars[1024];

/*

João Carlos Borges Silva Nº 2019216783
Pedro Afonso Ferreira Lopes Martins Nº 2019216826

*/

extern table* all;
extern int int_llvm;
extern int float32_llvm;
extern int string_llvm;
extern int bool_llvm;

int load=0;
int string_count=1;
int regs_changed=0;
//var* list_vars;

char* operatorLLVM(ast_tree* node){
    char* operator = node->token->id;
    char* operator_type = node->token->type;

	if (strcmp(operator, "Add") == 0){
        if(!strcmp(operator_type, "int")){        
    		return "add";
        }
        else{
            return "fadd";
        }
	} else if (strcmp(operator, "Sub") == 0){
		if(!strcmp(operator_type, "int")){        
    		return "sub";
        }
        else{
            return "fsub";
        }
	} else if (strcmp(operator, "Mul") == 0){
		if(!strcmp(operator_type, "int")){        
    		return "mul";
        }
        else{
            return "fmul";
        }
	} else if (strcmp(operator, "Div") == 0){
		if(!strcmp(operator_type, "int")){        
    		return "sdiv";
        }
        else{
            return "fdiv";
        }
	} else if (strcmp(operator, "Mod") == 0){
		return "srem";
	} else if (strcmp(operator, "And") == 0){
		return "and";
	} else if (strcmp(operator, "Or") == 0){
		return "or";
	} else if (strcmp(operator, "Not") == 0){
		return "fneg";
	} else if (strcmp(operator, "Eq") == 0){
		return "eq";
	} else if (strcmp(operator, "Ne") == 0){
		return "ne";
	} else if (strcmp(operator, "Lt") == 0){
		return "slt";
	} else if (strcmp(operator, "Gt") == 0){
		return "sgt";
	} else if (strcmp(operator, "Le") == 0){
		return "sle";
	} else if (strcmp(operator, "Ge") == 0){
		return "sge";
	}
	return NULL;
}

int nrEsSeq(char* string){
    int i=0;
    int count=0;

    while(string[i]!='\0'){
        if(string[i]=='\\'){
            i+=2;
            count++;
        }
        else{
            i++;
        }
    }

    return count;
}

void registers(ast_tree** symb, char* name_func, int reg){
    symbol* local = search_table(name_func)->first_symbol;
    char* name = (*symb)->value;

    while(local!=NULL){
        if(!strcmp(local->id, name)){
            local->register_tab=reg;
            return;
        }

        local=local->next_symbol;
    }  
}

int registers_what(ast_tree** symb, char* name_func){
    symbol* local = search_table(name_func)->first_symbol;
    char* name = (*symb)->value;

    while(local!=NULL){
        if(!strcmp(local->id, name)){
            return local->register_tab;
        }

        local=local->next_symbol;
    }  
    return -1;
}

//HA AQUI UM SEG FAULT DK WHERE
void initial_strings(ast_tree* node){
    if(node){
        if (node->first_child != NULL){
            initial_strings(node->first_child);
        }
        if (node->next_sibling != NULL){
            initial_strings(node->next_sibling);
        }

        if(!strcmp(node->token->id, "StrLit")){
            char* aux=node->value;
            int i=0;
            const int size = strlen(aux);
            int tosub;
            if(nrEsSeq(aux)>=1){
                tosub=(int)(nrEsSeq(aux)/2);
            }
            else{
                tosub=-1;
            }
            printf("@.str.%d = private unnamed_addr constant [%d x i8] c\"", string_count++, size-2-tosub);
            while(aux[i]!='\0'){
                if(aux[i]=='\\'){
                    printf("\\");
                    if(aux[i+1]=='n'){
                        printf("0A");
                    }
                    else if(aux[i+1]=='\\'){
                        printf("5C");
                    }
                    else if(aux[i+1]=='f'){
                        printf("0C");
                    }
                    else if(aux[i+1]=='t'){
                        printf("09");
                    }
                    else if(aux[i+1]=='"'){
                        printf("22");
                    }
                    else if(aux[i+1]=='r'){
                        printf("0D");
                    }

                    i+=2;
                }
                else{
                    if(aux[i]!='"'){
                        printf("%c", aux[i]);
                    }
                    i++;
                }
            }
            puts("\\00\"");
        }
    }
}


void allocaLLVM(ast_tree **node, int reg, int reg2){
    if(reg!=-1){
        printf("\t%%%d = alloca %s\n", reg, typeLLVM((*node)->first_child->token->id));  
    }
    else{
        printf("\t%%%d = alloca %%%d\n", reg, reg2);  
    }
}

void storeLLVM(ast_tree *what, ast_tree* where, int reg, int reg2){
    if(where!=NULL){
        if(strcmp(where->token->id, "Id")==0){
            if(reg==-1){
                printf("\tstore %s %%%d, %s* @%s\n", typeLLVM(where->token->type), reg2 ,typeLLVM(what->token->type), what->value);
            }
            else{
                printf("\tstore %s %%%d, %s* %%%d\n", typeLLVM(where->token->type), reg2 ,typeLLVM(what->token->type), reg);
            }
        }
        else if(operatorLLVM(where)){
            if(reg2==-1){
                printf("\tstore %s %%%d, %s* @%s\n", typeLLVM(where->token->type), reg ,typeLLVM(what->token->type), what->value);
            }
            else{
                printf("\tstore %s %%%d, %s* %%%d\n", typeLLVM(where->token->type), reg ,typeLLVM(what->token->type), reg);
            }
        }
        else{
            const int size = strlen(where->value);
            char* value = (char*)malloc((size+1)*sizeof(char));
            if(!strcmp(where->token->id, "Minus")){
                sprintf(value, "-%s", where->first_child->value);
            }
            else{
                if(!strcmp(where->token->id, "Plus")){
                    sprintf(value, "%s", where->first_child->value);
                }
                else{
                    sprintf(value, "%s", where->value);
                }
            }
            if(reg==-1){
                printf("\tstore %s %s, %s* @%s\n", typeLLVM(where->token->type), value,typeLLVM(what->token->type), what->value);
            }
            else{
                printf("\tstore %s %s, %s* %%%d\n", typeLLVM(where->token->type), value,typeLLVM(what->token->type), reg);
            }
        }            
    }
    else{
        if(reg2!=-1){
            if(!strcmp(what->token->id, "ParamDecl")){
                printf("\tstore %s %%%d, %s* %%%d\n", typeLLVM(what->first_child->token->id), reg2, typeLLVM(what->first_child->token->id), reg);
            }
            else{
                printf("\tstore %s %%%s, %s* %%%d\n", typeLLVM(what->first_child->token->id), what->first_child->next_sibling->value, typeLLVM(what->first_child->token->id), reg);
            }
        }
        else{
            if(!strcmp(typeLLVM(what->first_child->token->id), "i32")){
                printf("\tstore %s 0, %s* %%%d\n", typeLLVM(what->first_child->token->id), typeLLVM(what->first_child->token->id), reg);
            }
            else if(!strcmp(typeLLVM(what->first_child->token->id), "double")){
                printf("\tstore %s 0.0, %s* %%%d\n", typeLLVM(what->first_child->token->id), typeLLVM(what->first_child->token->id), reg);
            }
        }
    }
}

void loadLLVM(ast_tree **node, char* name_func, int reg){
    char* type;
    ast_tree* nul;

    if((*node)->first_child!=NULL){
        nul = (*node)->first_child;
        type=typeLLVM(nul->token->type);
    }
    else{
        nul = (*node);
        type=typeLLVM(nul->token->type);
    }

    typeLLVM(nul->token->type);

    if(!strcmp(nul->token->id, "Id")){
        int regist= registers_what( &nul, name_func);
        if(regist==-1){
            printf("\t%%%d = load %s, %s* @%s\n ", reg, type, type, nul->value);
        }
        else{
            printf("\t%%%d = load %s, %s* %%%d\n ", reg, type, type, regist);
        }
    }
    else{
        if(!strcmp(type, "i32")){
            printf("\t%%%d = add %s 0, %s\n", reg, type, nul->value);
        }
        else if(!strcmp(type, "double")){        
            printf("\t%%%d = fadd %s 0.0, %s\n", reg, type, nul->value);
        }
    }
}

void varDeclLLVM(ast_tree *node, char* tipo, int reg){
    if (strcmp(tipo, "global") == 0){
        char* type=typeLLVM(node->first_child->token->id);

        if(type!=NULL){
            if(!strcmp(type, "i8*")){
                printf("@%s = global %s null\n\n", node->first_child->next_sibling->value, type);
            }
            else if(!strcmp(type, "double")){
                printf("@%s = global %s 0.0\n\n", node->first_child->next_sibling->value, type);
            }
            else{
                printf("@%s = global %s 0\n\n", node->first_child->next_sibling->value, type);
            }
        }
    } 
    else {
        allocaLLVM(&node, reg, -1);
        storeLLVM(node, NULL, reg, -1);
    }
}

int assignLLVM(ast_tree* node, char* name_func, int reg_atual){
    ast_tree* first_op = node->first_child;
    ast_tree* second_op = node->first_child->next_sibling; 
    int reg = registers_what(&first_op,  name_func);

    if(!strcmp(second_op->token->id, "Id")){
        load=1;
        loadLLVM(&second_op, name_func, reg_atual);    
        storeLLVM(first_op, second_op, reg, reg_atual);
    }
    else if(!strcmp(second_op->token->id, "IntLit") || !strcmp(second_op->token->id, "RealLit") ){
        storeLLVM(first_op, second_op, reg, reg_atual);
    }
    else if(!strcmp(second_op->token->id, "Minus")){

    }
    else if(!strcmp(second_op->token->id, "Plus")){
        load=1;
        loadLLVM(&second_op->first_child, name_func, reg_atual);  
        storeLLVM(first_op, second_op->first_child, reg, reg_atual);
    }
    else{
        reg_atual+=generateDecl(second_op, name_func, reg_atual)-1;
        storeLLVM(first_op, second_op, reg_atual, reg);    
        return reg_atual;
    }

    return 1;
}

int operationLLVM(ast_tree *node, char* name_func, int reg_atual){
    ast_tree* first_op;
    ast_tree* second_op;
    
    char* op;
    if(node->first_child){
        first_op = node->first_child;
        second_op = node->first_child->next_sibling;
    } 
    int reg_return=reg_atual;

    if((op=operatorLLVM(node))!=NULL){
        int reg_1=reg_return+=operationLLVM(first_op, name_func, reg_return);
        int reg_2=reg_return+=operationLLVM(second_op, name_func, reg_return);

        printf("\t%%%d = %s %s %%%d, %%%d\n", reg_return, op, typeLLVM(first_op->token->type), reg_1-1, reg_2-1);

        return reg_return-reg_atual+1;
    }
    else{
        loadLLVM(&node, name_func, reg_return);
        return reg_return-reg_atual+1;
    }
}

void ifLLVM(ast_tree* node){

}

void forLLVM(ast_tree *node){

}


//FALTA BOOLS 
int printLLVM(ast_tree *node, char* name_func, int reg){
    char* type = typeLLVM(node->first_child->token->type);;
    

    if(operatorLLVM(node->first_child)){
        reg+=generateDecl(node->first_child, name_func, reg)-1;

        if(!strcmp(type, "i32")){
                
            printf("\t%%%d = call i32(i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.int, i32 0, i32 0), i32 %%%d)\n", reg+1, reg);
        }
        else if(!strcmp(type, "double")){
            printf("\t%%%d = call i32 (i8*, ...) @printf(i8* getelementptr  ([7 x i8], [7 x i8]* @.float, i32 0, i32 0), double %%%d)\n", reg+1, reg);
        }

        return reg;
    }
    else{
        if(!strcmp(node->first_child->token->id, "Id")){
            loadLLVM(&node->first_child, name_func, reg);

            if(!strcmp(type, "i32")){
                
                printf("\t%%%d = call i32(i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.int, i32 0, i32 0), i32 %%%d)\n", reg+1, reg);
            }
            else if(!strcmp(type, "double")){
                printf("\t%%%d = call i32 (i8*, ...) @printf(i8* getelementptr  ([7 x i8], [7 x i8]* @.float, i32 0, i32 0), double %%%d)\n", reg+1, reg);
            }
            load=1;
        }
        else{
            if(!strcmp(type, "i32")){
                const int size = strlen(node->first_child->value);
                char* value = (char*)malloc((size+1)*sizeof(char));
                if(!strcmp(node->first_child->token->id, "Minus")){
                    sprintf(value, "-%s", node->first_child->first_child->value);
                }
                else{
                    if(!strcmp(node->first_child->token->id, "Plus")){                
                        sprintf(value, "%s", node->first_child->first_child->value);
                    }
                    else{  
                        sprintf(value, "%s", node->first_child->value);
                    }
                }
                printf("\t%%%d = call i32(i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.int, i32 0, i32 0), i32 %s)\n", reg, value);   
            }
            else if(!strcmp(type, "double")){
                const int size = strlen(node->first_child->value);
                char* value = (char*)malloc((size+1)*sizeof(char));
                if(!strcmp(node->first_child->token->id, "Minus")){
                    sprintf(value, "-%s", node->first_child->first_child->value);
                }
                else{
                    if(!strcmp(node->first_child->token->id, "Plus")){                
                        sprintf(value, "%s", node->first_child->first_child->value);
                    }
                    else{  
                        sprintf(value, "%s", node->first_child->value);
                    }
                }
                printf("\t%%%d = call i32 (i8*, ...) @printf(i8* getelementptr  ([7 x i8], [7 x i8]* @.float, i32 0, i32 0), double %s)\n", reg, value);
            }
            else if(!strcmp(type, "i8*")){
                int tosub;
                if(nrEsSeq(node->first_child->value)>=1){
                    tosub=(int)(nrEsSeq(node->first_child->value)/2);
                }
                else{
                    tosub=-1;
                }
                const int size = strlen(node->first_child->value)-2-tosub;
                printf("\t%%%d = call i32 (i8*, ...) @printf(i8* getelementptr  ([%d x i8], [%d x i8]* @.str.%d, i32 0, i32 0))\n", reg, size, size, string_count++);
            }
        }
    }
    return 1;
}

int callLLVM(ast_tree *node, char* name_func, int reg, int main){
	ast_tree* aux = node->first_child;
	ast_tree* aux2 = aux->next_sibling;
    
	int is_first = 1;
	int counting = 0;
	int regs[1024];
	
	while(aux2 != NULL){
		if (!strcmp(aux2->token->id, "Call")){
			regs_changed += callLLVM(aux2, name_func, reg, 0);
			reg += regs_changed;
			if (main == 1){
				regs_changed = 0;
			}	
			regs[counting] = reg;
		} else if (operatorLLVM(aux) != NULL){
			reg+=operationLLVM(aux, name_func, reg);
		} else {
			loadLLVM(&aux2, name_func, reg);
			regs[counting] = reg;
		}	
		reg++;
		counting++;
		aux2 = aux2->next_sibling;
	}
	
	
	printf("\t%%%d = call %s @%s(", reg, typeLLVM(node->token->type), aux->value);
	for (int i = 0; i < counting; i++){
		aux = aux->next_sibling;
		if (is_first == 1){
			printf("%s %%%d", typeLLVM(aux->token->type), regs[i]);
			is_first = 0;
		} else {
			printf(", %s %%%d", typeLLVM(aux->token->type), regs[i]);
		}
	}
	reg++;
	printf(")\n");
	if (main == 0){
		return counting;
	} else {
		return reg;
	}
	
}

void returnLLVM(ast_tree *node, char* name_func, int reg){
    //Void
    if (node->first_child == NULL){
    	printf("\tret void\n");
    }
    else {
        //Operador
        if (operatorLLVM(node->first_child) != NULL){
            operationLLVM(node->first_child, name_func, 0);
            
        } 
        else{
            token* aux= node->first_child->token;
            char *type = aux->type;
            //Call
            if (!strcmp(aux->id, "Call")){
                
            } 
            //Id RealLit IntLit StrLit
            else if (!strcmp(aux->id, "Id") || !strcmp(aux->id, "IntLit")  || !strcmp(aux->id, "String") || !strcmp(aux->id, "Bool") ||  !strcmp(aux->id, "RealLit")){
                loadLLVM(&node, name_func, reg);
                //printf("\t%%%d = load %s, %s* %%%d\n", reg, typeLLVM(type), typeLLVM(node->first_child->token->type), registers_what(&node->first_child, name_func));
                printf("\tret %s %%%d\n",  typeLLVM(type), reg);
            }   
        }
    }
}



int  generateDecl(ast_tree* node, char* name_func, int reg){
    if (strcmp(node->token->id, "Call") == 0){
        return callLLVM(node, name_func, reg, 1);
    } 
    else if(operatorLLVM(node)){
        return operationLLVM(node, name_func, reg);
    }
    else if (strcmp(node->token->id, "Print") == 0){
        return printLLVM(node, name_func, reg);
    } 
    else if (strcmp(node->token->id, "If") == 0){
        ifLLVM(node);
    } 
    else if (strcmp(node->token->id, "Return") == 0){
        returnLLVM(node, name_func, reg);
    } 
    else if (strcmp(node->token->id, "VarDecl") == 0){
        registers(&node->first_child->next_sibling,  name_func, reg);
        varDeclLLVM(node, "local", reg);
    } 
    else if (strcmp(node->token->id, "For") == 0){
        forLLVM(node);
    } 
    else if (strcmp(node->token->id, "Assign") == 0){
        return assignLLVM(node, name_func, reg);
    }
    return 1;
}

void generateLLVM(ast_tree *node){
    if(int_llvm || float32_llvm || string_llvm || bool_llvm){
        printf("declare i32 @printf(i8*, ...)\n\n");
    }

    if(int_llvm){
        printf("@.int = private unnamed_addr constant [4 x i8] c\"%%d\\0A\\00\"\n\n");
    }
    if(float32_llvm){
        printf("@.float = private unnamed_addr constant [7 x i8] c\"%%.08f\\0A\\00\"\n\n");
    }
    if(bool_llvm){
        printf("@.true = private unnamed_addr constant [6 x i8] c\"true\\0A\\00\"\n\n");
        printf("@.false = private unnamed_addr constant [7 x i8] c\"false\\0A\\00\"\n\n");
    }
    //ainda dá erro de execução
    if(string_llvm){
        initial_strings(node);    
        string_count=1;
    }

    ast_tree *decl = node->first_child;
    ast_tree *decl2 = node->first_child;
    
    char* type;
    printf("define %s @main(i32 %%argc, i8** %%args", "i32");
    printf(") {\n");
    //printf("call i32(i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.int, i32 0, i32 0), i32 %%argc)\n");
    while(decl2 !=NULL){
        if (strcmp(decl2->token->id, "FuncDecl") == 0){
            if(!strcmp(decl2->first_child->first_child->value ,"main")) {   
                type = typeLLVM(decl2->first_child->first_child->next_sibling->token->id);
                printf("\tcall %s @main.entry(i32 %%argc, i8** %%args)\n", type);
                break;
            }
        }
        decl2=decl2->next_sibling;
    }
    printf("\tret i32 0\n");
    puts("}\n");
    
    
    while (decl != NULL){
        if (strcmp(decl->token->id, "VarDecl") == 0){
            varDeclLLVM(decl, "global", 0);
        } 
        
        else if (strcmp(decl->token->id, "FuncDecl") == 0){
            type = typeLLVM(decl->first_child->first_child->next_sibling->token->id);
            funcDeclLLVM(decl, type);
        }
        decl = decl->next_sibling;
    }
    
    
}



void funcDeclLLVM(ast_tree *node, char* type){
    ast_tree *params;

    int counter=1;
    
    if (nrChilds_node(node->first_child)==3){
        params = node->first_child->first_child->next_sibling->next_sibling;
    } else {
        params = node->first_child->first_child->next_sibling;
    }

    if(!strcmp(node->first_child->first_child->value, "main")){
        counter=3;
        printf("define %s @main.entry (i32, i8**", type);
        getFuncParams(params, 3);
        printf(") {\n");
        printf("\t%%3 = alloca i32\n");
        printf("\t%%4 = alloca i8**\n");
        printf("\tstore i32 %%0, i32* %%3\n");
        printf("\tstore i8** %%1, i8*** %%4\n");
        counter = 5;
    }
    else{
        printf("define %s @%s(", type, node->first_child->first_child->value);    
        int size = counter = getFuncParams(params, 0) + 1;    
        printf(") {\n");
        ast_tree *aux = params->first_child;
        while (aux != NULL){
            allocaLLVM(&aux, counter, counter-size);
            registers(&aux->first_child->next_sibling,  node->first_child->first_child->value, counter);
            storeLLVM(aux, NULL, counter, counter-size);
            counter++;
            aux = aux->next_sibling;
        }
    }
  

    printf("\n");
    
    //func_body
    ast_tree *aux2 = node->first_child->next_sibling->first_child;
    int return_pls=0;
    int count_reg=0;
        //childs_decl
        while (aux2 !=NULL){
            count_reg= generateDecl(aux2, node->first_child->first_child->value, counter);
            if(!strcmp(aux2->token->id,"Return")){
                return_pls=1;
            }

            if(strcmp(aux2->token->id,"Assign")!=0){
                counter++;

                if(!strcmp(aux2->token->id,"Print")){
                    if(load){
                        counter++;
                        load=0;
                    }

                    if(count_reg!=1){
                        counter=count_reg+2;
                    }
                }
                if(!strcmp(aux2->token->id,"Call")){
                    counter=count_reg;
                }
            }
            else{
                if(count_reg!=1){
                    counter=count_reg+1;
                }


                if(load){
                    counter++;
                    load=0;
                }
            }
            aux2=aux2->next_sibling;
        } 
        if(!return_pls){
            if(!strcmp(type, "void")){
            printf("\tret void\n}\n\n");
            }
            else{
                if(!strcmp(type, "i32")){            
                    printf("\tret %s 0\n}\n\n", type);
                }
                else if(!strcmp(type, "double")){    
                    printf("\tret %s 0.0\n}\n\n", type);
                } 
                else if(!strcmp(type, "i8*")){    
                    printf("\tret %s null\n}\n\n", type);
                } 
            }
        }
        else{
            puts("}\n");
        }
}

char* typeLLVM(char* type){
    if (!strcmp(type, "Int") || !strcmp(type, "IntLit") || !strcmp(type, "int")){
        return "i32";
    } else if (!strcmp(type, "Float32") || !strcmp(type, "RealLit") || !strcmp(type, "float32")){
        return "double";
    } else if (!strcmp(type, "Bool") || !strcmp(type, "bool")){
        return "i1";
    } else if (!strcmp(type, "String") || !strcmp(type, "StrLit") || !strcmp(type, "string")){
        return "i8*";
    }
    else{
        return "void";
    }
}

int getFuncParams(ast_tree *node, int start){
    int count = start;
    ast_tree *params = node->first_child;
    while (params != NULL){
        char *type = typeLLVM(params->first_child->token->id);
        if (count == 0){
            printf("%s", type);
            count++;
        } else {
            printf(", %s", type);
            count++;
        }
        params = params->next_sibling;
    }
    return count ;
}
