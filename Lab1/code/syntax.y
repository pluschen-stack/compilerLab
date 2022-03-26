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
Program:            ExtDefList                              { $$ = newNode(@$.first_line, NON_TERMINAL, "Program", 1, $1); root = $$; }
    ; 
ExtDefList:         ExtDef ExtDefList                       { $$ = newNode(@$.first_line, NON_TERMINAL, "ExtDefList", 2, $1, $2); }
    |                                                       { $$ = NULL; } 
    ; 
ExtDef:             Specifier ExtDecList SEMI               { $$ = newNode(@$.first_line, NON_TERMINAL, "ExtDef", 3, $1, $2, $3); }
    |               Specifier SEMI                          { $$ = newNode(@$.first_line, NON_TERMINAL, "ExtDef", 2, $1, $2); }
    |               Specifier FunDec CompSt                 { $$ = newNode(@$.first_line, NON_TERMINAL, "ExtDef", 3, $1, $2, $3); }
    |               error SEMI                              { syntaxerror = true; }
    ; 
ExtDecList:         VarDec                                  { $$ = newNode(@$.first_line, NON_TERMINAL, "ExtDecList", 1, $1); }
    |               VarDec COMMA ExtDecList                 { $$ = newNode(@$.first_line, NON_TERMINAL, "ExtDecList", 3, $1, $2, $3); }
    ; 

// Specifiers
Specifier:          TYPE                                    { $$ = newNode(@$.first_line, NON_TERMINAL, "Specifier", 1, $1); }
    |               StructSpecifier                         { $$ = newNode(@$.first_line, NON_TERMINAL, "Specifier", 1, $1); }
    ; 
StructSpecifier:    STRUCT OptTag LC DefList RC             { $$ = newNode(@$.first_line, NON_TERMINAL, "StructSpecifier", 5, $1, $2, $3, $4, $5); }
    |               STRUCT Tag                              { $$ = newNode(@$.first_line, NON_TERMINAL, "StructSpecifier", 2, $1, $2); }
    ; 
OptTag:             ID                                      { $$ = newNode(@$.first_line, NON_TERMINAL, "OptTag", 1, $1); }
    |                                                       { $$ = NULL; }
    ; 
Tag:                ID                                      { $$ = newNode(@$.first_line, NON_TERMINAL, "Tag", 1, $1); }
    ; 

// Declarators
VarDec:             ID                                      { $$ = newNode(@$.first_line, NON_TERMINAL, "VarDec", 1, $1); }
    |               VarDec LB INT RB                        { $$ = newNode(@$.first_line, NON_TERMINAL, "VarDec", 4, $1, $2, $3, $4); }
    |               error RB                                { syntaxerror = true; }
    ; 
FunDec:             ID LP VarList RP                        { $$ = newNode(@$.first_line, NON_TERMINAL, "FunDec", 4, $1, $2, $3, $4); }
    |               ID LP RP                                { $$ = newNode(@$.first_line, NON_TERMINAL, "FunDec", 3, $1, $2, $3); }
    |               error RP                                { syntaxerror = true; }
    ; 
VarList:            ParamDec COMMA VarList                  { $$ = newNode(@$.first_line, NON_TERMINAL, "VarList", 3, $1, $2, $3); }
    |               ParamDec                                { $$ = newNode(@$.first_line, NON_TERMINAL, "VarList", 1, $1); }
    ; 
ParamDec:           Specifier VarDec                        { $$ = newNode(@$.first_line, NON_TERMINAL, "ParamDec", 2, $1, $2); }
    ; 
    
// Statements
CompSt:             LC DefList StmtList RC                  { $$ = newNode(@$.first_line, NON_TERMINAL, "CompSt", 4, $1, $2, $3, $4); }
    |               error RC                                { syntaxerror = true; }
    ; 
StmtList:           Stmt StmtList                           { $$ = newNode(@$.first_line, NON_TERMINAL, "StmtList", 2, $1, $2); }
    |                                                       { $$ = NULL; }
    ; 
Stmt:               Exp SEMI                                { $$ = newNode(@$.first_line, NON_TERMINAL, "Stmt", 2, $1, $2); }
    |               CompSt                                  { $$ = newNode(@$.first_line, NON_TERMINAL, "Stmt", 1, $1); }
    |               RETURN Exp SEMI                         { $$ = newNode(@$.first_line, NON_TERMINAL, "Stmt", 3, $1, $2, $3); }    
    |               IF LP Exp RP Stmt %prec LOWER_THAN_ELSE { $$ = newNode(@$.first_line, NON_TERMINAL, "Stmt", 5, $1, $2, $3, $4, $5); }
    |               IF LP Exp RP Stmt ELSE Stmt             { $$ = newNode(@$.first_line, NON_TERMINAL, "Stmt", 7, $1, $2, $3, $4, $5, $6, $7); }
    |               WHILE LP Exp RP Stmt                    { $$ = newNode(@$.first_line, NON_TERMINAL, "Stmt", 5, $1, $2, $3, $4, $5); }
    |               error SEMI                              { syntaxerror = true; }
    ; 
