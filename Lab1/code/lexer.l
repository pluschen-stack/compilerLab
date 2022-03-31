%{
#include "syntax.tab.h"
#include "node.h"

extern bool lexerror;
int yycolumn = 1;

#define YY_USER_ACTION \
    yylloc.first_line=yylloc.last_line=yylineno; \
    yylloc.first_column=yycolumn; \
    yylloc.last_column=yycolumn+yyleng-1; \
    yycolumn+=yyleng;
%}

%option yylineno

linecomment "//".*
multilinecomment "/*"[/]*([^*/][/]*|[*][^*/]*)*"*/"
whitespace [ \t] 
digit [0-9]
letter [_a-zA-Z]
letteranddigit [0-9_a-zA-Z]
nonletter [^_a-zA-Z]
RELOP 	>|<|>=|<=|==|!=
IF if
ELSE else
WHILE while
TYPE int|float
STRUCT struct
RETURN return
PLUS "+"
MINUS "-"
STAR "*"
DIV "/"
AND "&&"
OR "||"
NOT "!"
DOT "."
SEMI ";"
COMMA ","
ASSIGNOP "="
LP "("
RP ")"
LB "["
RB "]"
LC "{"
RC "}"
ID {letter}{letteranddigit}*
/*识别八进制数，十进制数，十六进制数*/
INT (0[0-7]*)|([1-9]+[0-9]*)|(0[xX][0-9a-fA-F]+)
/*浮点数指数部分*/
FLOAT {digit}+"."{digit}+|{digit}*"."{digit}+[eE][+-]?{digit}+|{digit}+"."{digit}*[eE][+-]?{digit}+

%%
{whitespace}+ {;}
{linecomment} {;}
{multilinecomment} {;}
\n|\r { yycolumn = 1; }
{IF} { yylval.node = newTokenNode(yylineno, KEYWORD_TYPE, "IF", yytext); return IF; }
{ELSE} { yylval.node = newTokenNode(yylineno, KEYWORD_TYPE, "ELSE", yytext); return ELSE; }
{WHILE} { yylval.node = newTokenNode(yylineno, KEYWORD_TYPE, "WHILE", yytext); return WHILE; }
{TYPE} { yylval.node = newTokenNode(yylineno, TYPE_TYPE, "TYPE", yytext); return TYPE; }
{STRUCT} { yylval.node = newTokenNode(yylineno, KEYWORD_TYPE, "STRUCT", yytext); return STRUCT; }
{RETURN} { yylval.node = newTokenNode(yylineno, KEYWORD_TYPE, "RETURN", yytext); return RETURN; }

{RELOP} { yylval.node = newTokenNode(yylineno, OPERATOR_TYPE, "RELOP", yytext); return RELOP; }
{PLUS} { yylval.node = newTokenNode(yylineno, OPERATOR_TYPE, "PLUS", yytext); return PLUS; }
{MINUS} { yylval.node = newTokenNode(yylineno, OPERATOR_TYPE, "MINUS", yytext); return MINUS; }
{STAR} { yylval.node = newTokenNode(yylineno, OPERATOR_TYPE, "STAR", yytext); return STAR; }
{DIV} { yylval.node = newTokenNode(yylineno, OPERATOR_TYPE, "DIV", yytext); return DIV; }
{AND} { yylval.node = newTokenNode(yylineno, OPERATOR_TYPE, "AND", yytext); return AND; }
{OR} { yylval.node = newTokenNode(yylineno, OPERATOR_TYPE, "OR", yytext); return OR; }
{NOT} { yylval.node = newTokenNode(yylineno, OPERATOR_TYPE, "NOT", yytext); return NOT; }

{DOT} { yylval.node = newTokenNode(yylineno, PUNCTUATION_TYPE, "DOT", yytext); return DOT; }
{SEMI} { yylval.node = newTokenNode(yylineno, PUNCTUATION_TYPE, "SEMI", yytext); return SEMI; }
{COMMA} { yylval.node = newTokenNode(yylineno, PUNCTUATION_TYPE, "COMMA", yytext); return COMMA; }
{ASSIGNOP} { yylval.node = newTokenNode(yylineno, PUNCTUATION_TYPE, "ASSIGNOP", yytext); return ASSIGNOP; }

{LP} { yylval.node = newTokenNode(yylineno, PUNCTUATION_TYPE, "LP", yytext); return LP; }
{RP} { yylval.node = newTokenNode(yylineno, PUNCTUATION_TYPE, "RP", yytext); return RP; }
{LB} { yylval.node = newTokenNode(yylineno, PUNCTUATION_TYPE, "LB", yytext); return LB; }
{RB} { yylval.node = newTokenNode(yylineno, PUNCTUATION_TYPE, "RB", yytext); return RB; }
{LC} { yylval.node = newTokenNode(yylineno, PUNCTUATION_TYPE, "LC", yytext); return LC; }
{RC} { yylval.node = newTokenNode(yylineno, PUNCTUATION_TYPE, "RC", yytext); return RC; }

{ID} { yylval.node = newTokenNode(yylineno, ID_TYPE, "ID", yytext); return ID;}
{INT} { yylval.node = newTokenNode(yylineno,INT_TYPE, "INT", yytext); return INT;}
{FLOAT} { yylval.node = newTokenNode(yylineno, FLOAT_TYPE, "FLOAT", yytext); return FLOAT;}

0[0-9]+ {lexerror = true; fprintf(stderr,"Error type A at Line %d: Illegal octal number \'%s\'.\n", yylineno, yytext); yylval.node = newTokenNode(yylineno,INT_TYPE, "INT", 0); return INT;}
0[xX][0-9a-zA-Z]+ {lexerror = true; fprintf(stderr,"Error type A at Line %d: Illegal hexadecimal number \'%s\'.\n", yylineno, yytext); yylval.node = newTokenNode(yylineno,INT_TYPE, "INT", 0); return INT;}
{digit}+{ID}  { lexerror = true; printf("Error type A at Line %d: Illegal identifier \"%s\".\n", yylineno, yytext); }
"."{digit}+ { lexerror = true; printf("Error type A at Line %d: Illegal floating point number \"%s\".\n", yylineno, yytext); }
{digit}+"." { lexerror = true; printf("Error type A at Line %d: Illegal floating point number \"%s\".\n", yylineno, yytext); }
{digit}*"."{digit}+[eE] { lexerror = true; printf("Error type A at Line %d: Illegal floating point number \"%s\".\n", yylineno, yytext); }
{digit}+"."{digit}*[eE] { lexerror = true; printf("Error type A at Line %d: Illegal floating point number \"%s\".\n", yylineno, yytext); }
{digit}+[eE][+-]?{digit}* { lexerror = true; printf("Error type A at Line %d: Illegal floating point number \"%s\".\n", yylineno, yytext); }
"."[eE][+-]?{digit}+ { lexerror = true; printf("Error type A at Line %d: Illegal floating point number \"%s\".\n", yylineno, yytext); }
. { lexerror = true; fprintf(stderr,"Error type A at Line %d: Mysterious character \'%s\'.\n", yylineno, yytext); }

%%