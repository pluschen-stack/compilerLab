#include "semantics.h"
#include "util.h"

pSymbolTable symbolTable;
pFuncDecStack funcDeckStack;

/**
 * @brief 来自P.J.Weinberger提供的hash函数
 *
 * @param name 符号名
 * @return unsigned int hash值
 */
inline unsigned int hash_pjw(char *name)
{
    unsigned int val = 0, i;
    for (; *name; ++name)
    {
        val = (val << 2) + *name;
        if (i = val & ~SYMBOL_TABLE_SIZE)
        {
            val = (val ^ (i >> 12)) & SYMBOL_TABLE_SIZE;
        }
    }
    return val;
}

/**
 * @brief 用于打印错误信息
 *
 * @param type 错误类型
 * @param lineNumber 行号
 * @param name 错误地方的名字
 */
inline void pError(ErrorType type, int lineNumber, char *name)
{
    char msg[100] = "\0";
    if (type == UNDEF_VAR)
        sprintf(msg, "Undefined variable \"%s\".", name);
    else if (type == UNDEF_FUNC)
        sprintf(msg, "Undefined function \"%s\".", name);
    else if (type == REDEF_VAR)
        sprintf(msg, "Redefined variable \"%s\".", name);
    else if (type == REDEF_FUNC)
        sprintf(msg, "Redefined function \"%s\".", name);
    else if (type == TYPE_MISMATCH_ASSIGN)
        sprintf(msg, "Type mismatched for assignment.");
    else if (type == LEFT_VAR_ASSIGN)
        sprintf(msg, "The left-hand side of an assignment must be a variable.");
    else if (type == TYPE_MISMATCH_OP)
        sprintf(msg, "Type mismatched for operands.");
    else if (type == TYPE_MISMATCH_RETURN)
        sprintf(msg, "Type mismatched for return.");
    else if (type == FUNC_AGRC_MISMATCH)
        sprintf(msg, "Function \"%s\" is not applicable for arguments followed.", name);
    else if (type == NOT_A_ARRAY)
        sprintf(msg, "\"%s\" is not an array.", name);
    else if (type == NOT_A_FUNC)
        sprintf(msg, "\"%s\" is not a function.", name);
    else if (type == NOT_A_INT)
        sprintf(msg, "\"%s\" is not an integer.", name);
    else if (type == ILLEGAL_USE_DOT)
        sprintf(msg, "Illegal use of \".\".");
    else if (type == NONEXISTFIELD)
        sprintf(msg, "Non-existent field \"%s\".", name);
    else if (type == REDEF_FEILD)
    { // two different error report format
        if (name == NULL)
            sprintf(msg, "Field cannot be initialized.");
        else
            sprintf(msg, "Redefined field \"%s\".", name);
    }
    else if (type == DUPLICATED_NAME)
        sprintf(msg, "Duplicated name \"%s\".", name);
    else if (type == UNDEF_STRUCT)
        sprintf(msg, "Undefined structure \"%s\".", name);
    else if(type == DCLARE_BUTUNDEF_FUNC)
        sprintf(msg, "Undefined function \"%s\".", name);
    else if(type == DCLARE_FUNC_INCONSISTENT)
        sprintf(msg, "Inconsistent declaration of function \"%s\".", name);
    else
        printf("Unknown type\n");
    printf("Error type %d at Line %d: %s\n", type, lineNumber, msg);
}

/**
 * @brief 产生一个类型
 *
 * @param kind
 * @param ...
 * @return pType
 */
pType newType(Kind kind, ...)
{
    pType type = malloc(sizeof(struct Type_));
    assert(type != NULL);
    type->kind = kind;
    va_list vaList;
    switch (kind)
    {
    case BASIC:
        va_start(vaList, 1);
        type->u.basic = va_arg(vaList, BasicType);
        break;
    case ARRAY:
        va_start(vaList, 2);
        type->u.array.elem = va_arg(vaList, pType);
        type->u.array.size = va_arg(vaList, int);
        break;
    case STRUCTURE:
        va_start(vaList, 2);
        type->u.structure.name = va_arg(vaList, char *);
        type->u.structure.structureField = va_arg(vaList, pFieldList);
        break;
    case FUNCTION:
        va_start(vaList, 3);
        type->u.function.argc = va_arg(vaList, int);
        type->u.function.argv = va_arg(vaList, pFieldList);
        type->u.function.returnType = va_arg(vaList, pType);
        break;
    default:
        perror("error kind");
        exit(1);
        break;
    }
    va_end(vaList);
    return type;
}

void printType(pType type)
{
    if (type == NULL)
    {
        printf("type is NULL.\n");
    }
    else
    {
        printf("type kind: %d\n", type->kind);
        switch (type->kind)
        {
        case BASIC:
            printf("type basic: %d\n", type->u.basic);
            break;
        case ARRAY:
            printf("array size: %d\n", type->u.array.size);
            printType(type->u.array.elem);
            break;
        case STRUCTURE:
            if (!type->u.structure.name)
                printf("struct name is NULL\n");
            else
            {
                printf("struct name is %s\n", type->u.structure.name);
            }
            printFieldList(type->u.structure.structureField);
            break;
        case FUNCTION:
            printf("function argc is %d\n", type->u.function.argc);
            printf("function args:\n");
            printFieldList(type->u.function.argv);
            printf("function return type:\n");
            printType(type->u.function.returnType);
            break;
        }
    }
}

/**
 * @brief 用于复制type类型，在结构体内部定义中很有用，避免因为删除某一个Type导致出现其他
 * fieldList中的type不可以使用
 *
 * @param srcType
 * @return pType
 */
