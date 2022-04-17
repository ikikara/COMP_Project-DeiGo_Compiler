/*

João Carlos Borges Silva Nº 2019216753
Pedro Afonso Ferreira Lopes Martins Nº 2019216826

*/

%{
#include "syntax_tree.h"
#include "y.tab.h"


#define print_lex(text)\
        if (flagL==1){\
			if(strcmp(text,"RESERVED")==0)\
				printf("RESERVED(%s)\n", yytext);\
			else if(strcmp(text, "ID")==0)\
				printf("ID(%s)\n", yytext);\
			else if(strcmp(text, "INTLIT")==0)\
				printf("INTLIT(%s)\n", yytext);\
			else if(strcmp(text, "REALLIT")==0)\
				printf("REALLIT(%s)\n", yytext);\
			else if(strcmp(text, "STRLIT")==0)\
				printf("STRLIT(%s)\n", yytext);\
			else\
				printf(text);\
		}

#define send_yacc(text)\
		if (flagT) { \
			if(text==SEMICOLON){\
				yylval.token = new_token((char*)strdup(";"), yylineno, c-yyleng); \
			}\
			else{\
				new_token(yytext, yylineno, c-yyleng);\
				yylval.token = new_token((char*)strdup(yytext), yylineno, c-yyleng);\
			}\
			return text;\
		}

int c=1;
int lcoment=1;
int ccoment=1;
int semicolon=0;
int cstr=1;

int cError;
int lError;

extern int flagL;
extern int flagT;
extern int flagErrorEOF;
extern int flagErrorNL;
extern int flagTerror;

%}
%option yylineno
%X COMMENTS STR ESCSEQ

id 								[_a-zA-Z][a-zA-Z0-9_]*
intlit							(0([xX](_?[0-9a-fA-F]+))*)|(0(_?[0-7]+)*)|(0|([1-9](_?[0-9]+)*))|(0[bB](_?[0-1]+)*)
reallit							(([0-9]+"."[0-9]*)|([0-9]*"."[0-9]+))((("e"|"E")("+"|"-")?)[0-9]+)?|[0-9]*((("e"|"E")("+"|"-")?)[0-9]+)
reserved  						"break"|"case"|"chan"|"const"|"continue"|"default"|"defer"|"fallthrough"|"go"|"goto"|"import"|"interface"|"map"|"range"|"select"|"struct"|"switch"|"type"|"++"|"--"
	
semicolon      							";"
comma									","
blankid									"_"
assign									"="
star									"*"
div										"/"
minus									"-"
plus									"+"
eq										"=="
ge										">="
gt										">"
lbrace									"{"
le										"<="
lpar									"("
lsq										"["
lt										"<"
mod										"%"
ne										"!="
not										"!"
and										"&&"
or										"||"
rbrace									"}"
rpar									")"
rsq										"]"
package									"package"
return									"return"
else									"else"
for										"for"
if										"if"
var										"var"
int										"int"
float32									"float32"
bool									"bool"
string									"string"
print									"fmt.Println"
parseint								"strconv.Atoi"
func									"func"
cmdargs									"os.Args"

