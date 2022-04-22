#ifndef INTER_H
#define INTER_H

#include "semantics.h"
#include "util.h"

typedef struct Operand_ *pOperand;               //运算对象
typedef struct InterCode_ *pInterCode;           //中间代码的体
typedef struct InterCodes_ *pInterCodes;         //中间代码的头，包含了前后两条中间代码指针
typedef struct InterCodesWrap_ *pInterCodesWrap; //中间代码序列的包装，记录了中间代码的头和尾

extern pSymbolTable symbolTable;

struct Operand_
{
    enum
    {
        //前面加OPERAND是为了避免enum冲突。
        OPERAND_CONSTANT, //常量
        OPERAND_VARIABLE, //变量
        OPERAND_FUNCTION, //函数
        OPERAND_ADDRESS,  //函数的一维数组参数地址，改个名字可能好点
        OPERAND_RELOP,    //逻辑运算符
        OPERAND_LABEL     //标号，跳转语句用
    } kind;
    union
    {
        char *name;
        int value;
    } u;
};

struct InterCode_
{
    enum
    {
        IR_LABEL,
        IR_FUNCTION,
        IR_ASSIGN,
        IR_ADD,
        IR_SUB,
        IR_MUL,
        IR_DIV,
        IR_GET_ADDR,
        IR_READ_ADDR,
        IR_WRITE_ADDR,
        IR_GOTO,
        IR_IF_GOTO,
        IR_RETURN,
        IR_DEC,
        IR_ARG,
        IR_CALL,
        IR_PARAM,
        IR_READ,
        IR_WRITE,
    } kind;

    union
    {
        struct
        {
            pOperand op;
        } oneOp;
        struct
        {
            pOperand right, left;
        } assign;
        struct
        {
            pOperand result, op1, op2;
        } binOp;
        struct
        {
            pOperand x, relop, y, z;
        } ifGoto;
        struct
        {
            pOperand op;
            int size;
        } dec;
    } u;
};

struct InterCodes_
{
    pInterCode code;
    pInterCodes prev, next;
};

struct InterCodesWrap_
{
    pInterCodes head; //第一条中间代码的头
    pInterCodes tail; //末尾的中间代码的尾
    int labelNum;     //符号数,用于给符号命名，符号用于跳转
    int tempVarNum;   //临时变量数，用于给临时变量命令
};

// struct dimInfo_
// {
//     unsigned size;   //这一维拥有的大小
//     struct dimInfo_* next;
// };

// struct highDimarrayInfo_
// {
//     char *arrayName;

//      info;
// };

// struct highDimarrayInfo_
// {
//     struct
//     int size;
// };

pOperand newOperand(int kind, void *val);
void updateOperand(pOperand p, int kind, void *val);
pOperand copyOperand(pOperand p);
void freeOperand(pOperand p);
void printOperand(pOperand operand);

pInterCodes newInterCodes(pInterCode interCode);
void freeInterCodes(pInterCodes p);

pInterCodesWrap newInterCodesWrap();
void addInterCodesToWrap(pInterCodesWrap codes, pInterCodes newcode);
void freeInterCodesWrap(pInterCodesWrap codes);
void printInterCodes(pInterCodesWrap interCodesWrap);

// 这里的函数作用很简单，就是不停的自顶向下走就好了
void generateInterCodes(pNode node);
void translate_ExtDef(pNode node);
void translate_FunDec(pNode node);
void translate_CompSt(pNode node);
void translate_DefList(pNode node);
void translate_Def(pNode node);
void translate_DecList(pNode node);
void translate_Dec(pNode node);
void translate_VarDec(pNode node, pOperand place);
void translate_StmtList(pNode node);

//下面这三个基本表达式翻译，语句翻译，条件表达式翻译直接参考指导书进行翻译
void translate_Exp(pNode exp, pOperand place);
void translate_Stmt(pNode node);
pInterCodes translate_Cond(pNode node, pOperand labelTrue, pOperand labelFalse);

//很简单，直接翻译就好了，如果未来发现遇到的是write这样的就删掉代码就行
void translate_Args(pNode node);

#endif