pType copyType(pType srcType)
{
    if (srcType == NULL)
        return NULL;
    pType p = (pType)malloc(sizeof(struct Type_));
    assert(p != NULL);
    p->kind = srcType->kind;
    switch (p->kind)
    {
    case BASIC:
        p->u.basic = srcType->u.basic;
        break;
    case ARRAY:
        p->u.array.elem = copyType(srcType->u.array.elem);
        p->u.array.size = srcType->u.array.size;
        break;
    case STRUCTURE:
        p->u.structure.name = newString(srcType->u.structure.name);
        p->u.structure.structureField = copyFieldList(srcType->u.structure.structureField);
        break;
    case FUNCTION:
        p->u.function.argc = srcType->u.function.argc;
        p->u.function.argv = copyFieldList(srcType->u.function.argv);
        p->u.function.returnType = copyType(srcType->u.function.returnType);
        break;
    default:
        fprintf(stderr, "error kind in %s", __FUNCTION__);
        exit(1);
        break;
    }
    return p;
}

bool checkType(pType type1, pType type2)
{
    if (type1 == NULL || type2 == NULL)
        return true;
    if (type1->kind == FUNCTION || type2->kind == FUNCTION)
    {
        /*  如果比较两个的是函数，应当比较它们的名字，而不以checkType的判断为准，
        因为函数名不会被重定义
        */
        return false;
    }
    if (type1->kind != type2->kind)
        return false;
    else
    {
        switch (type1->kind)
        {
        case BASIC:
            return type1->u.basic == type2->u.basic;
        case ARRAY:
            return checkType(type1->u.array.elem, type2->u.array.elem);
        case STRUCTURE:
            return !strcmp(type1->u.structure.name,
                           type2->u.structure.name);
            //这个对于大多数结构体好像返回的都是true，因为设计不合理。。
        }
    }
}

void freeType(pType type)
{
    assert(type != NULL);
    switch (type->kind)
    {
    case BASIC:
        break;
    case ARRAY:
        freeType(type->u.array.elem); //  递归删除多维数组
        type->u.array.elem = NULL;
        break;
    case STRUCTURE:
        if (type->u.structure.name)
            FREE(type->u.structure.name);
        type->u.structure.name = NULL;

        pFieldList temp = type->u.structure.structureField;
        while (temp)
        {
            pFieldList toBeFree = temp;
            temp = temp->tail;
            freeFieldList(toBeFree);
        }
        type->u.structure.structureField = NULL;
        break;
    case FUNCTION:
        freeType(type->u.function.returnType);
        type->u.function.returnType = NULL;
        temp = type->u.function.argv;
        while (temp)
        {
            pFieldList toBeFree = temp;
            temp = temp->tail;
            freeFieldList(toBeFree);
        }
        type->u.function.argv = NULL;
        break;
    }
    FREE(type);
}

/**
 * @brief 新建一个表项
 *
 * @param depth 深度
 * @param feildList 域
 * @return pTableItem 返回一个带有深度和域，同时两个尾指针是空的表项
 */
pTableItem newTableItem(int depth, pFieldList feildList)
{
    pTableItem tableItem = (pTableItem)malloc(sizeof(struct TableItem_));
    assert(tableItem != NULL);
    tableItem->symbolDepth = depth;
    tableItem->field = feildList;
    tableItem->nextHash = NULL;
    tableItem->nextSymbol = NULL;
    return tableItem;
}

/**
 * @brief 向符号表中加入一个符号项
 *
 * @param newTableItem 符号表项
 */
void insertTableItem(pSymbolTable symbolTable, pTableItem newTableItem)
{
    assert(newTableItem != NULL);
    unsigned int hashCode = hash_pjw(newTableItem->field->name);
    pHashTable hashTable = symbolTable->hashTable;
    pStack stack = symbolTable->stack;

    newTableItem->nextHash = GET_HASH_HEAD(hashTable, hashCode);
    SET_HASH_HEAD(hashTable, hashCode, newTableItem);

    newTableItem->nextSymbol = GET_STACK_HEAD(stack);
    SET_STACK_HEAD(stack, newTableItem);
}

/**
 * @brief 按照name在hash表中查找最近的tableItem
 *
 * @param hashTable hash表
 * @param name 项名
 * @return pTableItem 如果存在则返回，否则返回NULL
 */
pTableItem getSymbolTableItem(pSymbolTable table, char *name)
{
    unsigned int hashCode = hash_pjw(name);
    pTableItem temp = GET_HASH_HEAD(table->hashTable, hashCode);
    while (temp)
    {
        if (!strcmp(temp->field->name, name))
        {
            return temp;
        }
        temp = temp->nextHash;
    }
    return NULL;
}

/**
 * @brief
 *
 * @param tableItem
 */
void freeTableItem(pTableItem tableItem)
{
    assert(tableItem != NULL);
    if (tableItem->field != NULL)
        freeFieldList(tableItem->field);
    FREE(tableItem);
}

pFieldList newFieldList(char *name, pType type)
{
    pFieldList fieldList = (pFieldList)malloc(sizeof(struct FieldList_));
    assert(fieldList != NULL);
    fieldList->name = newString(name);
    fieldList->type = type;
    fieldList->tail = NULL;
    return fieldList;
}

/**
 * @brief 方便debug所以很需要这个来检查错误
 *
 * @param fieldList
 */
void printFieldList(pFieldList fieldList)
{
    if (fieldList == NULL)
        printf("fieldList is NULL\n");
    else
    {
        printf("fieldList name is: %s\n", fieldList->name);
        printf("FieldList Type:\n");
        printType(fieldList->type);
        printFieldList(fieldList->tail);
    }
}

pFieldList copyFieldList(pFieldList srcFeildList)
{
    pFieldList head = NULL, cur = NULL;
    pFieldList temp = srcFeildList;

    while (temp)
    {
        if (!head)
        {
            head = newFieldList(temp->name, copyType(temp->type));
            cur = head;
            temp = temp->tail;
        }
        else
        {
            cur->tail = newFieldList(temp->name, copyType(temp->type));
            cur = cur->tail;
            temp = temp->tail;
        }
    }
    return head;
}
void freeFieldList(pFieldList feildList)
{
    assert(feildList != NULL);
    FREE(feildList->name);
    if (feildList->type)
        freeType(feildList->type);
    feildList->type == NULL;
    FREE(feildList);
}

