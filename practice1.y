%{
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

struct SymTabEntry {
    char lexeme[100];
    char token[100];
    int first_line_no;
    char data_type[100];
    char qual[100];
    int freq;
};

extern int yylex(void);
extern int yylineno;
void yyerror(const char *s);

extern struct SymTabEntry* SymTab[1000];
extern int sym_ptr;
%}

%union {
    int num;
    char* id;
}

%token <id> ID STRING
%token <num> NUMBER
%token IF ELSE FOR WHILE RETURN
%token TYPE QUAL
%token RELOP

%left '+' '-'
%left '*' '/'

%%

program:
      decls stmts
;

decls:
      /* empty */
    | decls decl
;

decl:
      TYPE ID ';'                    { printf("Declared variable %s\n", $2); }
    | TYPE ID '[' NUMBER ']' ';'      { printf("Declared array %s[%d]\n", $2, $4); }
    | TYPE ID '=' STRING ';'          { printf("Declared string %s = %s\n", $2, $4); }
    | QUAL TYPE ID ';'                { printf("Qualified declaration of %s\n", $3); }
;

stmts:
      /* empty */
    | stmts stmt
;

stmt:
      assign_stmt ';'
    | if_stmt
    | while_stmt
    | for_stmt
    | block
    | RETURN expr ';'                 { printf("Return statement\n"); }
;

assign_stmt:
      ID '=' expr                     { printf("Assignment: %s = ...\n", $1); }
    | ID '[' expr ']' '=' expr        { printf("Array assignment: %s[...]=...\n", $1); }
;

if_stmt:
      IF '(' cond ')' stmt
    | IF '(' cond ')' stmt ELSE stmt
;

while_stmt:
      WHILE '(' cond ')' stmt
;

for_stmt:
      FOR '(' assign_stmt ';' cond ';' assign_stmt ')' stmt
;

block:
      '{' stmts '}'
;

cond:
      expr RELOP expr
;

expr:
      NUMBER
    | ID
    | STRING
    | expr '+' expr
    | expr '-' expr
    | expr '*' expr
    | expr '/' expr
    | '(' expr ')'
    | ID '[' expr ']'                 { printf("Array access: %s[...]\n", $1); }
;

%%

void yyerror(const char* s){
    fprintf(stderr,"Parser error at line %d: %s\n", yylineno, s);
}

int main(){
    printf("Enter your C-like program. Ctrl+D to stop.\n");
    yyparse();

    printf("\nSymbol Table:\n");
    printf("------------------------------------------------------------\n");
    printf("| Lexeme       | Token     | Line No | Data Type | Qualifier | Frequency |\n");
    printf("------------------------------------------------------------\n");

    for (int i = 0; i < sym_ptr; i++) {
        printf("| %-12s | %-9s | %-7d | %-9s | %-9s | %-9d |\n",
               SymTab[i]->lexeme,
               SymTab[i]->token,
               SymTab[i]->first_line_no,
               SymTab[i]->data_type,
               SymTab[i]->qual,
               SymTab[i]->freq);
    }

    printf("------------------------------------------------------------\n");
    return 0;
}
