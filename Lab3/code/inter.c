#include "inter.h"

/**
 * @brief 产生一条中间代码
 *
 * @param kind 什么类型的中间代码
 * @param val 不同的中间代码需要的pOperand都是四字节的，因此使用void*
 */
pOperand newOperand(int kind, void *val)
{
    //突发奇想可以用宏来解决内存申请
    //比如glibc里面就有__malloc_hook
    pOperand p = (pOperand)malloc(sizeof(struct Operand_));
    if (!p)
    {
        fprintf(stderr, "[%s:%d]Out of memory(%ld bytes)\n", __FILE__, __LINE__, sizeof(struct Operand_));
        exit(EXIT_FAILURE);
    }
    assert(kind > 0 && kind < 6);
    switch (kind)
    {
    case OPERAND_CONSTANT:
        p->u.value = (int)val;
        break;
    case OPERAND_VARIABLE:
    case OPERAND_FUNCTION:
    case OPERAND_ADDRESS:
    case OPERAND_RELOP:
    case OPERAND_LABEL:
        p->u.name = (char *)val;
        break;
    }
    return p;
}

void updateOperand(pOperand p, int kind, void *val)
{
    assert(p != NULL);
    assert(kind >= 0 && kind < 6);
    p->kind = kind;
    switch (kind)
    {
    case OPERAND_CONSTANT:
        p->u.value = (int)val;
        break;
    case OPERAND_VARIABLE:
    case OPERAND_FUNCTION:
    case OPERAND_ADDRESS:
    case OPERAND_RELOP:
    case OPERAND_LABEL:
        FREE(p->u.name);
        p->u.name = (char *)val;
        break;
    }
}

void deleteOperand(pOperand p)
{
    if (p == NULL)
        return;
    assert(p->kind >= 0 && p->kind < 6);
    switch (p->kind)
    {
    case OPERAND_CONSTANT:
        break;
    case OPERAND_VARIABLE:
    case OPERAND_FUNCTION:
    case OPERAND_ADDRESS:
    case OPERAND_RELOP:
    case OPERAND_LABEL:
        FREE(p->u.name);
        break;
    }
    free(p);
}

/**
 * @brief 产生一条中间代码
 *
 * @param kind 类型
 * @param ... 不同的中间代码需要的参数不同
 * @return pInterCode
 */
pInterCode newInterCode(int kind, int argc, ...)
{
    pInterCode p = malloc(sizeof(struct InterCode_));
    if (!p)
    {
        fprintf(stderr, "[%s:%d]Out of memory(%ld bytes)\n", __FILE__, __LINE__, sizeof(struct Operand_));
        exit(EXIT_FAILURE);
    }
    va_list vaList;
    va_start(vaList, argc);
    switch (kind)
    {
    case IR_ASSIGN:
        /* code */
        break;

    default:
        break;
    }
    return p;
}

/**
 * @brief 产生中间代码，入口函数
 *
 * @param node 语法分析树的节点
 */
void generateInterCodes(pNode node)
{
    if (node)
    {
        if (!strcmp(node->name, "ExtDef"))
        {
            translate_ExtDef(node);
            generateInterCodes(node->brother);
            return;
        }
        generateInterCodes(node->child);
        generateInterCodes(node->brother);
    }
}

void translate_ExtDef(pNode node)
{
    assert(node != NULL);
    pNode secondChild = node->child->brother;
    //无函数声明，全局变量定义，结构体
    if (!strcmp(secondChild->name, "strcmp"))
    {
        translate_FunDec(secondChild);
        translate_CompSt(secondChild->brother);
    }
}

void translate_FunDec(pNode node)
{
    assert(node != NULL);
    /*
    FunDec:             ID LP VarList RP
        |               ID LP RP
        |               error RP
    */
    pNode child = node->child;
    
}

void translate_CompSt(pNode node)
{
    assert(node != NULL);
}