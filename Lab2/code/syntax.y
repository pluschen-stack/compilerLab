%{
    #include<stdio.h>
    #include"node.h"
    #include"lex.yy.c"
    extern int syntaxerror;
    
    pNode root;
    #define YYERROR_VERBOSE 1

%}

%locations

/*
    在bison语法分析器中，每个语法符号，包括记号和非终结符，都可以有一个相应的值。
    默认情况下所有的符号值都是整数，但是真正有用的程序通常需要更多有价值的符号值。
    而%union，正如它的名字所暗示的那样，可以用来为符号值创建一个C语言的union类型。
*/
//声明联合类型，以上说明摘自flex && bison
%union{
    pNode node; 
}

/*
    一旦联合类型被定义，我们需要告诉bison每种语法符号所用的值类型，这通过防止在尖括号
    （<>）中的联合类型的相应成员名字来确定。
*/
//词法单元
%token <node> INT
%token <node> FLOAT
%token <node> ID
%token <node> TYPE
%token <node> COMMA
%token <node> DOT
%token <node> SEMI
%token <node> RELOP
%token <node> ASSIGNOP
%token <node> PLUS MINUS STAR DIV
%token <node> AND OR NOT 
%token <node> LP RP LB RB LC RC
%token <node> IF
%token <node> ELSE
%token <node> WHILE
%token <node> STRUCT
%token <node> RETURN

// non-terminals

%type <node> Program ExtDefList ExtDef ExtDecList   //  High-level Definitions
%type <node> Specifier StructSpecifier OptTag Tag   //  Specifiers
%type <node> VarDec FunDec VarList ParamDec         //  Declarators
%type <node> CompSt StmtList Stmt                   //  Statements
%type <node> DefList Def Dec DecList                //  Local Definitions
%type <node> Exp Args                               //  Expressions

//优先级定义，从上到下优先级变强
%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT
%left DOT
%left LB RB
%left LP RP
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%
// High-level Definitions
Program:            ExtDefList                              { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "Program", 1, $1); root = $$; }
    ; 
ExtDefList:         ExtDef ExtDefList                       { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "ExtDefList", 2, $1, $2); }
    |                                                       { $$ = NULL; } 
    ; 
ExtDef:             Specifier ExtDecList SEMI               { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "ExtDef", 3, $1, $2, $3); }
    |               Specifier SEMI                          { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "ExtDef", 2, $1, $2); }
    |               Specifier FunDec CompSt                 { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "ExtDef", 3, $1, $2, $3); }
    |               error SEMI                              { syntaxerror = true; }
    ; 
ExtDecList:         VarDec                                  { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "ExtDecList", 1, $1); }
    |               VarDec COMMA ExtDecList                 { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "ExtDecList", 3, $1, $2, $3); }
    ; 

// Specifiers
Specifier:          TYPE                                    { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "Specifier", 1, $1); }
    |               StructSpecifier                         { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "Specifier", 1, $1); }
    ; 
StructSpecifier:    STRUCT OptTag LC DefList RC             { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "StructSpecifier", 5, $1, $2, $3, $4, $5); }
    |               STRUCT Tag                              { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "StructSpecifier", 2, $1, $2); }
    ; 
OptTag:             ID                                      { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "OptTag", 1, $1); }
    |                                                       { $$ = NULL; }
    ; 
Tag:                ID                                      { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "Tag", 1, $1); }
    ; 

// Declarators
VarDec:             ID                                      { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "VarDec", 1, $1); }
    |               VarDec LB INT RB                        { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "VarDec", 4, $1, $2, $3, $4); }
    |               error RB                                { syntaxerror = true; }
    ; 
FunDec:             ID LP VarList RP                        { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "FunDec", 4, $1, $2, $3, $4); }
    |               ID LP RP                                { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "FunDec", 3, $1, $2, $3); }
    |               error RP                                { syntaxerror = true; }
    ; 