/**
 * @brief 新建一个hash表
 *
 * @return pHashTable
 */
pHashTable newHashTable()
{
    pHashTable hashTable = malloc(sizeof(struct HashTable_));
    assert(hashTable != NULL);
    hashTable->hashArray = malloc(sizeof(struct TableItem_) * SYMBOL_TABLE_SIZE);
    assert(hashTable->hashArray != NULL);
    for (int i = 0; i < SYMBOL_TABLE_SIZE; i++)
    {
        hashTable->hashArray[i] = NULL;
    }
    return hashTable;
}

/**
 * @brief 释放hash表的空间
 *
 * @param hashTable hash表
 */
void freeHashTable(pHashTable hashTable)
{
    if (!hashTable)
    {
        for (int i = 0; i < SYMBOL_TABLE_SIZE; i++)
        {
            pTableItem temp = hashTable->hashArray[i];
            while (temp)
            {
                pTableItem toBeFreeTemp = temp;
                temp = temp->nextHash;
                freeTableItem(toBeFreeTemp);
            }
            hashTable->hashArray[i] = NULL;
        }
    }
    FREE(hashTable->hashArray);
    FREE(hashTable);
}

/**
 * @brief 新建一个栈
 *
 * @return pStack
 */
pStack newStack()
{
    pStack stack = malloc(sizeof(struct Stack_));
    assert(stack != NULL);
    stack->stackArray = malloc(sizeof(struct TableItem_) * SYMBOL_TABLE_SIZE);
    assert(stack->stackArray != NULL);
    for (int i = 0; i < SYMBOL_TABLE_SIZE; i++)
    {
        stack->stackArray[i] = NULL;
    }
    stack->stackDepth = 0;
    return stack;
}

void printStack(pStack stack)
{
    pTableItem item = GET_STACK_HEAD(stack);
    while (item)
    {
        printf("[%d] -> name: %s depth: %d address :%p \n", hash_pjw(item->field->name), item->field->name,
               item->symbolDepth, item);
        printf("========FiledList========\n");
        printFieldList(item->field);
        printf("===========End===========\n");
        item = item->nextSymbol;
        printf("\n");
    }
}
/**
 * @brief 清除符号表的栈的最高层
 *
 * @param symbolTable 一个符号表
 */
void clearHeadLayerStack(pSymbolTable symbolTable)
{
    assert(symbolTable != NULL);
    pStack stack = symbolTable->stack;
    pHashTable hashTable = symbolTable->hashTable;
    pTableItem temp = GET_STACK_HEAD(stack);
    while (temp)
    {
        pTableItem tobeFree = temp;
        temp = temp->nextSymbol;
        unsigned hashCode = hash_pjw(tobeFree->field->name);
        pTableItem prev = GET_HASH_HEAD(hashTable, hashCode);
        //  上面一定要用pTableItem承接GET_HASH_HEAD的值，不然比较不了。可能是因为我宏定
        //  义的是一个三元表达式
        //  如果要删除的项在hash表最内层那就很容易，直接删掉链接就好了
        if (tobeFree == prev)
        {
            SET_HASH_HEAD(hashTable, hashCode, tobeFree->nextHash);
        }
        //  如果要删除的项在hash表的中间层及更后面
        else
        {
            //  因为prev肯定不是了，所以直接比较它的儿子
            while (prev->nextHash != tobeFree && prev->nextHash != NULL)
            {
                prev = prev->nextHash;
            }
            prev->nextHash = tobeFree->nextHash;
        }
        freeTableItem(tobeFree);
    }

    SET_STACK_HEAD(stack, NULL);
}

/**
 * @brief 释放栈空间，需要注意的是由于这个函数需配合freeHashTable使用，并且在其后调用，不然存在内存泄露
 *
 * @param stack 栈
 */
void freeStack(pStack stack)
{
    assert(stack != NULL);
    FREE(stack->stackArray)
    stack->stackDepth = 0;
    FREE(stack);
}

/**
 * @brief 新建一个符号表
 *
 * @return pSymbolTable
 */
pSymbolTable initSymbolTable()
{
    pSymbolTable symbolTable = malloc(sizeof(struct SymbolTable_));
    assert(symbolTable != NULL);
    symbolTable->hashTable = newHashTable();
    symbolTable->stack = newStack();
    symbolTable->unamedStructNum = 0;
    return symbolTable;
}

void printSymbolTable(pSymbolTable table)
{
    printf("----------------hash_table----------------\n");
    for (int i = 0; i < SYMBOL_TABLE_SIZE; i++)
    {
        pTableItem item = GET_HASH_HEAD(table->hashTable, i);
        if (item)
        {
            printf("[%d]", i);
            while (item)
            {
                printf(" -> name: %s depth: %d address:%p\n", item->field->name,
                       item->symbolDepth, item);
                printf("========FiledList========\n");
                printFieldList(item->field);
                printf("===========End===========\n");
                item = item->nextHash;
            }
            printf("\n");
        }
    }
    printf("-------------------end--------------------\n");
}

/**
 * @brief 这个函数的实现对于结构体的定义，也就是结构体中的变量到底算第几级
 * 有问题，如果不影响验收就不改了，当然也有可能是我还没有想明白
 * 补充：根据假设七可知可以不用考虑这种问题
 *
 * @param table
 * @param item
 * @return true
 * @return false
 */
bool checkTableItemConflict(pSymbolTable table, pTableItem item)
{
    pTableItem temp = getSymbolTableItem(table, item->field->name);
    if (temp == NULL)
        return false;
    while (temp)
    {
        if (!strcmp(temp->field->name, item->field->name))
        {
            if (temp->field->type->kind == STRUCTURE ||
                item->field->type->kind == STRUCTURE)
                return true;
            if (temp->symbolDepth == table->stack->stackDepth)
            {
                return true;
            }
        }
        temp = temp->nextHash;
    }
    return false;
}

