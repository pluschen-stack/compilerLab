#ifndef INTER_H
#define INTER_H

#include "semantics.h"

typedef struct Operand_ *pOperand;       //运算对象
typedef struct InterCode_ *pInterCode;   //一条中间代码
typedef struct InterCodes_ *pInterCodes; //双向链表

struct Operand_
{
    enum
    {
        //前面加OPERAND是为了避免enum冲突。
        OPERAND_CONSTANT, //常量
        OPERAND_VARIABLE, //变量
        OPERAND_FUNCTION, //函数
        OPERAND_ADDRESS,//地址
        OPERAND_RELOP,
        OPERAND_LABEL
    } kind;
    union
    {
        char *name;
        int value;
    } u;
};

struct InterCode_
{
    enum {
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

    union {
        struct {
            pOperand op;
        } oneOp;
        struct {
            pOperand right, left;
        } assign;
        struct {
            pOperand result, op1, op2;
        } binOp;
        struct {
            pOperand x, relop, y, z;
        } ifGoto;
        struct {
            pOperand op;
            int size;
        } dec;
    } u;
};

struct InterCodes_
{
    pInterCode code; //中间代码
    pInterCodes prev, next;
};

pOperand newOperand(int kind, void *val);
void updateOperand(pOperand p, int kind, void *val);
void deleteOperand(pOperand p);

void generateInterCodes(pNode node);
void translate_ExtDef(pNode node);
void translate_FunDec(pNode node);
void translate_CompSt(pNode node);

void printInterCode();

#endif