VarList:            ParamDec COMMA VarList                  { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "VarList", 3, $1, $2, $3); }
    |               ParamDec                                { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "VarList", 1, $1); }
    ; 
ParamDec:           Specifier VarDec                        { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "ParamDec", 2, $1, $2); }
    ; 
    
// Statements
CompSt:             LC DefList StmtList RC                  { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "CompSt", 4, $1, $2, $3, $4); }
    |               error RC                                { syntaxerror = true; }
    ; 
StmtList:           Stmt StmtList                           { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "StmtList", 2, $1, $2); }
    |                                                       { $$ = NULL; }
    ; 
Stmt:               Exp SEMI                                { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "Stmt", 2, $1, $2); }
    |               CompSt                                  { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "Stmt", 1, $1); }
    |               RETURN Exp SEMI                         { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "Stmt", 3, $1, $2, $3); }    
    |               IF LP Exp RP Stmt %prec LOWER_THAN_ELSE { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "Stmt", 5, $1, $2, $3, $4, $5); }
    |               IF LP Exp RP Stmt ELSE Stmt             { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "Stmt", 7, $1, $2, $3, $4, $5, $6, $7); }
    |               WHILE LP Exp RP Stmt                    { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "Stmt", 5, $1, $2, $3, $4, $5); }
    |               error SEMI                              { syntaxerror = true; }
    ; 
// Local Definitions
DefList:            Def DefList                             { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "DefList", 2, $1, $2); }
    |                                                       { $$ = NULL; }
    ;     
Def:                Specifier DecList SEMI                  { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "Def", 3, $1, $2, $3); }
    ; 
DecList:            Dec                                     { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "DecList", 1, $1); }
    |               Dec COMMA DecList                       { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "DecList", 3, $1, $2, $3); }
    ; 
Dec:                VarDec                                  { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "Dec", 1, $1); }
    |               VarDec ASSIGNOP Exp                     { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "Dec", 3, $1, $2, $3); }
    ; 
//7.1.7 Expressions
Exp:                Exp ASSIGNOP Exp                        { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "Exp", 3, $1, $2, $3); }
    |               Exp AND Exp                             { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "Exp", 3, $1, $2, $3); }
    |               Exp OR Exp                              { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "Exp", 3, $1, $2, $3); }
    |               Exp RELOP Exp                           { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "Exp", 3, $1, $2, $3); }
    |               Exp PLUS Exp                            { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "Exp", 3, $1, $2, $3); }
    |               Exp MINUS Exp                           { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "Exp", 3, $1, $2, $3); }
    |               Exp STAR Exp                            { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "Exp", 3, $1, $2, $3); }
    |               Exp DIV Exp                             { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "Exp", 3, $1, $2, $3); }
    |               LP Exp RP                               { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "Exp", 3, $1, $2, $3); }
    |               MINUS Exp                               { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "Exp", 2, $1, $2); }
    |               NOT Exp                                 { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "Exp", 2, $1, $2); }
    |               ID LP Args RP                           { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "Exp", 4, $1, $2, $3, $4); }
    |               ID LP RP                                { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "Exp", 3, $1, $2, $3); }
    |               Exp LB Exp RB                           { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "Exp", 4, $1, $2, $3, $4); }
    |               Exp DOT ID                              { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "Exp", 3, $1, $2, $3); }
    |               ID                                      { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "Exp", 1, $1); }
    |               INT                                     { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "Exp", 1, $1); }
    |               FLOAT                                   { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "Exp", 1, $1); }
    ; 
Args :              Exp COMMA Args                          { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "Args", 3, $1, $2, $3); }
    |               Exp                                     { $$ = newSyntaxNode(@$.first_line, NON_TERMINAL, "Args", 1, $1); }
    ; 
%%

int yyerror(char* msg){
    fprintf(stderr, "Error type B at line %d: %s.\n", yylineno, msg);
}