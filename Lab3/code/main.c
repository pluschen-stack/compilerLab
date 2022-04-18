#include "node.h"
#include "syntax.tab.h"
#include "semantics.h"

extern pNode root;
extern pSymbolTable symbolTable;
extern int yylineno;
extern int yyparse();
extern void yyrestart(FILE *);
extern pFuncDecStack funcDeckStack;

bool lexerror = false;
bool syntaxerror = false;

/**
 * @brief 启动程序
 *
 * @param argc
 * @param argv c--文件名
 * @return int
 */
int main(int argc, char **argv)
{
    if (argc <= 1)
    {
        yyparse();
        return 0;
    }
    setbuf(stdout, NULL);
    FILE *f = fopen(argv[1], "r");
    if (!f)
    {
        perror(argv[1]);
        return 1;
    }
    yyrestart(f);
    yyparse();
    /*如果既没有词法分析错误也没有语法分析错误就打印先根遍历打印语法树*/
    if (!lexerror && !syntaxerror)
    {
        // printSyntaxTree(root, 0);
        symbolTable = initSymbolTable();
        funcDeckStack = malloc(sizeof(struct FuncDeclarationStack_));
        assert(funcDeckStack != NULL);
        funcDeckStack->stackDepth = 0;
        funcDeckStack->item = NULL;
        startSemanticAnalysis(root);
        //分析完后现在检查是否有声明了但是没有被定义的函数

        freeSymbolTable(symbolTable);
    }
    freeNode(root);
    return 0;
}

/**
 * @brief 检查是否有函数声明了但是没有定义
 *
 */
void checkFucDeclare()
{
    pTableItem temp = funcDeckStack->item;
    while(temp){
        if(checkTableItemConflict(symbolTable,temp)){
            
        }
    }
    
    

}