/**
 * @brief 释放符号表的空间
 *
 * @param symbolTable 符号表
 */
void freeSymbolTable(pSymbolTable symbolTable)
{
    freeHashTable(symbolTable->hashTable);
    symbolTable->hashTable = NULL;
    freeStack(symbolTable->stack);
    symbolTable->stack = NULL;
    symbolTable->unamedStructNum = 0;
    FREE(symbolTable);
}

/**
 * @brief 按照先根遍历开始语义分析
 *
 * @param currentNode 语法树的节点
 */
void startSemanticAnalysis(pNode currentNode)
{
    if (currentNode)
    {
        if (!strcmp(currentNode->name, "ExtDef"))
        {
            ExtDef(currentNode);
            startSemanticAnalysis(currentNode->brother);
            return;
        }
        startSemanticAnalysis(currentNode->child);
        startSemanticAnalysis(currentNode->brother);
    }
}

/**
 * @brief 对ExtDef这课树进行语义分析
 *
 * @param currentNode 当前节点要求为ExtDef
 */
void ExtDef(pNode currentNode)
{
    /*
    ExtDef:             Specifier ExtDecList SEMI
        |               Specifier SEMI
        |               Specifier FunDec CompSt
        |               Specifier FunDec SEMI
    */
    assert(currentNode != NULL);
    pNode secondChild = currentNode->child->brother;
    pType type = Specifier(currentNode->child);
    if (!strcmp(secondChild->name, "ExtDecList"))
    {
        ExtDecList(secondChild, type);
    }
    else if (!strcmp(secondChild->name, "FunDec"))
    {
        FunDec(secondChild, type);
        if (!strcmp(secondChild->brother->name, "CompSt"))
        {
            //只有函数定义的时候才需要进来
            CompSt(secondChild->brother, type);
        }
        else
        {
            //需要将FunDec中添加的深度为1的栈清除
            STACK_INC_DEPTH(symbolTable->stack);
            clearHeadLayerStack(symbolTable);
            STACK_DEC_DEPTH(symbolTable->stack);
        }
    }
}

/**
 * @brief
 *
 * @param currentNode
 * @return pType
 */
pType Specifier(pNode currentNode)
{
    /*
    Specifier:          TYPE
        |               StructSpecifier
    */
    assert(currentNode != NULL);
    pType resultType;
    assert(currentNode->child != NULL);
    pNode child = currentNode->child;
    if (!strcmp(child->name, "TYPE"))
    {
        if (!strcmp(child->value, "float"))
        {
            return newType(BASIC, FLOAT_TYPE_);
        }
        else if (!strcmp(child->value, "int"))
        {
            return newType(BASIC, INT_TYPE_);
        }
    }
    else if (!strcmp(child->name, "StructSpecifier"))
    {
        return StructSpecifier(currentNode->child);
    }
}

bool isStructDef(pTableItem tableItem)
{
    if (tableItem == NULL)
        return false;
    if (tableItem->field->type->kind != STRUCTURE)
        return false;
    if (tableItem->field->type->u.structure.name)
        return false;
    return true;
}

pType StructSpecifier(pNode currentNode)
{
    /*
    StructSpecifier:    STRUCT OptTag LC DefList RC
        |               STRUCT Tag

    OptTag -> ID | e 在语法分析中如果OptTag为空就不会建立节点
    Tag -> ID
    */

    assert(currentNode != NULL);
    pType returnType = NULL;
    assert(currentNode->child != NULL);
    pNode child = currentNode->child->brother;

    if (!strcmp(child->name, "Tag"))
    {
        pTableItem structureItem = getSymbolTableItem(symbolTable, child->child->value);

        if (structureItem == NULL || !isStructDef(structureItem))
        {
            pError(UNDEF_STRUCT, currentNode->lineno, child->child->value);
        }
        else
            returnType = newType(
                STRUCTURE, newString(structureItem->field->name),
                copyFieldList(structureItem->field->type->u.structure.structureField));
    }
    // OptTag -> ID | e
    else
    {
        pTableItem structureItem =
            newTableItem(symbolTable->stack->stackDepth,
                         newFieldList(NULL, newType(STRUCTURE, NULL, NULL)));
        // OptTag -> ID
        if (!strcmp(child->name, "OptTag"))
        {
            SET_FEILDLIST_NAME(structureItem->field, child->child->value);
            child = child->brother->brother;
        }
        // OptTag -> e
        else
        {
            char msg[30] = "\0";
            sprintf(msg, "unamedStruct%d", symbolTable->unamedStructNum++);
            SET_FEILDLIST_NAME(structureItem->field, msg);
            child = child->brother;
        }
        if (!strcmp(child->name, "DefList"))
        {
            DefList(child, structureItem);
        }

        //存在相同结构体定义
        if (checkTableItemConflict(symbolTable, structureItem))
        {
            pError(DUPLICATED_NAME, currentNode->lineno, structureItem->field->name);
            freeTableItem(structureItem);
        }
        //不存在相同结构体定义
        else
        {
            returnType = newType(
                STRUCTURE, newString(structureItem->field->name),
                copyFieldList(structureItem->field->type->u.structure.structureField));
            if (!strcmp(currentNode->child->brother->name, "OptTag"))
            {
                insertTableItem(symbolTable, structureItem);
            }
            // OptTag -> e
            else
            {
                freeTableItem(structureItem);
            }
        }
    }

    return returnType;
}

/**
 * @brief 创建结构体清单，得到的东西将写到structureItem中
 *
 * @param currentNode
 * @param structureItem
 */
void DefList(pNode currentNode, pTableItem structureItem)
{
    /*
    因为DefList可能是空的
    DefList:            Def DefList
        |
    */
    while (currentNode)
    {
        Def(currentNode->child, structureItem);
        currentNode = currentNode->child->brother;
    }
}

/**
 * @brief
 *
 * @param currentNode
 * @param structureItem
 */
