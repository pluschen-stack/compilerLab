#ifndef SEMANTICS_H
#define SEMANTICS_H



#define SYMBOL_TABLE_SIZE 0X3FFF
#define FREE(p) if(p){free(p);p = NULL;} //安全的释放动态内存

#include"node.h"

typedef struct Type_ *pType;
typedef struct FieldList_ *pFieldList;
typedef struct HashTable_ *pHashTable;
typedef struct TableItem_ *pTableItem;
typedef struct Stack_ *pStack;
typedef struct SymbolTable_ *pSymbolTable;

struct Type_
{
    enum
    {
        BASIC,
        ARRAY,
        STRUCTURE,
    } kind;
    union
    {
        //  基本类型
        enum
        {
            INT_TYPE_,//和之前的定义冲突了，因此这里小小的修改一下
            FLOAT_TYPE_//和之前的定义冲突了，因此这里小小的修改一下
        } basic;
        //  数组类型信息包括元素类型大小构成
        struct
        {
            pType elem;
            int size;
        } array;
        //  结构体类型信息是一个链表
        pFieldList structure;
    } u;
};

struct FieldList_
{
    char *name;      //  域的名字
    pType type;      //  域的类型
    pFieldList tail; //  下一个域
};

/**
 * @brief 符号表中的一个项
 *
 */
struct TableItem_
{
    int symbolDepth;  //  在符号表中的深度，可以方便查看后续有多少个同hash值的item
    pFieldList field;
    //使用十字链表所以需要两个指针，一个指栈中同一层的item，一个指相同hash的item
    pTableItem nextSymbol;  //  相同嵌套深度的符号，竖指针
    pTableItem nextHash;  //  同hash值的item，横指针
};

/**
 * @brief 符号表中的hash表
 *
 */
struct HashTable_
{
    pTableItem *hashArray;
};

/**
 * @brief 符号栈，主要处理在多重作用域下的情况
 *
 */
struct Stack_
{
    int stackDepth;
    pTableItem *stackArray;
};

/**
 * @brief 符号表通过hash表和一个栈构成
 *
 */
struct SymbolTable_
{
    pHashTable hashTable;
    pStack stack;
};

typedef enum _errorType {
    UNDEF_VAR = 1,         // Undefined Variable
    UNDEF_FUNC,            // Undefined Function
    REDEF_VAR,             // Redefined Variable
    REDEF_FUNC,            // Redefined Function
    TYPE_MISMATCH_ASSIGN,  // Type mismatchedfor assignment.
    LEFT_VAR_ASSIGN,  // The left-hand side of an assignment must be a variable.
    TYPE_MISMATCH_OP,      // Type mismatched for operands.
    TYPE_MISMATCH_RETURN,  // Type mismatched for return.
    FUNC_AGRC_MISMATCH,    // Function is not applicable for arguments
    NOT_A_ARRAY,           // Variable is not a Array
    NOT_A_FUNC,            // Variable is not a Function
    NOT_A_INT,             // Variable is not a Integer
    ILLEGAL_USE_DOT,       // Illegal use of "."
    NONEXISTFIELD,         // Non-existentfield
    REDEF_FEILD,           // Redefined field
    DUPLICATED_NAME,       // Duplicated name
    UNDEF_STRUCT           // Undefined structure
} ErrorType;

unsigned int hash_pjw(char *name);
void pError(ErrorType type, int line, char* msg);
pHashTable newHashTable();
void freeHashTable(pHashTable hashTable);
pStack newStack();
void freeStack(pStack stack);
pSymbolTable initSymbolTable();
void freeSymbolTable(pSymbolTable symbolTable);

#endif