invoctal								0(_?[0-7]*[8-9]+[0-7]*)*	
invescape								"\\"[^"f""n""r""t""\\"\"\n\r]
esc_seq									"\\"("f"|"n"|"r"|"t"|"\\"|\")

comments								"//"[^\n]*(\n)?

newline									\n|\r\n|\r

%%
\"\"															{print_lex("STRLIT"); c+=yyleng; semicolon=1; send_yacc(STRLIT);}

\"																{yymore(); BEGIN STR; cError=c; cstr=c; lError=yylineno;}
<STR>([^\"\n\r\\]*{esc_seq}*)*				 					{yymore(); semicolon=1;}
<STR><<EOF>>													{printf("Line %d, column %d: unterminated string literal\n", lError, cstr); semicolon=0; return 0;}
<STR>([^\"\n\r\\]*{esc_seq}*)*{newline}							{printf("Line %d, column %d: unterminated string literal\n", lError, cstr); c=1; BEGIN 0; semicolon=0;}
<STR>([^\"\n\r\\]*{esc_seq}*)*"\\"								{printf("Line %d, column %d: invalid escape sequence (%s)\n", lError, c+yyleng-1, &yytext[yyleng-1]); cError+=yyleng+1; c+=yyleng; BEGIN ESCSEQ;  semicolon=0;}
<STR>([^\"\n\r\\]*{esc_seq}*)*{invescape}						{printf("Line %d, column %d: invalid escape sequence (%s)\n", lError, c+yyleng-2, &yytext[yyleng-2]); cError+=yyleng+1; c+=yyleng; BEGIN ESCSEQ;  semicolon=0;}
<STR>\"															{BEGIN 0; print_lex("STRLIT"); c+=yyleng; send_yacc(STRLIT);}	


<ESCSEQ><<EOF>>													{printf("Line %d, column %d: unterminated string literal\n", lError, cstr);  semicolon=0; return 0;}
<ESCSEQ>{newline}												{printf("Line %d, column %d: unterminated string literal\n", lError, cstr); c=1; ; BEGIN 0;  semicolon=0;}
<ESCSEQ>"\\"													{printf("Line %d, column %d: invalid escape sequence (%s)\n", lError, c+yyleng-1, &yytext[yyleng-1]); cError+=yyleng+1; c+=yyleng;  semicolon=0;}
<ESCSEQ>{invescape}												{printf("Line %d, column %d: invalid escape sequence (%s)\n", lError, c+yyleng-2, &yytext[yyleng-2]); cError+=yyleng+1; c+=yyleng;  semicolon=0;}	
<ESCSEQ>([^\"\n\r\\]*{esc_seq}*)*								{c+=yyleng;}
<ESCSEQ>\"														{BEGIN 0;	c+=yyleng;}


"/*"															{BEGIN COMMENTS; lcoment=yylineno; ccoment=c; c+=yyleng;}
<COMMENTS>"*/"													{BEGIN 0;  c+=yyleng;}
<COMMENTS><<EOF>>												{printf("Line %d, column %d: unterminated comment\n", lcoment, ccoment); BEGIN 0; return 0;}
<COMMENTS>{newline}												{if(semicolon==1){ print_lex("SEMICOLON\n"); send_yacc(SEMICOLON);}  c=1; semicolon=0;}											
<COMMENTS>.														{c+=yyleng;}

{comments}														{if(semicolon==1){ print_lex("SEMICOLON\n"); send_yacc(SEMICOLON);}  c=1; semicolon=0; }



{newline}														{lError=yylineno; flagErrorNL=1; cError=c; if(semicolon==1){print_lex("SEMICOLON\n"); semicolon=0; c=1; send_yacc(SEMICOLON); }
																 else{semicolon=0; } flagErrorNL=0; c=1;}

<<EOF>>															{lError=yylineno; flagErrorEOF=1; cError=c; if(semicolon==1){; semicolon=0; print_lex("SEMICOLON\n"); send_yacc(SEMICOLON);} return 0;}


{semicolon}     										    	{print_lex("SEMICOLON\n"); c+=yyleng; semicolon=0; send_yacc(SEMICOLON);}
{comma}															{print_lex("COMMA\n"); c+=yyleng; semicolon=0; send_yacc(COMMA);}
{blankid}														{print_lex("BLANKID\n"); c+=yyleng; semicolon=0; send_yacc(BLANKID);}
{assign}														{print_lex("ASSIGN\n"); c+=yyleng; semicolon=0; send_yacc(ASSIGN);}
{star}															{print_lex("STAR\n"); c+=yyleng; semicolon=0; send_yacc(STAR);}
{div}															{print_lex("DIV\n"); c+=yyleng; semicolon=0; send_yacc(DIV);}
{minus}															{print_lex("MINUS\n"); c+=yyleng; semicolon=0; send_yacc(MINUS);}
{plus}															{print_lex("PLUS\n"); c+=yyleng; semicolon=0; send_yacc(PLUS);}
{eq}															{print_lex("EQ\n"); c+=yyleng; semicolon=0; send_yacc(EQ);}
{ge}															{print_lex("GE\n"); c+=yyleng; semicolon=0; send_yacc(GE);}
{gt}															{print_lex("GT\n"); c+=yyleng; semicolon=0; send_yacc(GT);}
{lbrace}														{print_lex("LBRACE\n"); c+=yyleng; semicolon=0; send_yacc(LBRACE);}
{le}															{print_lex("LE\n"); c+=yyleng; semicolon=0; send_yacc(LE);}
{lpar}															{print_lex("LPAR\n"); c+=yyleng; semicolon=0; send_yacc(LPAR);}
{lsq}															{print_lex("LSQ\n"); c+=yyleng; semicolon=0; send_yacc(LSQ);}
{lt}															{print_lex("LT\n"); c+=yyleng; semicolon=0; send_yacc(LT);}
{mod}															{print_lex("MOD\n"); c+=yyleng; semicolon=0; send_yacc(MOD);}
{ne}															{print_lex("NE\n"); c+=yyleng; semicolon=0; send_yacc(NE); }
{not}															{print_lex("NOT\n"); c+=yyleng; semicolon=0; send_yacc(NOT);}
{and}															{print_lex("AND\n"); c+=yyleng; semicolon=0; send_yacc(AND);}
{or}															{print_lex("OR\n"); c+=yyleng; semicolon=0; send_yacc(OR);}
{rbrace}														{print_lex("RBRACE\n"); c+=yyleng; semicolon=1; send_yacc(RBRACE);}				 	
{rpar}															{print_lex("RPAR\n"); c+=yyleng; semicolon=1; send_yacc(RPAR);}				 	
{rsq}															{print_lex("RSQ\n"); c+=yyleng; semicolon=1; send_yacc(RSQ);}				 	
{package}														{print_lex("PACKAGE\n"); c+=yyleng; semicolon=0; send_yacc(PACKAGE);}
{return}														{print_lex("RETURN\n"); c+=yyleng; semicolon=1; send_yacc(RETURN);} 	
{else}															{print_lex("ELSE\n"); c+=yyleng; semicolon=0; send_yacc(ELSE);}
{for}															{print_lex("FOR\n"); c+=yyleng; semicolon=0; send_yacc(FOR);}
{if}															{print_lex("IF\n"); c+=yyleng; semicolon=0; send_yacc(IF);}
{var}															{print_lex("VAR\n"); c+=yyleng; semicolon=0; send_yacc(VAR);}
{int}															{print_lex("INT\n"); c+=yyleng; semicolon=0; send_yacc(INT);}
{float32}														{print_lex("FLOAT32\n"); c+=yyleng; semicolon=0; send_yacc(FLOAT32);}
{bool}															{print_lex("BOOL\n"); c+=yyleng; semicolon=0; send_yacc(BOOL);}
{string}														{print_lex("STRING\n"); c+=yyleng; semicolon=0; send_yacc(STRING);}
{print}															{print_lex("PRINT\n"); c+=yyleng; semicolon=0; send_yacc(PRINT);}
{parseint}														{print_lex("PARSEINT\n"); c+=yyleng; semicolon=0; send_yacc(PARSEINT);}
{func}															{print_lex("FUNC\n"); c+=yyleng; semicolon=0; send_yacc(FUNC);}
{cmdargs}														{print_lex("CMDARGS\n"); c+=yyleng; semicolon=0; send_yacc(CMDARGS);}	

{reserved}														{print_lex("RESERVED"); c+=yyleng; semicolon=0; send_yacc(RESERVED);}
{id} 															{print_lex("ID"); c+=yyleng; semicolon=1; send_yacc(ID);}
{intlit}														{print_lex("INTLIT"); c+=yyleng; semicolon=1; send_yacc(INTLIT);}
{reallit}														{print_lex("REALLIT"); c+=yyleng; semicolon=1; send_yacc(REALLIT);}


" "|\t															{c+=yyleng;}


{invoctal}														{printf("Line %d, column %d: invalid octal constant (%s)\n", yylineno, c, yytext); c+=yyleng; semicolon=0;}
.																{printf("Line %d, column %d: illegal character (%c)\n", yylineno, c, yytext[0]); c+=yyleng; semicolon=0;}


%%
int yywrap(){
	return 1;
}

void yyerror (char *s) {
	flagTerror=1;
	if(flagErrorEOF==1){
		printf ("Line %d, column %d: %s: %s\n", lError, cError, s, yytext );	
	}
	else if(flagErrorNL==1 || strcmp(yytext,"\r\n")==0){
		if(strcmp(yytext,"\n")==0 || strcmp(yytext, "\r\n")==0){
			printf ("Line %d, column %d: %s: %s\n", lError-1, cError, s, yytext);
		}
		else{
			if(c==1){
				printf ("Line %d, column %d: %s: %s\n", yylineno, 1, s, yytext);
			}
			else{
				printf ("Line %d, column %d: %s: %s\n", yylineno, c-yyleng, s, yytext);
			}
		}
	}
	else{
		printf ("Line %d, column %d: %s: %s\n", yylineno, c-yyleng, s, yytext );
	}
}
