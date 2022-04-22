#include "inter.h"

pInterCodesWrap interCodesWrap;

/**
 * @brief 产生一个运算对象
 *
 * @param kind 什么类型的运算分量
 * @param value 运算分量可能是char *也可能是int，但都只有一个，因此使用void*
 */
pOperand newOperand(int kind, void *value)
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
        p->u.value = *((int *)value);
        break;
    case OPERAND_VARIABLE:
    case OPERAND_FUNCTION:
    case OPERAND_ADDRESS:
    case OPERAND_RELOP:
    case OPERAND_LABEL:
        p->u.name = (char *)value;
        break;
    }
    return p;
}

void updateOperand(pOperand p, int kind, void *value)
{
    assert(p != NULL);
    assert(kind >= 0 && kind < 6);
    p->kind = kind;
    switch (kind)
    {
    case OPERAND_CONSTANT:
        p->u.value = *((int *)value);
        break;
    case OPERAND_VARIABLE:
    case OPERAND_FUNCTION:
    case OPERAND_ADDRESS:
    case OPERAND_RELOP:
    case OPERAND_LABEL:
        FREE(p->u.name);
        p->u.name = (char *)value;
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
    case IR_LABEL:
    case IR_FUNCTION:
    case IR_GOTO:
    case IR_RETURN:
    case IR_ARG:
    case IR_PARAM:
    case IR_READ:
    case IR_WRITE:
        p->u.oneOp.op = va_arg(vaList, pOperand);
        break;
    case IR_ASSIGN:
    case IR_GET_ADDR:
    case IR_READ_ADDR:
    case IR_WRITE_ADDR:
    case IR_CALL:
        p->u.assign.left = va_arg(vaList, pOperand);
        p->u.assign.right = va_arg(vaList, pOperand);
        break;
    case IR_ADD:
    case IR_SUB:
    case IR_MUL:
    case IR_DIV:
        p->u.binOp.result = va_arg(vaList, pOperand);
        p->u.binOp.op1 = va_arg(vaList, pOperand);
        p->u.binOp.op2 = va_arg(vaList, pOperand);
        break;
    case IR_DEC:
        p->u.dec.op = va_arg(vaList, pOperand);
        p->u.dec.size = va_arg(vaList, int);
        break;
    case IR_IF_GOTO:

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
    while (tobeFreed != NULL)
    {
        pInterCodes temp = tobeFreed;
        tobeFreed = tobeFreed->next;
        freeInterCodes(temp);
    }
    codes->head = NULL;
    codes->tail = NULL;
    free(codes);
}

pOperand newTemp()
{
    char tName[10] = {0};
    sprintf(tName, "t%d", interCodesWrap->tempVarNum);
    interCodesWrap->tempVarNum++;
    pOperand temp = newOperand(OPERAND_VARIABLE, newString(tName));
    return temp;
}

pOperand newLabel()
{
    char lName[10] = {0};
    sprintf(lName, "label%d", interCodesWrap->labelNum);
    interCodesWrap->labelNum++;
    pOperand temp = newOperand(OPERAND_LABEL, newString(lName));
    return temp;
}

/**
 * @brief 打印运算分量到终端
 *
 * @param operand 运算分量
 */
void printOperand(pOperand operand)
{
    assert(operand != NULL);
    switch (operand->kind)
    {
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

unsigned int getSize(pType type)
{
    if (type == NULL)
        return 0;
    else if (type->kind == BASIC)
        return 4;
    else if (type->kind == ARRAY)
        return type->u.array.size * getSize(type->u.array.elem);
    else if (type->kind == STRUCTURE)
    {
        int size = 0;
        pFieldList temp = type->u.structure.structureField;
        while (temp)
        {
            size += getSize(temp->type);
            temp = temp->tail;
        }
        return size;
    }
    return 0;
}

/**
 * @brief 打印中间代码到终端，如果需要打印到文件，可以妥善利用linux的>
 *
 * @param interCodesWrap 中间代码结构包装
 */
void printInterCodes(pInterCodesWrap interCodesWrap)
{
    for (pInterCodes cur = interCodesWrap->head; cur != NULL; cur = cur->next)
    {
        assert(cur->code->kind >= 0 && cur->code->kind < 19);
        switch (cur->code->kind)
        {
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
        }
        printf("\n");
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
    addInterCodesToWrap(interCodesWrap, newCode);
    pTableItem tableItem = getSymbolTableItem(symbolTable, child->value);
    pFieldList argv = tableItem->field->type->u.function.argv;
    while (argv)
    {
        newCode = newInterCodes(newInterCode(IR_PARAM, 1,
                                             newOperand(OPERAND_VARIABLE, newString(argv->name))));
        addInterCodesToWrap(interCodesWrap, newCode);
        argv = argv->tail;
    }
}

void translate_CompSt(pNode node)
{
    assert(node != NULL);
    /*
    CompSt:             LC DefList StmtList RC
        |               error RC
    */
    pNode secondChild = node->child->brother;
    if (!strcmp(secondChild->name, "DefList"))
    {
        translate_DefList(secondChild);
    }
    if (!strcmp(secondChild->brother->name, "StmtList"))
    {
        translate_StmtList(secondChild->brother);
    }
}

void translate_DefList(pNode node)
{
    assert(node != NULL);
    /*
    DefList:            Def DefList
        |
    */
    while (node)
    {
        translate_Def(node->child);
        node = node->child->brother;
    }
}

void translate_Def(pNode node)
{
    assert(node != NULL);
    /*
    Def:                Specifier DecList SEMI
        ;
    */
    pNode child = node->child;
    //直接分析DecList，不用担心Specifier，因为已经在语义分析中分析过了
    if (!strcmp(child->brother->name, "DecList"))
    {
        translate_DecList(child->brother);
    }
}

void translate_DecList(pNode node)
{
    assert(node != NULL);
    /*
    DecList:            Dec
        |               Dec COMMA DecList
        ;
    */
    while (node)
    {
        translate_Dec(node->child);
        if (node->child->brother){
            node = node->child->brother->brother;
        }
        else{
            break;
        }
        
    }
}

void translate_Dec(pNode node)
{
    assert(node != NULL);
    /*
    Dec:                VarDec
        |               VarDec ASSIGNOP Exp
        ;
    */
    pNode child = node->child;
    // VarDec ASSIGNOP Exp
    if (child->brother)
    {
        pOperand t1 = newTemp();
        translate_VarDec(child->brother, t1);
        pOperand t2 = newTemp();
        translate_Exp(child->brother->brother, t2);
        //只用考虑简单变量的复制
        addInterCodesToWrap(interCodesWrap, newInterCodes(newInterCode(IR_ASSIGN, 2, t1, t2)));
    }
    // VarDec
    else
    {
        translate_VarDec(child, NULL);
    }
}

void translate_VarDec(pNode node, pOperand place)
{
    assert(node != NULL);
    /*
    VarDec:             ID
        |               VarDec LB INT RB
        |               error RB
        ;
    */
    // VarDec -> ID
    if (!strcmp(node->child->name, "ID"))
    {
        pTableItem temp = getSymbolTableItem(symbolTable, node->child->value);
        pType type = temp->field->type;
        if (type->kind == BASIC)
        {
            if (place)
            {
                interCodesWrap->tempVarNum--;
                updateOperand(place, OPERAND_VARIABLE,
                              newString(temp->field->name));
            }
            //如果只是简单的变量声明语句不用特地的打印中间代码
        }
        else if (type->kind == ARRAY)
        {
            // TODO 如果需要完成高维数组就还需要继续修改
            pInterCodes p = newInterCodes(newInterCode(
                IR_DEC,
                2,
                newOperand(OPERAND_VARIABLE, newString(temp->field->name)),
                getSize(type)));
            addInterCodesToWrap(interCodesWrap, p);
        }
        // else if (type->kind == STRUCTURE)
        // {
        //     // 3.1选做
        //     pInterCodes p = newInterCodes(newInterCode(
        //         IR_DEC,
        //         2,
        //         newOperand(OPERAND_VARIABLE, newString(temp->field->name)),
        //         getSize(type)));
        //     addInterCodesToWrap(interCodesWrap, p);
        // }
    }
    // VarDec -> VarDec LB INT RB
    else
    {
        translate_VarDec(node->child, place);
    }
}

/**
 * @brief 表达式的翻译，本来应该有返回值的，不过Unabletocode没写，因此这个函数有很多副作用，我正在慢慢修改
 *
 * @param exp
 * @param place
 */
void translate_Exp(pNode exp, pOperand place)
{
    assert(exp != NULL);
    // Exp -> Exp ASSIGNOP Exp
    //      | Exp AND Exp
    //      | Exp OR Exp
    //      | Exp RELOP Exp
    //      | Exp PLUS Exp
    //      | Exp MINUS Exp
    //      | Exp STAR Exp
    //      | Exp DIV Exp

    //      | MINUS Exp
    //      | NOT Exp
    //      | ID LP Args RP
    //      | ID LP RP
    //      | Exp LB Exp RB
    //      | Exp DOT ID
    //      | ID
    //      | INT
    //      | FLOAT

    // Exp -> LP Exp RP
    pNode child = exp->child;
    if (!strcmp(child->name, "LP"))
    {
        translate_Exp(child->brother, place);
    }
    else if (!strcmp(child->name, "Exp") ||
             !strcmp(child->name, "NOT"))
    {
        // 条件表达式 和 基本表达式
        if (strcmp(child->brother->name, "LB") &&
            strcmp(child->brother->name, "DOT"))
        {
            // Exp -> Exp AND Exp
            //      | Exp OR Exp
            //      | Exp RELOP Exp
            //      | NOT Exp
            //条件表达式
            /*
            方便理解的翻译例子：
            n = a > b;
            得到的中间代码：
            t3 := #0
            IF a > b GOTO label1
            GOTO label2
            LABEL label1 :
            t3 := #1
            LABEL label2 :
            n := t3
            */
            if (!strcmp(child->brother->name, "AND") ||
                !strcmp(child->brother->name, "OR") ||
                !strcmp(child->brother->name, "RELOP") ||
                !strcmp(child->name, "NOT"))
            {
                pOperand label1 = newLabel(interCodesWrap);
                pOperand label2 = newLabel(interCodesWrap);
                int TRUE_CONSTANT = 1;
                int FALSE_CONSTANT = 0;
                pOperand true_num = newOperand(OPERAND_CONSTANT, &TRUE_CONSTANT);
                pOperand false_num = newOperand(OPERAND_CONSTANT, &FALSE_CONSTANT);
                pInterCodes code0 = newInterCodes(newInterCode(IR_ASSIGN, 2, place, false_num));
                addInterCodesToWrap(interCodesWrap, code0);
                pInterCodes code1 = translate_Cond(exp, label1, label2);
                addInterCodesToWrap(interCodesWrap, code1);
                pInterCodes code2 = newInterCodes(newInterCode(IR_LABEL, 1, label1));
                addInterCodesToWrap(interCodesWrap, code2);
                pInterCodes code3 = newInterCodes(newInterCode(IR_ASSIGN, 2, place, true_num));
                addInterCodesToWrap(interCodesWrap, code3);
                pInterCodes code4 = newInterCodes(newInterCode(IR_LABEL, 1, label2));
                addInterCodesToWrap(interCodesWrap, code4);
            }
            else
            {
                // Exp -> Exp ASSIGNOP Exp
                if (!strcmp(child->brother->name, "ASSIGNOP"))
                {
                    //寻找左边的变量，因为可能为ID或者数组赋值
                    pOperand t1 = newTemp();
                    translate_Exp(child, t1);
                    pOperand t2 = newTemp();
                    pNode exp2 = child->brother->brother;
                    translate_Exp(exp2, t2);
                    //如果左边是数组,所以它是一个地址值
                    if (child->child->brother && strcmp(child->child->brother->name, "LB"))
                    {
                        //如果右边也是一个数组，所以它也是一个地址值
                        if (exp2->child->brother && strcmp(exp2->child->brother->name, "LB"))
                        {
                            pInterCodes code1 = newInterCodes(newInterCode(IR_READ_ADDR, 2, t2, t2));
                            addInterCodesToWrap(interCodesWrap, code1);
                        }
                        addInterCodesToWrap(interCodesWrap, newInterCodes(newInterCode(IR_WRITE_ADDR, 2, t1, t2)));
                    }
                    else
                    {
                        //如果右边也是一个数组，所以它也是一个地址值
                        if (exp2->child->brother && strcmp(exp2->child->brother->name, "LB"))
                        {
                            pInterCodes code1 = newInterCodes(newInterCode(IR_READ_ADDR, 2, t2, t2));
                            addInterCodesToWrap(interCodesWrap, code1);
                        }
                        addInterCodesToWrap(interCodesWrap, newInterCodes(newInterCode(IR_ASSIGN, 2, t1, t2)));
                    }
                    // 这里无论是地址还是变量都应该使用这条语句
                    addInterCodesToWrap(interCodesWrap, newInterCodes(newInterCode(IR_ASSIGN, 2, place, t1)));
                }
                //      | Exp PLUS Exp
                //      | Exp MINUS Exp
                //      | Exp STAR Exp
                //      | Exp DIV Exp
                else
                {
                    pOperand t1 = newTemp();
                    translate_Exp(child, t1);
                    //如果t1现在是数组的地址,因此需要从t1中读取值
                    if (child->child->brother && !strcmp(child->child->brother->name, "LB"))
                    {
                        pInterCodes addCode = newInterCodes(newInterCode(IR_READ_ADDR, 2, t1, t1));
                        addInterCodesToWrap(interCodesWrap, addCode);
                    }
                    pOperand t2 = newTemp();
                    pNode exp2 = child->brother->brother;
                    translate_Exp(exp2, t2);
                    if (exp2->child->brother && !strcmp(exp2->child->brother->name, "LB"))
                    {
                        pInterCodes addCode = newInterCodes(newInterCode(IR_READ_ADDR, 2, t2, t2));
                        addInterCodesToWrap(interCodesWrap, addCode);
                    }

                    // Exp -> Exp PLUS Exp
                    if (!strcmp(child->brother->name, "PLUS"))
                    {
                        pInterCodes addCode = newInterCodes(newInterCode(IR_ADD, 3, place, t1, t2));
                        addInterCodesToWrap(interCodesWrap, addCode);
                    }
                    // Exp -> Exp MINUS Exp
                    else if (!strcmp(child->brother->name, "MINUS"))
                    {
                        pInterCodes subCode = newInterCodes(newInterCode(IR_SUB, 3, place, t1, t2));
                        addInterCodesToWrap(interCodesWrap, subCode);
                    }
                    // Exp -> Exp STAR Exp
                    else if (!strcmp(child->brother->name, "STAR"))
                    {
                        pInterCodes starCode = newInterCodes(newInterCode(IR_MUL, 3, place, t1, t2));
                        addInterCodesToWrap(interCodesWrap, starCode);
                    }
                    // Exp -> Exp DIV Exp
                    else if (!strcmp(child->brother->name, "DIV"))
                    {
                        pInterCodes divCode = newInterCodes(newInterCode(IR_DIV, 3, place, t1, t2));
                        addInterCodesToWrap(interCodesWrap, divCode);
                    }
                }
            }
        }
        // 数组和结构体访问
        // TODO 先写到这里
        else
        {
            // Exp -> Exp LB Exp RB
            // 数组
            if (!strcmp(child->brother->name, "LB"))
            {
                // 高维数组
                if (child->child->brother && !strcmp(child->child->brother->name, "LB"))
                {
                    perror("没有选做高维数组，翻译失败");
                    return;
                }
                // 低维数组
                else
                {
                    pOperand idx = newTemp();
                    translate_Exp(child->brother->brother, idx);
                    pOperand base = newTemp();
                    translate_Exp(child, base);

                    pOperand width;
                    pOperand offset = newTemp();
                    pOperand target;
                    // 根据假设，Exp1只会展开为 Exp DOT ID 或 ID
                    // 我们让前一种情况吧ID作为name回填进place返回到这里的base处，
                    // 在语义分析时将结构体变量也填进表（因为假设无重名），这样两种情况都可以查表得到。
                    pTableItem item = getSymbolTableItem(symbolTable, base->u.name);
                    assert(item->field->type->kind == ARRAY);
                    unsigned size = getSize(item->field->type->u.array.elem);
                    width = newOperand(
                        OPERAND_CONSTANT, &size);
                    addInterCodesToWrap(interCodesWrap, newInterCodes(newInterCode(IR_MUL, 3, offset, idx, width)));
                    target = newTemp();
                    addInterCodesToWrap(interCodesWrap, newInterCodes(newInterCode(IR_GET_ADDR, 2, target, base)));
                    // // 如果是ID[Exp],
                    // // 则需要对ID取址，如果前面是结构体内访问，则会返回一个地址类型，不需要再取址
                    // if (base->kind == OPERAND_VARIABLE)
                    // {
                    //     printf("非结构体数组访问\n");
                    //     target = newTemp();
                    //     pInterCodes code = newInterCodes(newInterCode(IR_GET_ADDR, target, base));
                    //     addInterCodesToWrap(interCodesWrap, code);
                    // }
                    // else
                    // {
                    //     // printf("结构体数组访问\n");
                    //     target = base;
                    // }
                    addInterCodesToWrap(interCodesWrap, newInterCodes(newInterCode(IR_ADD, 3, place, target, offset)));
                    // 注意：现在place中放置的值是对应数组的下标的地址
                }
            }
            // Exp -> Exp DOT ID
            else
            {
                perror("没有选做结构体，因此不完成");
                // //结构体
                // pOperand temp = newTemp();
                // translate_Exp(child, temp);
                // // 两种情况，Exp直接为一个变量，则需要先取址，
                // // 若Exp为数组或者多层结构体访问或结构体形参，则target会被填成地址，可以直接用。
                // pOperand target;
                // if (temp->kind == OPERAND_ADDRESS)
                // {
                //     target = newOperand(temp->kind, temp->u.name);
                // }
                // else
                // {
                //     target = newTemp();
                //     addInterCodesToWrap(interCodesWrap,newInterCodes(newInterCode(IR_GET_ADDR,target,temp)));
                // }
                // pOperand id = newOperand(
                //     OPERAND_VARIABLE, newString(child->brother->brother->value));
                // unsigned offset = 0;
                // pTableItem item = getSymbolTableItem(symbolTable, temp->u.name);
                // //结构体数组，temp是临时变量，查不到表，需要用处理数组时候记录下的数组名查表
                // if (item == NULL)
                // {
                //     item = getSymbolTableItem(symbolTable, interCodesWrap->lastArrayName);
                // }
                // pFieldList tmp;
                // // 结构体数组 eg: a[5].b
                // if (item->field->type->kind == ARRAY)
                // {
                //     tmp = item->field->type->u.array.elem->u.structure.structureField;
                // }
                // // 一般结构体
                // else
                // {
                //     tmp = item->field->type->u.structure.structureField;
                // }
                // // 遍历获得offset
                // while (tmp)
                // {
                //     if (!strcmp(tmp->name, id->u.name))
                //         break;
                //     offset += getSize(tmp->type);
                //     tmp = tmp->tail;
                // }
                // pOperand tOffset = newOperand(OPERAND_CONSTANT, offset);
                // if (place)
                // {
                //     addInterCodesToWrap(interCodesWrap,newInterCodes(newInterCode(IR_ADD,3,place, target, tOffset)));
                //     // 为了处理结构体里的数组把id名通过place回传给上层
                //     updateOperand(place, OPERAND_ADDRESS, (void *)newString(id->u.name));
                //     // place->isAddr = TRUE;
                // }
            }
        }
    }
    //单目运算符
    // Exp -> MINUS Exp
    else if (!strcmp(child->name, "MINUS"))
    {
        pOperand t1 = newTemp();
        pNode exp2 = child->brother;
        translate_Exp(exp2, t1);
        pOperand zero = newOperand(OPERAND_CONSTANT, 0);
        // 如果是数组
        if (exp2->child->brother && !strcmp(exp2->child->brother->name, "LB"))
        {
            addInterCodesToWrap(interCodesWrap, newInterCodes(newInterCode(IR_READ_ADDR, 2, t1, t1)));
        }
        addInterCodesToWrap(interCodesWrap, newInterCodes(newInterCode(IR_SUB, 3, place, zero, t1)));
    }
    // 函数调用
    // Exp -> ID LP Args RP
    //		| ID LP RP
    else if (!strcmp(child->name, "ID") && child->brother)
    {
        pOperand funcTemp =
            newOperand(OPERAND_FUNCTION, newString(child->value));
        // Exp -> ID LP Args RP
        // 带参数的函数调用
        if (!strcmp(child->brother->brother->name, "Args"))
        {
            translate_Args(child->brother->brother);
            if (!strcmp(child->value, "write"))
            {
                // 因为write传递函数参数的方式不一样因此需要如下修改
                pOperand temp = interCodesWrap->tail->code->u.oneOp.op;
                pInterCodes prevCode = interCodesWrap->tail->prev;
                freeInterCodes(interCodesWrap->tail);
                interCodesWrap->tail = prevCode;
                addInterCodesToWrap(interCodesWrap, newInterCodes(newInterCode(IR_WRITE, 1, temp)));
            }
            else
            {
                if (place)
                {
                    addInterCodesToWrap(interCodesWrap, newInterCodes(newInterCode(
                                                            IR_CALL, 2, place, funcTemp)));
                }
                else
                {
                    pOperand temp = newTemp();
                    addInterCodesToWrap(interCodesWrap, newInterCodes(newInterCode(
                                                            IR_CALL, 2, temp, funcTemp)));
                }
            }
        }
        // Exp -> ID LP RP
        else
        {
            if (!strcmp(child->value, "read"))
            {
                addInterCodesToWrap(interCodesWrap, newInterCodes(newInterCode(IR_READ, 1, place)));
            }
            else
            {
                if (place)
                {
                    addInterCodesToWrap(interCodesWrap, newInterCodes(newInterCode(
                                                            IR_CALL, 2, place, funcTemp)));
                }
                else
                {
                    pOperand temp = newTemp();
                    addInterCodesToWrap(interCodesWrap, newInterCodes(newInterCode(
                                                            IR_CALL, 2, temp, funcTemp)));
                }
            }
        }
    }
    // Exp -> ID
    else if (!strcmp(child->name, "ID"))
    {
        // pTableItem item = getSymbolTableItem(symbolTable, child->value);
        interCodesWrap->tempVarNum--;
        updateOperand(place, OPERAND_VARIABLE, newString(child->value));
        // 根据讲义，因为结构体不允许赋值，结构体做形参时是传址的方式
        // 仔细思考，这里的应用场景应该是当结构体做形参的时候，然后去使用的时候
        // if (item->field->isParam = true && item->field->type->kind == STRUCTURE)
        // {
        //     updateOperand(place, OPERAND_ADDRESS, newString(child->value));
        // }
        // 非结构体参数情况都当做变量处理
        // 即使此时是结构体，因为是通过DEC t1 得到的，t1是地址
        // else
        // {
        // updateOperand(place, OPERAND_VARIABLE, newString(child->value));
        // }
    }
    else
    {
        // Exp -> INT
        interCodesWrap->tempVarNum--;
        //因为updateOperand需要的是void *
        int constant_Int = atoi(child->value);
        updateOperand(place, OPERAND_CONSTANT, &constant_Int);
    }
}

void translate_Args(pNode node)
{
    assert(node != NULL);
    /*
    Args :              Exp COMMA Args
         |               Exp
    ;
     */
    pNode child = node->child;
    pOperand temp = newTemp();
    translate_Exp(child, temp);
    addInterCodesToWrap(interCodesWrap, newInterCodes(newInterCode(IR_ARG, 1, temp)));
    // Args -> Exp COMMA Args
    if (child->brother)
    {
        translate_Args(child->brother->brother);
    }
}

/**
 * @brief 条件表达式的翻译模式
 *
 * @param node
 * @param p1
 * @param p2
 */
pInterCodes translate_Cond(pNode exp, pOperand p1, pOperand p2)
{
}

void translate_StmtList(pNode node)
{
}