// Local Definitions
DefList:            Def DefList                             { $$ = newNode(@$.first_line, NON_TERMINAL, "DefList", 2, $1, $2); }
    |                                                       { $$ = NULL; }
    ;     
Def:                Specifier DecList SEMI                  { $$ = newNode(@$.first_line, NON_TERMINAL, "Def", 3, $1, $2, $3); }
    ; 
DecList:            Dec                                     { $$ = newNode(@$.first_line, NON_TERMINAL, "DecList", 1, $1); }
    |               Dec COMMA DecList                       { $$ = newNode(@$.first_line, NON_TERMINAL, "DecList", 3, $1, $2, $3); }
    ; 
Dec:                VarDec                                  { $$ = newNode(@$.first_line, NON_TERMINAL, "Dec", 1, $1); }
    |               VarDec ASSIGNOP Exp                     { $$ = newNode(@$.first_line, NON_TERMINAL, "Dec", 3, $1, $2, $3); }
    ; 
//7.1.7 Expressions
Exp:                Exp ASSIGNOP Exp                        { $$ = newNode(@$.first_line, NON_TERMINAL, "Exp", 3, $1, $2, $3); }
    |               Exp AND Exp                             { $$ = newNode(@$.first_line, NON_TERMINAL, "Exp", 3, $1, $2, $3); }
    |               Exp OR Exp                              { $$ = newNode(@$.first_line, NON_TERMINAL, "Exp", 3, $1, $2, $3); }
    |               Exp RELOP Exp                           { $$ = newNode(@$.first_line, NON_TERMINAL, "Exp", 3, $1, $2, $3); }
    |               Exp PLUS Exp                            { $$ = newNode(@$.first_line, NON_TERMINAL, "Exp", 3, $1, $2, $3); }
    |               Exp MINUS Exp                           { $$ = newNode(@$.first_line, NON_TERMINAL, "Exp", 3, $1, $2, $3); }
    |               Exp STAR Exp                            { $$ = newNode(@$.first_line, NON_TERMINAL, "Exp", 3, $1, $2, $3); }
    |               Exp DIV Exp                             { $$ = newNode(@$.first_line, NON_TERMINAL, "Exp", 3, $1, $2, $3); }
    |               LP Exp RP                               { $$ = newNode(@$.first_line, NON_TERMINAL, "Exp", 3, $1, $2, $3); }
    |               MINUS Exp                               { $$ = newNode(@$.first_line, NON_TERMINAL, "Exp", 2, $1, $2); }
    |               NOT Exp                                 { $$ = newNode(@$.first_line, NON_TERMINAL, "Exp", 2, $1, $2); }
    |               ID LP Args RP                           { $$ = newNode(@$.first_line, NON_TERMINAL, "Exp", 4, $1, $2, $3, $4); }
    |               ID LP RP                                { $$ = newNode(@$.first_line, NON_TERMINAL, "Exp", 3, $1, $2, $3); }
    |               Exp LB Exp RB                           { $$ = newNode(@$.first_line, NON_TERMINAL, "Exp", 4, $1, $2, $3, $4); }
    |               Exp DOT ID                              { $$ = newNode(@$.first_line, NON_TERMINAL, "Exp", 3, $1, $2, $3); }
    |               ID                                      { $$ = newNode(@$.first_line, NON_TERMINAL, "Exp", 1, $1); }
    |               INT                                     { $$ = newNode(@$.first_line, NON_TERMINAL, "Exp", 1, $1); }
    |               FLOAT                                   { $$ = newNode(@$.first_line, NON_TERMINAL, "Exp", 1, $1); }
    ; 
Args :              Exp COMMA Args                          { $$ = newNode(@$.first_line, NON_TERMINAL, "Args", 3, $1, $2, $3); }
    |               Exp                                     { $$ = newNode(@$.first_line, NON_TERMINAL, "Args", 1, $1); }
    ; 
%%

yyerror(char* msg){
    fprintf(stderr, "Error type B at line %d: %s.\n", yylineno, msg);
}