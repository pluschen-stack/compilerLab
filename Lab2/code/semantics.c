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

void printType(pType type) {
    if (type == NULL) {
        printf("type is NULL.\n");
    } else {
        printf("type kind: %d\n", type->kind);
        switch (type->kind) {
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
                else {
                    printf("struct name is %s\n", type->u.structure.name);
                }
                printFieldList(type->u.structure.structureField);
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
pType copyType(pType srcType){
    if (srcType == NULL) return NULL;
    pType p = (pType)malloc(sizeof(struct Type_));
    assert(p != NULL);
    p->kind = srcType->kind;
    switch (p->kind) {
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
        default:
            fprintf(stderr,"error kind in %s",__FUNCTION__);
            exit(1);
            break;
    }
    return p;
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
void insertTableItem(pSymbolTable symbolTable,pTableItem newTableItem)
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
void printFieldList(pFieldList fieldList) {
    if (fieldList == NULL)
        printf("fieldList is NULL\n");
    else {
        printf("fieldList name is: %s\n", fieldList->name);
        printf("FieldList Type:\n");
        printType(fieldList->type);
        printFieldList(fieldList->tail);
    }
}

pFieldList copyFieldList(pFieldList srcFeildList){
    pFieldList head = NULL, cur = NULL;
    pFieldList temp = srcFeildList;

    while (temp) {
        if (!head) {
            head = newFieldList(temp->name, copyType(temp->type));
            cur = head;
            temp = temp->tail;
        } else {
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
    if(feildList->type) freeType(feildList->type);
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

bool checkTableItemConflict(pSymbolTable table, pTableItem item) {
    pTableItem temp = getSymbolTableItem(table, item->field->name);
    if (temp == NULL) return false;
    while (temp) {
        if (!strcmp(temp->field->name, item->field->name)) {
            //因为结构体只能在函数外定义，所以直接出错
            if (temp->field->type->kind == STRUCTURE ||
                item->field->type->kind == STRUCTURE)
                return true;
            //其他非结构体就需要比较它们的深度才行
            if (temp->symbolDepth == table->stack->stackDepth) return true;
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

bool isStructDef(pTableItem tableItem){
    if (tableItem == NULL) return false;
    if (tableItem->field->type->kind != STRUCTURE) return false;
    if (tableItem->field->type->u.structure.name) return false;
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
        
        if (structureItem == NULL || !isStructDef(structureItem)) {
            pError(UNDEF_STRUCT, currentNode->lineno, child->child->value);
        } else
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
        if (checkTableItemConflict(symbolTable, structureItem)) {
            pError(DUPLICATED_NAME, currentNode->lineno, structureItem->field->name);
            freeTableItem(structureItem);
        }
        //不存在相同结构体定义 
        else {
            returnType = newType(
                STRUCTURE, newString(structureItem->field->name),
                copyFieldList(structureItem->field->type->u.structure.structureField));
            if (!strcmp(currentNode->child->brother->name, "OptTag")) {
                insertTableItem(symbolTable, structureItem);
            }
            // OptTag -> e
            else {
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
    assert(currentNode!=NULL);
    pNode child = currentNode->child;
    // Dec -> VarDec ASSIGNOP Exp
    if(child->brother){
        // //处在结构体定义内
        // if(!structureItem){
        //     VarDec(currentNode,type);
        // }
        // //在函数的定义语句中
        // else{

        // }
    }
    // Dec -> VarDec
    else{
        //处在结构体定义内
        if(structureItem){

            pFieldList feildList = VarDec(child,type);
            pFieldList structField = structureItem->field->type->u.structure.structureField;
            pFieldList last = NULL;
            while (structField != NULL) {
                // then we have to check
                if (!strcmp(feildList->name, structField->name)) {
                    pError(REDEF_FEILD,currentNode->lineno,feildList->name);
                    freeFieldList(feildList);
                    return;
                } else {
                    last = structField;
                    structField = structField->tail;
                }
            }
            //新建一个fieldlist,删除之前的item
            if (last == NULL) {
                // that is good
                structureItem->field->type->u.structure.structureField =
                    copyFieldList(feildList);
            } else {
                last->tail = copyFieldList(feildList);
            }
            freeFieldList(feildList);
        }
        //在函数的定义语句中
        else{

        }
    }

}

pFieldList VarDec(pNode currentNode, pType type){
    /*
    VarDec:             ID 
        |               VarDec LB INT RB
    */
    assert(currentNode != NULL);
    pNode child = currentNode->child;
    // VarDec -> ID
    if(!strcmp(child->name,"ID")){
        return newFieldList(child->value,copyType(type));
    }
    // VarDec -> VarDec LB INT RB
    else{
        //需要注意的是ID在里面
        pType temp = type;
        while (child->child)
        {
            temp = newType(ARRAY,copyType(temp),atoi(child->brother->brother->value));
            child = child->child;
        }
        return newFieldList(child->value,temp);
    }
}