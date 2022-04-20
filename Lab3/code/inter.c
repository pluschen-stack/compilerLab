#include "inter.h"


pInterCodesWrap interCodesWrap;

/**
 * @brief 产生一个运算对象
 *
 * @param kind 什么类型的运算分量
 * @param val 运算分量可能是char *也可能是int，但都只有一个，因此使用void*
 */
pOperand newOperand(int kind, void *val)
{
    //突发奇想可以用宏来解决内存申请
    //比如glibc里面就有__malloc_hook
    pOperand p = malloc(sizeof(struct Operand_));
    if (!p)
    {
        fprintf(stderr, "[%s:%d]Out of memory(%ld bytes)\n", __FILE__, __LINE__, sizeof(struct Operand_));
        exit(EXIT_FAILURE);
    }
    p->kind = kind;
    assert(kind > 0 && kind < 6);
    switch (kind)
    {
    case OPERAND_CONSTANT:
        p->u.value = *((int *)val);
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
        p->u.value = *((int *)val);
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

void freeOperand(pOperand p)
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
    assert(kind >= 0 && kind < 19);
    p->kind = kind;
    switch (kind)
    {
    // TODO 还需要继续产生其他类型的中间结构体
    case IR_FUNCTION:
        p->u.oneOp.op = va_arg(vaList, pOperand);
        break;
    case IR_ARG:

    default:

        break;
    }
    return p;
}

void freeInterCode(pInterCode p)
{
    assert(p != NULL);
    assert(p->kind >= 0 && p->kind < 19);
    switch (p->kind)
    {
    case IR_LABEL:
    case IR_FUNCTION:
    case IR_GOTO:
    case IR_RETURN:
    case IR_ARG:
    case IR_PARAM:
    case IR_READ:
    case IR_WRITE:
        freeOperand(p->u.oneOp.op);
        break;
    case IR_ASSIGN:
    case IR_GET_ADDR:
    case IR_READ_ADDR:
    case IR_WRITE_ADDR:
    case IR_CALL:
        freeOperand(p->u.assign.left);
        freeOperand(p->u.assign.right);
        break;
    case IR_ADD:
    case IR_SUB:
    case IR_MUL:
    case IR_DIV:
        freeOperand(p->u.binOp.result);
        freeOperand(p->u.binOp.op1);
        freeOperand(p->u.binOp.op2);
        break;
    case IR_DEC:
        freeOperand(p->u.dec.op);
        break;
    case IR_IF_GOTO:
        freeOperand(p->u.ifGoto.x);
        freeOperand(p->u.ifGoto.relop);
        freeOperand(p->u.ifGoto.y);
        freeOperand(p->u.ifGoto.z);
    }
    FREE(p);
}

/**
 * @brief 新建中间代码的头
 *
 * @param code 中间代码的体
 * @return pInterCodes
 */
pInterCodes newInterCodes(pInterCode interCode)
{
    pInterCodes p = (pInterCodes)malloc(sizeof(struct InterCodes_));
    if (!p)
    {
        fprintf(stderr, "[%s:%d]Out of memory(%ld bytes)\n", __FILE__, __LINE__, sizeof(struct Operand_));
        exit(EXIT_FAILURE);
    }
    p->code = interCode;
    p->prev = NULL;
    p->next = NULL;
}

void freeInterCodes(pInterCodes p)
{
    assert(p != NULL);
    freeInterCode(p->code);
    free(p);
}

/**
 * @brief 创建中间代码头的包装
 *
 * @return pInterCodesWrap
 */
pInterCodesWrap newInterCodesWrap()
{
    pInterCodesWrap p = malloc(sizeof(struct InterCodesWrap_));
    if (!p)
    {
        fprintf(stderr, "[%s:%d]Out of memory(%ld bytes)\n", __FILE__, __LINE__, sizeof(struct Operand_));
        exit(EXIT_FAILURE);
    }
    p->head = NULL;
    p->tail = NULL;
    p->labelNum = 0;
    return p;
}

/**
 * @brief 将中间代码的头加到包装中
 *
 * @param codes 中间代码结构包装
 * @param newcode 新的中间代码的头
 */
void addInterCodesToWrap(pInterCodesWrap codes, pInterCodes newcode)
{
    if (codes->head == NULL)
    {
        codes->head = newcode;
        codes->tail = newcode;
    }
    else
    {
        codes->tail->next = newcode;
        newcode->prev = codes->tail;
        codes->tail = newcode;
    }
}

/**
 * @brief 清除中间代码结构包装
 * 
 * @param codes 中间代码结构包装
 */
void freeInterCodesWrap(pInterCodesWrap codes)
{
    assert(codes != NULL);
    pInterCodes tobeFreed = codes->head;
    while(tobeFreed!=NULL){
        pInterCodes temp = tobeFreed;
        tobeFreed= tobeFreed->next;
        freeInterCodes(temp);
    }
    codes->head =NULL;
    codes->tail =NULL;
    free(codes);
}

/**
 * @brief 打印运算分量到终端
 * 
 * @param operand 运算分量
 */
void printOperand(pOperand operand){
    assert(operand != NULL);
    switch (operand->kind) {
        case OPERAND_CONSTANT:
            printf("#%d", operand->u.value);
            break;
        case OPERAND_VARIABLE:
        case OPERAND_FUNCTION:
        case OPERAND_ADDRESS:
        case OPERAND_RELOP:
        case OPERAND_LABEL:
            printf("%s", operand->u.name);
            break;
    }
}

/**
 * @brief 打印中间代码到终端，如果需要打印到文件，可以妥善利用linux的>
 * 
 * @param interCodesWrap 中间代码结构包装
 */
void printInterCodes(pInterCodesWrap interCodesWrap) {
    for (pInterCodes cur = interCodesWrap->head; cur != NULL; cur = cur->next) {
        assert(cur->code->kind >= 0 && cur->code->kind < 19);
        switch (cur->code->kind) {
            case IR_LABEL:
                printf("LABEL ");
                printOperand(cur->code->u.oneOp.op);
                printf(" :");
                break;
            case IR_FUNCTION:
                printf("FUNCTION ");
                printOperand(cur->code->u.oneOp.op);
                printf(" :");
                break;
            case IR_ASSIGN:
                printOperand(cur->code->u.assign.left);
                printf(" := ");
                printOperand(cur->code->u.assign.right);
                break;
            case IR_ADD:
                printOperand(cur->code->u.binOp.result);
                printf(" := ");
                printOperand(cur->code->u.binOp.op1);
                printf(" + ");
                printOperand(cur->code->u.binOp.op2);
                break;
            case IR_SUB:
                printOperand(cur->code->u.binOp.result);
                printf(" := ");
                printOperand(cur->code->u.binOp.op1);
                printf(" - ");
                printOperand(cur->code->u.binOp.op2);
                break;
            case IR_MUL:
                printOperand(cur->code->u.binOp.result);
                printf(" := ");
                printOperand(cur->code->u.binOp.op1);
                printf(" * ");
                printOperand(cur->code->u.binOp.op2);
                break;
            case IR_DIV:
                printOperand(cur->code->u.binOp.result);
                printf(" := ");
                printOperand(cur->code->u.binOp.op1);
                printf(" / ");
                printOperand(cur->code->u.binOp.op2);
                break;
            case IR_GET_ADDR:
                printOperand(cur->code->u.assign.left);
                printf(" := &");
                printOperand(cur->code->u.assign.right);
                break;
            case IR_READ_ADDR:
                printOperand(cur->code->u.assign.left);
                printf(" := *");
                printOperand(cur->code->u.assign.right);
                break;
            case IR_WRITE_ADDR:
                printf("*");
                printOperand(cur->code->u.assign.left);
                printf(" := ");
                printOperand(cur->code->u.assign.right);
                break;
            case IR_GOTO:
                printf("GOTO ");
                printOperand(cur->code->u.oneOp.op);
                break;
            case IR_IF_GOTO:
                printf("IF ");
                printOperand(cur->code->u.ifGoto.x);
                printf(" ");
                printOperand(cur->code->u.ifGoto.relop);
                printf(" ");
                printOperand(cur->code->u.ifGoto.y);
                printf(" GOTO ");
                printOperand(cur->code->u.ifGoto.z);
                break;
            case IR_RETURN:
                printf("RETURN ");
                printOperand(cur->code->u.oneOp.op);
                break;
            case IR_DEC:
                printf("DEC ");
                printOperand(cur->code->u.dec.op);
                printf(" ");
                printf("%d", cur->code->u.dec.size);
                break;
            case IR_ARG:
                printf("ARG ");
                printOperand(cur->code->u.oneOp.op);
                break;
            case IR_CALL:
                printOperand(cur->code->u.assign.left);
                printf(" := CALL ");
                printOperand(cur->code->u.assign.right);
                break;
            case IR_PARAM:
                printf("PARAM ");
                printOperand(cur->code->u.oneOp.op);
                break;
            case IR_READ:
                printf("READ ");
                printOperand(cur->code->u.oneOp.op);
                break;
            case IR_WRITE:
                printf("WRITE ");
                printOperand(cur->code->u.oneOp.op);
                break;
            printf("\n");
        }
    }
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
    if (!strcmp(secondChild->name, "FunDec"))
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
    pInterCodes newCode = newInterCodes(newInterCode(IR_FUNCTION, 1, 
        newOperand(OPERAND_FUNCTION, newString(child->value))));
    addInterCodesToWrap(interCodesWrap,newCode);

}

void translate_CompSt(pNode node)
{
    assert(node != NULL);
}