void Def(pNode currentNode, pTableItem structureItem)
{
    /*
    Def:                Specifier DecList SEMI
        ;
    */
    assert(currentNode != NULL);
    pNode child = currentNode->child;
    pType type = Specifier(child);
    DecList(currentNode->child->brother, type, structureItem);
    if (type)
        freeType(type); //使用完后有关的type就已经在structureItem中了，因此可以去掉了
}

/**
 * @brief
 *
 * @param currentNode
 * @param type
 * @param structureItem
 */
void DecList(pNode currentNode, pType type, pTableItem structureItem)
{
    /*
    DecList:            Dec
        |               Dec COMMA DecList
        ;
    */
    while (currentNode)
    {
        Dec(currentNode->child, type, structureItem);
        currentNode = currentNode->child->brother ? currentNode->child->brother->brother : NULL;
    }
}

void Dec(pNode currentNode, pType type, pTableItem structureItem)
{
    /*
    Dec:                VarDec
        |               VarDec ASSIGNOP Exp
    */
    assert(currentNode != NULL);
    pNode child = currentNode->child;
    // Dec -> VarDec ASSIGNOP Exp
    if (child->brother)
    {
        //处在结构体定义内
        if (structureItem)
        {
            pError(REDEF_FEILD, currentNode->lineno,
                   NULL);
        }
        //在函数的定义语句中的赋值语句
        else
        {
            // 判断赋值类型是否相符
            //如果成功，注册该符号
            pTableItem tableItem = newTableItem(
                symbolTable->stack->stackDepth, VarDec(child, type));
            pType exptype = Exp(child->brother->brother);
            if (checkTableItemConflict(symbolTable, tableItem))
            {
                pError(REDEF_VAR, currentNode->lineno, tableItem->field->name);
                freeTableItem(tableItem);
            }
            if (!checkType(tableItem->field->type, exptype))
            {
                //类型不相符
                //报错
                pError(TYPE_MISMATCH_ASSIGN, currentNode->lineno,
                       NULL);
                freeTableItem(tableItem);
            }
            if (tableItem->field->type && tableItem->field->type->kind == ARRAY)
            {
                //报错，对非basic类型赋值
                pError(TYPE_MISMATCH_ASSIGN, currentNode->lineno,
                       NULL);
                freeTableItem(tableItem);
            }
            else
            {
                insertTableItem(symbolTable, tableItem);
            }
            // exp不出意外应该返回一个无用的type，删除
            if (exptype)
                freeType(exptype);
        }
    }
    // Dec -> VarDec
    else
    {
        //处在结构体定义内
        if (structureItem)
        {

            pFieldList feildList = VarDec(child, type);
            pFieldList structField = structureItem->field->type->u.structure.structureField;
            pFieldList last = NULL;
            while (structField != NULL)
            {
                // then we have to check
                if (!strcmp(feildList->name, structField->name))
                {
                    pError(REDEF_FEILD, currentNode->lineno, feildList->name);
                    freeFieldList(feildList);
                    return;
                }
                else
                {
                    last = structField;
                    structField = structField->tail;
                }
            }
            //新建一个fieldlist,删除之前的item
            if (last == NULL)
            {
                // that is good
                structureItem->field->type->u.structure.structureField =
                    copyFieldList(feildList);
            }
            else
            {
                last->tail = copyFieldList(feildList);
            }
            freeFieldList(feildList);
        }
        //在函数的定义语句中
        else
        {
            // 非结构体内，判断返回的item有无冲突，无冲突放入表中，有冲突报错就删除
            pTableItem tableItem = newTableItem(symbolTable->stack->stackDepth, VarDec(child, type));
            if (checkTableItemConflict(symbolTable, tableItem))
            {
                pError(REDEF_VAR, currentNode->lineno, tableItem->field->name);
                freeTableItem(tableItem);
            }
            else
            {
                insertTableItem(symbolTable, tableItem);
            }
        }
    }
}

pFieldList VarDec(pNode currentNode, pType type)
{
    /*
    VarDec:             ID
        |               VarDec LB INT RB
    */
    assert(currentNode != NULL);
    pNode child = currentNode->child;
    // VarDec -> ID
    if (!strcmp(child->name, "ID"))
    {
        return newFieldList(child->value, copyType(type));
    }
    // VarDec -> VarDec LB INT RB
    else
    {
        //需要注意的是ID在里面
        pType temp = type;
        while (child->child)
        {
            temp = newType(ARRAY, copyType(temp), atoi(child->brother->brother->value));
            child = child->child;
        }
        return newFieldList(child->value, temp);
    }
}

void ExtDecList(pNode currentNode, pType type)
{
    /*
    ExtDecList:         VarDec
        |               VarDec COMMA ExtDecList
    */
    assert(currentNode != NULL);
    pNode child = currentNode->child;
    while (child)
    {
        pFieldList fieldList = VarDec(child, type);
        pTableItem tableItem = newTableItem(symbolTable->stack->stackDepth, fieldList);
        if (checkTableItemConflict(symbolTable, tableItem))
        {
            pError(REDEF_VAR, child->lineno, tableItem->field->name);
            freeTableItem(tableItem);
        }
        else
        {
            insertTableItem(symbolTable, tableItem);
        }
        if (child->brother)
            child = child->brother->brother->child;
        else
            child = NULL;
    }
}

