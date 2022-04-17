/*

João Carlos Borges Silva Nº 2019216753
Pedro Afonso Ferreira Lopes Martins Nº 2019216826

*/

%{
    #include <stdlib.h>
    #include <stdio.h>
    #include <string.h>
    #include "syntax_tree.h"
    #include "symbol_table.h"
    #include "check_semantic.h"
    #include "LLVM.h"
    #include "y.tab.h"

    int yylex (void);
    void yyerror(char* s);
    int yylex_destroy(void);

    int flagL=0;
    int flagT=0;  
    int flagS=0;
    int flagTerror=0;
    int flagErrorEOF=0;
    int flagErrorNL=0;

    ast_tree *program;
    table *all;
    char type_aux[8];

    extern int l;
    extern int c;

    int int_llvm=0;
    int float32_llvm=0;
    int string_llvm=0;
    int bool_llvm=0;

    #define givetype(nodes, type)\
        ast_tree* auxnode = NULL;\
		for (ast_tree *current = nodes; current; current = current->next_sibling){\
            auxnode=ast_node(type->token->id, NULL, "");\
            auxnode->next_sibling=current->first_child;\
            current->first_child=auxnode;\
        }

    #define type_type(node, t)\
        node->token->type = t;\

    

%}


%union {
    token* token;
    ast_tree *node;
}

%token <token> SEMICOLON COMMA BLANKID 
%token <token> ASSIGN STAR DIV MINUS PLUS MOD
%token <token> EQ GE GT LE LT NE NOT AND OR
%token <token> LBRACE LSQ LPAR RPAR RSQ RBRACE
%token <token> PACKAGE RETURN 
%token <token> IF ELSE FOR
%token <token> VAR INT FLOAT32 BOOL STRING
%token <token> ID INTLIT REALLIT STRLIT
%token <token> PRINT PARSEINT FUNC CMDARGS RESERVED


%left COMMA
%right ASSIGN 
%left OR 
%left AND 
%left EQ NE GT GE LT LE
%left MINUS PLUS 
%left STAR DIV MOD
%right NOT

%nonassoc NO_ELSE
%nonassoc ELSE
%nonassoc RPAR LPAR LSQ RSQ

%type <node> Program Declarations VarDeclaration VarSpec_Rep Type
%type <node> FuncDeclaration 
%type <node> Parameters Parameters_Rep FuncBody VarsAndStatements 
%type <node> Statement Statement_Rep
%type <node> ParseArgs FuncInvocation  FuncInvocation_Rep Expr
%type <node> Parameters_Decl


%%

Program:
    PACKAGE ID SEMICOLON Declarations                                       {$$ = program = ast_node("Program", NULL, ""); add_childs(program, 1, $4);}    
    ;

Declarations:   
      FuncDeclaration SEMICOLON Declarations                                {$$ = $1; add_siblings($$, 1, $3);}
    | VarDeclaration SEMICOLON Declarations                                 {$$ = $1; add_siblings($$, 1, $3);}
    |                                                                       {$$ = NULL;}
    ;

VarDeclaration:
    VAR ID VarSpec_Rep Type                                                 {$$ = ast_node("VarDecl", NULL, ""); 
                                                                            add_childs($$, 2, $4, ast_node("Id", $2, $2->id)); 
                                                                            givetype($3, $4); add_siblings($$, 1, $3);}

    | VAR LPAR ID VarSpec_Rep Type SEMICOLON RPAR                           {$$ = ast_node("VarDecl", NULL, ""); 
                                                                            add_childs($$, 2, $5, ast_node("Id", $3, $3->id)); 
                                                                            givetype($4, $5); add_siblings($$, 1, $4);}
    ;

VarSpec_Rep:
     COMMA ID VarSpec_Rep                                                   {$$ = ast_node("VarDecl", NULL, ""); 
                                                                            add_childs($$, 1, ast_node("Id", $2, $2->id)); 
                                                                            add_siblings($$, 1, $3);}

    |                                                                       {$$ = NULL;}
    ;

