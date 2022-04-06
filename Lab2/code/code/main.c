#include "node.h"
#include "syntax.tab.h"

extern pNode root;

extern int yylineno;
extern int yyparse();
extern void yyrestart(FILE *);

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
    fprintf(stdout, "open the file %s\n", argv[1]);
    yyrestart(f);
    yyparse();
    /*如果既没有词法分析错误也没有语法分析错误就打印先根遍历打印语法树*/
    if (!lexerror && !syntaxerror)
    {
        printSyntaxTree(root, 0);
    }
    freeNode(root);
    return 0;
}