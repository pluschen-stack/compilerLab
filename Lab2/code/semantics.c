#include"semantics.h"




/**
 * @brief 来自P.J.Weinberger提供的hash函数
 * 
 * @param name 符号名
 * @return unsigned int hash值 
 */
inline unsigned int hash_pjw(char* name){
    unsigned int val = 0,i;
    for (; *name; ++name)
    {
        val = (val << 2) + *name;
        if( i = val & ~SYMBOL_TABLE_SIZE){
            val = (val ^ (i >> 12)) & SYMBOL_TABLE_SIZE;
        }
    }
    return val;
    
}

/**
 * @brief 用于打印错误信息
 * 
 * @param type 
 * @param line 
 * @param msg 
 */
inline void pError(ErrorType type, int line, char* msg) {
    printf("Error type %d at Line %d: %s\n", type, line, msg);
}

void freeTableItem(pTableItem tableItem){
    //TODO
}

/**
 * @brief 新建一个hash表
 * 
 * @return pHashTable 
 */
pHashTable newHashTable(){
    pHashTable hashTable = malloc(sizeof(struct HashTable_));
    assert(hashTable != NULL);
    hashTable->hashArray = malloc(sizeof(struct TableItem_)*SYMBOL_TABLE_SIZE);
    assert(hashTable->hashArray != NULL);
    for (int i = 0; i < SYMBOL_TABLE_SIZE; i++) {
        hashTable->hashArray[i] = NULL;
    }
    return hashTable;
}

/**
 * @brief 释放hash表的空间
 * 
 * @param hashTable hash表
 */
void freeHashTable(pHashTable hashTable){
    if(!hashTable){
        for(int i = 0; i < SYMBOL_TABLE_SIZE;i++){
            pTableItem temp = hashTable->hashArray[i];
            while(temp){
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
pStack newStack(){
    pStack stack = malloc(sizeof(struct Stack_));
    assert(stack != NULL);
    stack->stackArray = malloc(sizeof(struct TableItem_)*SYMBOL_TABLE_SIZE);
    assert(stack->stackArray != NULL);
    for (int i = 0; i < SYMBOL_TABLE_SIZE; i++) {
        stack->stackArray[i] = NULL;
    }
    return stack;
}

/**
 * @brief 释放栈空间，需要注意的是由于这个函数需配合freeHashTable使用，并且在其后调用，不然存在内存泄露
 * 
 * @param stack 栈
 */
void freeStack(pStack stack){
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
pSymbolTable initSymbolTable(){
    pSymbolTable symbolTable = malloc(sizeof(struct SymbolTable_));
    assert(symbolTable != NULL);
    symbolTable->hashTable= newHashTable();
    symbolTable->stack = newStack();
    return symbolTable;
}

/**
 * @brief 释放符号表的空间
 * 
 * @param symbolTable 符号表
 */
void freeSymbolTable(pSymbolTable symbolTable){
    freeHashTable(symbolTable->hashTable);
    symbolTable->hashTable = NULL;
    freeStack(symbolTable ->stack);
    symbolTable->stack = NULL;
    FREE(symbolTable);
}