void FunDec(pNode currentNode, pType type)
{
    /*
    FunDec:             ID LP VarList RP
        |               ID LP RP
    */
    assert(currentNode != NULL);
    pNode child = currentNode->child;
    pTableItem tableItem = newTableItem(symbolTable->stack->stackDepth, newFieldList(child->value,
                                                                                     newType(FUNCTION, 0, NULL, copyType(type))));
    if (!strcmp(child->brother->brother->name, "VarList"))
    {
        unsigned argc = 0;
        tableItem->field->type->u.function.argv = VarList(child->brother->brother, &argc);
        tableItem->field->type->u.function.argc = argc;
    }
    //是声明语句
    if (!strcmp(currentNode->brother->name, "SEMI"))
    {
        pTableItem temp = funcDeckStack->item;
        while (temp)
        {
            //检查是否有相同的函数声明
            if (!strcmp(temp->field->name, tableItem->field->name))
            {
                break;
            }
            temp = temp->nextSymbol;
        }
        if (temp)
        {
            //有相同的函数声明,检查它的参数是否一致,如果不一致就报错
            //使用双指针查找一致性
            pFieldList p1 = temp->field;
            pFieldList p2 = tableItem->field;
            // 首先检查返回值
            if(!checkType(p1->type->u.function.returnType,p2->type->u.function.returnType)){
                pError(DCLARE_FUNC_INCONSISTENT,currentNode->lineno,temp->field->name);
                freeTableItem(tableItem);
                tableItem = NULL;
                return;
            }
            // 接着比较参数
            p1 = p1->type->u.function.argv;
            p2 = p2->type->u.function.argv;
            while(p1 && p2){
                //名字不相同或者类型不相同就说明有问题
                if(!checkType(p1->type,p2->type)||strcmp(p1->name,p2->name)){
                    pError(DCLARE_FUNC_INCONSISTENT,currentNode->lineno,temp->field->name);
                    break;
                }
                p1 = p1->tail;
                p2 = p2->tail;
            }
            //参数数量不一致
            if(p1 && !p2){
                pError(DCLARE_FUNC_INCONSISTENT,currentNode->lineno,temp->field->name);
            }else if(!p1 && p2){
                pError(DCLARE_FUNC_INCONSISTENT,currentNode->lineno,temp->field->name);
            }
            freeTableItem(tableItem);
            tableItem = NULL;
            return;
        }
        else
        {
            STACK_INC_DEPTH(funcDeckStack);
            tableItem->nextSymbol = funcDeckStack->item;
            funcDeckStack->item = tableItem;
        }
    }
    //函数定义语句
    else if (checkTableItemConflict(symbolTable, tableItem))
    {
        pError(REDEF_FUNC, currentNode->lineno, tableItem->field->name);
        freeTableItem(tableItem);
        tableItem = NULL;
    }
    else
    {
        pTableItem temp = funcDeckStack->item;
        while (temp)
        {
            //检查是否有相同的函数声明
            if (!strcmp(temp->field->name, tableItem->field->name))
            {
                break;
            }
            temp = temp->nextSymbol;
        }
        if (temp)
        {
            //有相同的函数声明,检查它的参数是否一致,如果不一致就报错
            //使用双指针查找一致性
            pFieldList p1 = temp->field;
            pFieldList p2 = tableItem->field;
            // 首先检查返回值
            if(!checkType(p1->type->u.function.returnType,p2->type->u.function.returnType)){
                pError(DCLARE_FUNC_INCONSISTENT,currentNode->lineno,temp->field->name);
                freeTableItem(tableItem);
                tableItem = NULL;
                return;
            }
            
            // 接着比较参数
            p1 = p1->type->u.function.argv;
            p2 = p2->type->u.function.argv;
            while(p1 && p2){
                //名字不相同或者类型不相同就说明有问题
                if(!checkType(p1->type,p2->type)||strcmp(p1->name,p2->name)){
                    pError(DCLARE_FUNC_INCONSISTENT,currentNode->lineno,temp->field->name);
                    break;
                }
                p1 = p1->tail;
                p2 = p2->tail;
                
            }
            //参数数量不一致
            if(p1 && !p2){
                pError(DCLARE_FUNC_INCONSISTENT,currentNode->lineno,temp->field->name);
            }else if(!p1 && p2){
                pError(DCLARE_FUNC_INCONSISTENT,currentNode->lineno,temp->field->name);
            }
        }
        //不管是不是不一致，定义都加入符号表。
        insertTableItem(symbolTable, tableItem);
    }
}

pFieldList VarList(pNode currentNode, int *argc)
{
    /*
    VarList:            ParamDec COMMA VarList
        |               ParamDec
    */
    assert(currentNode != NULL);
    STACK_INC_DEPTH(symbolTable->stack);
    pFieldList head = NULL, tail = NULL;
    pNode child = currentNode->child;
    while (child)
    {
        if (head)
        {
            tail->tail = copyFieldList(ParamDec(child));
            if (tail->tail)
            {
                tail = tail->tail;
                (*argc)++; //后面的参数有可能重复定义，只有当这个参数不是重复定义的时候才加一
            }
        }
        else
        {
            head = copyFieldList(ParamDec(child));
            tail = head;
            (*argc)++; //第一个参数肯定不会重复定义，直接加一
        }
        if (child->brother)
            child = child->brother->brother->child;
        else
            child = NULL;
    }
    STACK_DEC_DEPTH(symbolTable->stack);
    return head;
}

pFieldList ParamDec(pNode currentNode)
{
    /*
    ParamDec:           Specifier VarDec
    */
    assert(currentNode != NULL);
    pNode child = currentNode->child;
    pType type = Specifier(child);
    pTableItem tableItem = newTableItem(symbolTable->stack->stackDepth, VarDec(child->brother, type));

    if (type)
        freeType(type);
    // 重复定义
    if (checkTableItemConflict(symbolTable, tableItem))
    {
        pError(REDEF_VAR, currentNode->lineno, tableItem->field->name);
        freeTableItem(tableItem);
        return NULL;
    }
    else
    {
        insertTableItem(symbolTable, tableItem);
        return tableItem->field;
    }
}

/**
 * @brief 这是一个语句块
 *
 * @param currentNode
 * @param returnType 如果是函数的语句块就是函数的返回类型，否则为空
 */