Type:
    INT                                                                     {$$ = ast_node("Int", NULL, "");}
    | FLOAT32                                                               {$$ = ast_node("Float32", NULL, ""); }
    | BOOL                                                                  {$$ = ast_node("Bool", NULL, "");}
    | STRING                                                                {$$ = ast_node("String", NULL, "");}
    ;

FuncDeclaration:
    FUNC ID LPAR Parameters RPAR Type FuncBody                              {$$ = ast_node("FuncDecl", NULL, ""); 
                                                                            ast_tree* aux=ast_node("FuncHeader", NULL, ""); add_childs(aux, 3, ast_node("Id", $2, $2->id), $6, $4); 
                                                                            add_childs($$, 2, aux, $7); $$->first_child->first_child->token->is_func = 1;}

    | FUNC ID LPAR RPAR Type FuncBody                                       {$$ = ast_node("FuncDecl", NULL, ""); 
                                                                            ast_tree* aux=ast_node("FuncHeader", NULL, ""); add_childs(aux, 3, ast_node("Id", $2, $2->id),  $5, ast_node("FuncParams", NULL, ""));
                                                                            add_childs($$, 2, aux, $6); $$->first_child->first_child->token->is_func = 1;}

    | FUNC ID LPAR Parameters RPAR FuncBody                                 {$$ = ast_node("FuncDecl", NULL, ""); 
                                                                            ast_tree* aux=ast_node("FuncHeader", NULL, ""); add_childs(aux, 2, ast_node("Id", $2, $2->id), $4); 
                                                                            add_childs($$, 2, aux, $6); $$->first_child->first_child->token->is_func = 1;}

    | FUNC ID LPAR RPAR FuncBody                                            {$$ = ast_node("FuncDecl", NULL, "");
                                                                            ast_tree* aux=ast_node("FuncHeader", NULL, ""); add_childs(aux, 2, ast_node("Id", $2, $2->id), ast_node("FuncParams", NULL, "")); 
                                                                            add_childs($$, 2, aux, $5); $$->first_child->first_child->token->is_func = 1;}
    ;
    
Parameters:
    Parameters_Decl Parameters_Rep                                          {$$ = ast_node("FuncParams", NULL, ""); add_childs($$, 2, $1, $2);}
    ;

Parameters_Rep:
    COMMA Parameters_Decl Parameters_Rep                                    {$$ = $2; add_siblings($$, 1, $3);}
    |                                                                       {$$ = NULL;}
    ;

Parameters_Decl:
    ID Type                                                                 {$$ = ast_node("ParamDecl", NULL, ""); add_childs($$, 2, $2, ast_node("Id", $1, $1->id));}

FuncBody:
    LBRACE VarsAndStatements RBRACE                                         {$$ = ast_node("FuncBody", NULL, ""); add_childs($$, 1, $2);}
    ;

VarsAndStatements:
    VarDeclaration  SEMICOLON  VarsAndStatements                            {if($$!=NULL){$$ = $1; add_siblings($$, 1, $3);} else { $$=$3;}}
    | Statement  SEMICOLON  VarsAndStatements                               {if($$!=NULL){$$ = $1; add_siblings($$, 1, $3);} else { $$=$3;}}
    | SEMICOLON  VarsAndStatements                                          {$$ = $2;}
    |                                                                       {$$ = NULL;}
    ;


