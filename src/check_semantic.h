/*

João Carlos Borges Silva Nº 2019216783
Pedro Afonso Ferreira Lopes Martins Nº 2019216826

*/

char* getOperator(char* operator);
char* typeCheck(table tabela, char *nome, ast_tree *node);
int existince_table(char* name_func);
int existince(char* name, char* name_func, int global_f);
void cannotFindSymbol(ast_tree *node, char* toprint);
void symbolNeverUsed();
void operatorNotApplied(ast_tree *node);
void symbolAlreadyDefined(ast_tree *node);
void incompatibleType(ast_tree *node);
