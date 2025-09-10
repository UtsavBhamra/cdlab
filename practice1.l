%{
#include "y.tab.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

struct SymTabEntry{
    char lexeme[100];
    char token[100];
    int first_line_no;
    char data_type[100];
    char qual[100];
    int freq;
};

struct SymTabEntry* SymTab[1000];
int sym_ptr=0;

int enable = 0;
char curr_data_type[100] = "";
char curr_qual[100] = "";

int find_entry(char* lex){
    for(int i=0;i<sym_ptr;i++){
        if(strcmp(SymTab[i]->lexeme,lex)==0){
            return i+1;
        }
    }
    return -1;
}

int add_entry(char* lex,char* token1,int line_num,char* data_type1,char* qual1){
    int val = find_entry(lex);
    if(val==-1){
        struct SymTabEntry* temp = malloc(sizeof(struct SymTabEntry));
        strcpy(temp->lexeme,lex);
        strcpy(temp->token,token1);
        temp->first_line_no = line_num;
        strcpy(temp->data_type,data_type1);
        strcpy(temp->qual,qual1);
        temp->freq =1;
        SymTab[sym_ptr++] = temp;
    }
    else{
        int idx = val-1;
        SymTab[idx]->freq++;
    }
}

void reset_curr(){
    enable = 0;
    strcpy(curr_data_type,"");
    strcpy(curr_qual,"");
}
%}

%option yylineno

%%
"const"|"static"        { enable=1; strcpy(curr_qual,yytext); return QUAL; }
"int"|"float"|"char"    { enable=1; strcpy(curr_data_type,yytext); return TYPE; }
"for"                   { add_entry(yytext,"keyword",yylineno,"",""); return FOR; }
"while"                 { add_entry(yytext,"keyword",yylineno,"",""); return WHILE; }
"if"                    { add_entry(yytext,"keyword",yylineno,"",""); return IF; }
"else"                  { add_entry(yytext,"keyword",yylineno,"",""); return ELSE; }
"return"                { return RETURN; }
\"([^\\\"]|\\.)*\"      { add_entry(yytext,"string",yylineno,"",""); yylval.id = strdup(yytext); return STRING; }

"=="|"!="|"<="|">="|"<"|">"   { add_entry(yytext,"operator",yylineno,"",""); return RELOP; }
"+"|"-"|"*"|"/"                { add_entry(yytext,"operator",yylineno,"",""); return *yytext; }
"="                            { add_entry(yytext,"operator",yylineno,"",""); return '='; }

[0-9]+                         { add_entry(yytext,"constant",yylineno,"",""); yylval.num = atoi(yytext); return NUMBER; }
[a-zA-Z_][a-zA-Z0-9_]*          { add_entry(yytext,"identifier",yylineno,curr_data_type,curr_qual);
                                  yylval.id = strdup(yytext);
                                  reset_curr();
                                  return ID; }

";"                             { return ';'; }
","                             { return ','; }
"("                             { return '('; }
")"                             { return ')'; }
"{"                             { return '{'; }
"}"                             { return '}'; }
"["                             { return '['; }
"]"                             { return ']'; }

[ \t\r\n]+                      { /* skip whitespace */ }
.                               { printf("Invalid char: %c\n",yytext[0]); }
%%

int yywrap(void){ return 1; }