Statement:
    ID ASSIGN Expr                                                          {$$ = ast_node("Assign", $2, $2->id); 
                                                                            if($2!=NULL) add_childs($$, 2, ast_node("Id", $1, $1->id), $3);}

    | LBRACE Statement_Rep RBRACE                                           {if($2!=NULL){
                                                                                if(nrChilds_blocks($2)>=2){
                                                                                    $$ = ast_node("Block", NULL, ""); add_childs($$, 1, $2);
                                                                                }  
                                                                                else {
                                                                                    $$ = $2;
                                                                                } 
                                                                             }  
                                                                             else{
                                                                                $$ = NULL; 
                                                                             }
                                                                            } 
                                                                            
    | IF Expr LBRACE Statement_Rep RBRACE %prec NO_ELSE                     {$$ = ast_node("If", NULL, "");
                                                                            ast_tree* block=ast_node("Block", NULL, ""); add_childs(block, 1, $4); 
                                                                            ast_tree* block2=ast_node("Block", NULL, ""); 
                                                                            if($2!=NULL) add_childs($$, 3, $2, block, block2);}
    
    | IF Expr LBRACE Statement_Rep RBRACE ELSE LBRACE Statement_Rep RBRACE  {$$ = ast_node("If", NULL, ""); 
                                                                            ast_tree* block=ast_node("Block", NULL, ""); add_childs(block, 1, $4);
                                                                            ast_tree* block2=ast_node("Block", NULL, ""); add_childs(block2, 1, $8);  
                                                                            if($2!=NULL) add_childs($$, 3, $2, block, block2);}

    | FOR Expr LBRACE Statement_Rep RBRACE                                  {$$ = ast_node("For", NULL, ""); 
                                                                            ast_tree* block=ast_node("Block", NULL, ""); add_childs(block, 1, $4); 
                                                                            if($2!=NULL) add_childs($$, 2, $2, block);}

    | FOR LBRACE Statement_Rep RBRACE                                       {$$ = ast_node("For", NULL, ""); 
                                                                            ast_tree* block=ast_node("Block", NULL, ""); add_childs(block, 1, $3); 
                                                                            if($2!=NULL) add_childs($$, 1, block);}

    | RETURN Expr                                                           {$$ = ast_node("Return", NULL, ""); add_childs($$, 1, $2);}
    | RETURN                                                                {$$ = ast_node("Return", $1, $1->id); }
    | FuncInvocation                                                        {$$ = ast_node("Call", NULL, "");  add_childs($$, 1, $1); $$->token->is_func = 1; 
                                                                             if($1!=NULL){
                                                                                $$->token->line=$1->token->line;
                                                                                $$->token->column=$1->token->column; 
                                                                             }
                                                                             else{
                                                                                $$->token->line=0;
                                                                                $$->token->column=0;
                                                                             }
                                                                            }
    | ParseArgs                                                             {$$ = ast_node("ParseArgs", NULL, ""); add_childs($$, 1, $1);}
    | PRINT LPAR Expr RPAR                                                  {$$ = ast_node("Print", NULL, ""); add_childs($$, 1, $3);}
    | PRINT LPAR STRLIT RPAR                                                {$$ = ast_node("Print", NULL, ""); add_childs($$, 1, ast_node("StrLit", $3, $3->id));}
    | error                                                                 {$$ = NULL;}



Statement_Rep:
      Statement SEMICOLON Statement_Rep                                     {if($$ != NULL){ $$ = $1; add_siblings($1, 1, $3);}else{ $$ = $3;}}
    |                                                                       {$$ = NULL;}
    ;

ParseArgs:
    ID COMMA BLANKID ASSIGN PARSEINT LPAR CMDARGS LSQ Expr RSQ RPAR         {$$ = ast_node("Id", $1, $1->id); add_siblings($$, 1, $9);}
    | ID COMMA BLANKID ASSIGN PARSEINT LPAR error RPAR                      {$$ = NULL;}
    ;

FuncInvocation:
    ID LPAR Expr FuncInvocation_Rep RPAR                                    {$$ = ast_node("Id", $1, $1->id); add_siblings($$, 2, $3, $4);}
    | ID LPAR RPAR                                                          {$$ = ast_node("Id", $1, $1->id); }
    | ID LPAR error RPAR                                                    {$$ = NULL;}
    ;

FuncInvocation_Rep:
    COMMA Expr FuncInvocation_Rep                                           {$$ = $2; add_siblings($$, 1, $3);}
    |                                                                       {$$ = NULL;}
    ;