void CompSt(pNode currentNode, pType returnType)
{
    /*
    CompSt:             LC DefList StmtList RC
     */
    assert(currentNode != NULL);
    //局部变量了，所以加一层
    STACK_INC_DEPTH(symbolTable->stack);
    pNode child = currentNode->child;
    if (!strcmp(child->brother->name, "DefList"))
    {
        DefList(child->brother, NULL);
        child = child->brother; //这条语句不是没有用的，因为DefList和StmtList可能为空
    }
    if (!strcmp(child->brother->name, "StmtList"))
    {
        StmtList(child->brother, returnType);
    }
    clearHeadLayerStack(symbolTable);
    STACK_DEC_DEPTH(symbolTable->stack);
}

void StmtList(pNode currentNode, pType returnType)
{
    /*
    StmtList:           Stmt StmtList
        |         e
    */
    while (currentNode)
    {
        Stmt(currentNode->child, returnType);
        currentNode = currentNode->child->brother;
    }
}

void Stmt(pNode currentNode, pType returnType)
{
    /*
    Stmt:               Exp SEMI
        |               CompSt
        |               RETURN Exp SEMI
        |               IF LP Exp RP Stmt %prec LOWER_THAN_ELSE
        |               IF LP Exp RP Stmt ELSE Stmt
        |               WHILE LP Exp RP Stmt
    */
    pType expType = NULL;
    pNode child = currentNode->child;
    // Stmt -> Exp SEMI
    if (!strcmp(child->name, "Exp"))
    {
        expType = Exp(child);
    }
    // Stmt -> CompSt
    else if (!strcmp(child->name, "CompSt"))
        CompSt(child, returnType);

    // Stmt -> RETURN Exp SEMI
    else if (!strcmp(child->name, "RETURN"))
    {
        expType = Exp(child->brother);

        // check return type
        if (!checkType(returnType, expType))
            pError(TYPE_MISMATCH_RETURN, currentNode->lineno, NULL);
    }

    // Stmt -> IF LP Exp RP Stmt 因为语义分析只是判断类型，不关注执行逻辑
    //，所以接下来就是检查各个表达式是否合适
    else if (!strcmp(child->name, "IF"))
    {
        pNode stmt = child->brother->brother->brother->brother;
        expType = Exp(child->brother->brother);
        Stmt(stmt, returnType);
        // Stmt -> IF LP Exp RP Stmt ELSE Stmt
        if (stmt->brother != NULL)
            Stmt(stmt->brother->brother, returnType);
    }

    // Stmt -> WHILE LP Exp RP Stmt
    else if (!strcmp(child->name, "WHILE"))
    {
        expType = Exp(child->brother->brother);
        Stmt(child->brother->brother->brother->brother, returnType);
    }

    //释放无用类型，因为已经判断完了
    if (expType)
        freeType(expType);
}

/**
 * @brief 困了
 *
 * @param currentNode
 * @return pType
 */
