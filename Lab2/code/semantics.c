#include "semantics.h"
#include "util.h"

pSymbolTable symbolTable;

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
    default:
        perror("error kind");
        exit(1);
        break;
    }
    va_end(vaList);
    return type;
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
void insertTableItem(pTableItem newTableItem){
    assert(newTableItem != NULL);
    unsigned int hashCode= hash_pjw(newTableItem->field->name);
    pHashTable hashTable = symbolTable->hashTable;
    pStack stack = symbolTable->stack;

    newTableItem -> nextHash = GET_HASH_HEAD(hashTable,hashCode);
    SET_HASH_HEAD(hashTable,hashCode,newTableItem);

    newTableItem -> nextSymbol = GET_STACK_HEAD(stack);
    SET_STACK_HEAD(stack,newTableItem);
}

/**
 * @brief 按照name在hash表中查找最近的tableItem
 * 
 * @param hashTable hash表 
 * @param name 项名
 * @return pTableItem 如果存在则返回，否则返回NULL
 */
pTableItem getTableItem(pHashTable hashTable,char * name){
    pTableItem result = NULL;
    unsigned int hashCode = hash_pjw(name);
    pTableItem temp = GET_HASH_HEAD(hashTable,hashCode);
    while(temp){
        if(!strcmp(temp->field->name,name)){
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

void freeFieldList(pFieldList feildList)
{
    assert(feildList != NULL);
    pFieldList temp = feildList;
    FREE(feildList->name);
    freeType(feildList->type);
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
    return symbolTable;
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
    */
    assert(currentNode != NULL);
    pNode secondChild = currentNode->child->brother;
    pType type = Specifier(currentNode->child);
    if (!strcmp(secondChild->name, "ExtDecList"))
    {
        // ExtDecList();
    }
    else if (!strcmp(secondChild->name, "FunDec"))
    {
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

pType StructSpecifier(pNode currentNode)
{
    /*
    StructSpecifier:    STRUCT OptTag LC DefList RC
        |               STRUCT Tag

    OptTag -> ID | e 在语法分析中如果OptTag为空就不会建立节点
    Tag -> ID
    */

    assert(currentNode != NULL);
    pType resultType;
    assert(currentNode->child != NULL);
    pNode child = currentNode->child->brother;

    if (!strcmp(child->name, "Tag"))
    {
        
    }
    // OptTag -> ID
    else if (!strcmp(child->name, "OptTag"))
    {
        pTableItem structureItem =
            newTableItem(symbolTable->stack->stackDepth,
                         newFieldList(NULL, newType(STRUCTURE, NULL, NULL)));
        structureItem->field->name = newString(child->child->value);
        insertTableItem(structureItem);
        print(getTableItem(symbolTable->hashTable,structureItem->field->name)->field->name);
        freeTableItem(structureItem);
    }
    return newType(STRUCTURE);
}

// void ExtDecList()
// {
// }