Expr:
     Expr OR Expr                                                           {$$ = ast_node("Or", $2, $2->id); add_childs($$, 2, $1, $3);}
    | Expr AND Expr                                                         {$$ = ast_node("And", $2, $2->id); add_childs($$, 2, $1, $3);}
    | Expr LT Expr                                                          {$$ = ast_node("Lt", $2, $2->id); add_childs($$, 2, $1, $3);}
    | Expr GT Expr                                                          {$$ = ast_node("Gt", $2, $2->id); add_childs($$, 2, $1, $3);}
    | Expr EQ Expr                                                          {$$ = ast_node("Eq", $2, $2->id); add_childs($$, 2, $1, $3);}
    | Expr NE Expr                                                          {$$ = ast_node("Ne", $2, $2->id); add_childs($$, 2, $1, $3);}
    | Expr LE Expr                                                          {$$ = ast_node("Le", $2, $2->id); add_childs($$, 2, $1, $3);}
    | Expr GE Expr                                                          {$$ = ast_node("Ge", $2, $2->id); add_childs($$, 2, $1, $3);}
    | Expr PLUS Expr                                                        {$$ = ast_node("Add", $2, $2->id); add_childs($$, 2, $1, $3);}
    | Expr MINUS Expr                                                       {$$ = ast_node("Sub", $2, $2->id); add_childs($$, 2, $1, $3);}
    | Expr STAR Expr                                                        {$$ = ast_node("Mul", $2, $2->id); add_childs($$, 2, $1, $3);}
    | Expr DIV Expr                                                         {$$ = ast_node("Div", $2, $2->id); add_childs($$, 2, $1, $3);}
    | Expr MOD Expr                                                         {$$ = ast_node("Mod", $2, $2->id); add_childs($$, 2, $1, $3);}
    | NOT Expr %prec NOT                                                    {$$ = ast_node("Not", $1, $1->id); add_childs($$, 1, $2);}
    | MINUS Expr %prec NOT                                                  {$$ = ast_node("Minus", $1, $1->id); add_childs($$, 1, $2);}
    | PLUS Expr %prec NOT                                                   {$$ = ast_node("Plus", $1, $1->id); add_childs($$, 1, $2);}
    | INTLIT                                                                {$$ = ast_node("IntLit", $1, $1->id);}
    | REALLIT                                                               {$$ = ast_node("RealLit", $1, $1->id);}
    | ID                                                                    {$$ = ast_node("Id", $1, $1->id);}
    | FuncInvocation                                                        {$$ = ast_node("Call", NULL, ""); add_childs($$, 1, $1); $$->token->is_func = 1; 
                                                                             if($1!=NULL){
                                                                                $$->token->line=$1->token->line;
                                                                                $$->token->column=$1->token->column; 
                                                                             }
                                                                             else{
                                                                                $$->token->line=0;
                                                                                $$->token->column=0;
                                                                             }
                                                                            }
    | LPAR Expr RPAR                                                        {$$ = $2;}
    | LPAR error RPAR                                                       {$$ = NULL;}
    ;



%%

int main(int argc, char *argv[]) {
	if(argc==2){
		if(strcmp(argv[1],"-l")==0){
			flagL=1;
			flagT=0;
			yylex();
		}
		else if(strcmp(argv[1],"-t")==0){
			flagT=1;
			flagL=0;

			yyparse();

            if( flagTerror==0){
                print_tree(program, 0);
            }
		}
        else if(strcmp(argv[1], "-s")==0){
            flagT=1;
			flagL=0;

            yyparse();

            if( flagTerror==0){
                all=new_table("Global", NULL, NULL, NULL);
                create_tables(program, all, 0);
                anotate_tree(&program);
                symbolNeverUsed();

                print_tables(all);
                print_tree(program, 0);
            }
        }
	}
    else if(argc==1){
        flagT=1;
        flagL=0;

        yyparse();

        if( flagTerror==0){
            all=new_table("Global", NULL, NULL, NULL);
            create_tables(program, all, 0);
            anotate_tree(&program);
            symbolNeverUsed();

            if(flagS==0){
                generateLLVM(program);
                //print_tables(all);
            }
        }
    }

	free_tree(program);
    free_table(all);
    yylex_destroy();

	return 0;
}