pType Exp(pNode currentNode)
{
    assert(currentNode != NULL);
    /*  Exp -> Exp ASSIGNOP Exp
            | Exp AND Exp
            | Exp OR Exp
            | Exp RELOP Exp
            | Exp PLUS Exp
            | Exp MINUS Exp
            | Exp STAR Exp
            | Exp DIV Exp
            | LP Exp RP
            | MINUS Exp
            | NOT Exp
            | ID LP Args RP
            | ID LP RP
            | Exp LB Exp RB
            | Exp DOT ID
            | ID
            | INT
            | FLOAT
    */
    pNode child = currentNode->child;
    /*
    Exp -> Exp ASSIGNOP Exp
        | Exp AND Exp
        | Exp OR Exp
        | Exp RELOP Exp
        | Exp PLUS Exp
        | Exp MINUS Exp
        | Exp STAR Exp
        | Exp DIV Exp
        | Exp LB Exp RB
        | Exp DOT ID
    */
    if (!strcmp(child->name, "Exp"))
    {
        // 基本数学运算符
        /*
        Exp -> Exp ASSIGNOP Exp
            | Exp AND Exp
            | Exp OR Exp
            | Exp RELOP Exp
            | Exp PLUS Exp
            | Exp MINUS Exp
            | Exp STAR Exp
            | Exp DIV Exp
        */
        if (strcmp(child->brother->name, "LB") && strcmp(child->brother->name, "DOT"))
        {
            pType p1 = Exp(child);                   //左边的类型
            pType p2 = Exp(child->brother->brother); //右边的类型
            pType returnType = NULL;

            // Exp -> Exp ASSIGNOP Exp
            if (!strcmp(child->brother->name, "ASSIGNOP"))
            {
                //检查左值
                pNode lchild = child->child;

                if (!strcmp(lchild->name, "FLOAT") ||
                    !strcmp(lchild->name, "INT"))
                {
                    //报错，左值
                    pError(LEFT_VAR_ASSIGN, child->lineno, NULL);
                }
                else if (!strcmp(lchild->name, "ID") ||
                         !strcmp(lchild->brother->name, "LB") ||
                         !strcmp(lchild->brother->name, "DOT"))
                {
                    if (!checkType(p1, p2))
                    {
                        //报错，类型不匹配
                        pError(TYPE_MISMATCH_ASSIGN, child->lineno, NULL);
                    }
                    else
                        returnType = copyType(p1);
                }
                else
                {
                    //报错，左值
                    pError(LEFT_VAR_ASSIGN, child->lineno, NULL);
                }
            }
            // Exp -> Exp AND Exp
            //      | Exp OR Exp
            //      | Exp RELOP Exp
            //      | Exp PLUS Exp
            //      | Exp MINUS Exp
            //      | Exp STAR Exp
            //      | Exp DIV Exp
            else
            {
                if (p1 && p2 && (p1->kind == ARRAY || p2->kind == ARRAY))
                {
                    //报错，数组，结构体运算
                    pError(TYPE_MISMATCH_OP, child->lineno, NULL);
                }
                else if (!checkType(p1, p2))
                {
                    //报错，类型不匹配
                    pError(TYPE_MISMATCH_OP, child->lineno, NULL);
                }
                else
                {
                    if (p1 && p2)
                    {
                        returnType = copyType(p1);
                    }
                }
            }

            if (p1)
                freeType(p1);
            if (p2)
                freeType(p2);
            return returnType;
        }
        // 数组和结构体访问
        else
        {
            // Exp -> Exp LB Exp RB
            if (!strcmp(child->brother->name, "LB"))
            {
                //数组
                pType p1 = Exp(child);
                pType p2 = Exp(child->brother->brother);
                pType returnType = NULL;

                if (!p1)
                {
                    // 第一个exp为null，上层报错，这里不用再管
                }
                else if (p1 && p1->kind != ARRAY)
                {
                    //报错，非数组使用[]运算符
                    pError(NOT_A_ARRAY, child->lineno, child->child->value);
                }
                else if (!p2 || p2->kind != BASIC ||
                         p2->u.basic != INT_TYPE_)
                {
                    //报错，不用int索引[]
                    pError(NOT_A_INT, child->lineno, child->brother->brother->child->value);
                }
                else
                {
                    returnType = copyType(p1->u.array.elem);
                }
                if (p1)
                    freeType(p1);
                if (p2)
                    freeType(p2);
                return returnType;
            }
            // Exp -> Exp DOT ID
            else
            {
                pType p1 = Exp(child);
                pType returnType = NULL;
                if (!p1 || p1->kind != STRUCTURE ||
                    !p1->u.structure.name)
                {
                    //报错，对非结构体使用.运算符
                    pError(ILLEGAL_USE_DOT, child->lineno, NULL);
                    if (p1)
                    {
                        freeType(p1);
                        p1 = NULL;
                    }
                }
                else
                {
                    pNode ref_id = child->brother->brother;
                    pFieldList structfield = p1->u.structure.structureField;
                    while (structfield != NULL)
                    {
                        if (!strcmp(structfield->name, ref_id->value))
                        {
                            break;
                        }
                        structfield = structfield->tail;
                    }
                    if (structfield == NULL)
                    {
                        //报错，没有可以匹配的域名
                        pError(NONEXISTFIELD, currentNode->lineno, ref_id->value);
                    }
                    else
                    {
                        returnType = copyType(structfield->type);
                    }
                }
                if (p1)
                {
                    freeType(p1);
                }

                return returnType;
            }
        }
    }
    //单目运算符
    // Exp -> MINUS Exp
    //      | NOT Exp
    else if (!strcmp(child->name, "MINUS") || !strcmp(child->name, "NOT"))
    {
        pType p1 = Exp(child->brother);
        pType returnType = NULL;
        if (!p1 || p1->kind != BASIC)
        {
            //报错，数组，结构体运算
            pError(TYPE_MISMATCH_OP, currentNode->lineno, NULL);
        }
        else
        {
            returnType = copyType(p1);
        }
        if (p1)
            freeType(p1);
        return returnType;
    }
    else if (!strcmp(child->name, "LP"))
    {
        return Exp(child->brother);
    }
    // Exp -> ID LP Args RP
    //		| ID LP RP
    else if (!strcmp(child->name, "ID") && child->brother)
    {
        pTableItem funcInfo = getSymbolTableItem(symbolTable, child->value);

        // function not find
        if (funcInfo == NULL)
        {
            pError(UNDEF_FUNC, currentNode->lineno, child->value);
            return NULL;
        }
        else if (funcInfo->field->type->kind != FUNCTION)
        {
            pError(NOT_A_FUNC, currentNode->lineno, child->value);
            return NULL;
        }
        // Exp -> ID LP Args RP
        else if (!strcmp(child->brother->brother->name, "Args"))
        {
            Args(child->brother->brother, funcInfo);
            return copyType(funcInfo->field->type->u.function.returnType);
        }
        // Exp -> ID LP RP
        else
        {
            if (funcInfo->field->type->u.function.argc != 0)
            {
                pError(FUNC_AGRC_MISMATCH, currentNode->lineno, funcInfo->field->name);
            }
            return copyType(funcInfo->field->type->u.function.returnType);
        }
    }
    // Exp -> ID
    else if (!strcmp(child->name, "ID"))
    {
        pTableItem tp = getSymbolTableItem(symbolTable, child->value);
        if (tp == NULL || isStructDef(tp))
        {
            pError(UNDEF_VAR, child->lineno, child->value);
            return NULL;
        }
        else
        {
            // good
            return copyType(tp->field->type);
        }
    }
    else
    {
        // Exp -> FLOAT
        if (!strcmp(child->name, "FLOAT"))
        {
            return newType(BASIC, FLOAT_TYPE_);
        }
        // Exp -> INT
        else
        {
            return newType(BASIC, INT_TYPE_);
        }
    }
}

void Args(pNode currentNode, pTableItem funcInfo)
{
    assert(currentNode != NULL);
    // Args -> Exp COMMA Args
    //       | Exp
    // printTreeInfo(currentNode, 0);
    pNode temp = currentNode;
    pFieldList arg = funcInfo->field->type->u.function.argv;
    // printf("-----function atgs-------\n");
    // printFieldList(arg);
    // printf("---------end-------------\n");
    while (temp)
    {
        if (arg == NULL)
        {
            pError(FUNC_AGRC_MISMATCH, currentNode->lineno, funcInfo->field->name);
            break;
        }
        pType realType = Exp(temp->child);
        // printf("=======arg type=========\n");
        // printType(realType);
        // printf("===========end==========\n");
        if (!checkType(realType, arg->type))
        {
            pError(FUNC_AGRC_MISMATCH, currentNode->lineno, funcInfo->field->name);
            if (realType)
                freeType(realType);
            return;
        }
        if (realType)
            freeType(realType);

        arg = arg->tail;
        if (temp->child->brother)
        {
            temp = temp->child->brother->brother;
        }
        else
        {
            break;
        }
    }
    if (arg != NULL)
    {
        pError(FUNC_AGRC_MISMATCH, currentNode->lineno, funcInfo->field->name);
    }
}
