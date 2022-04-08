#ifndef SEMANTICS_H
#define SEMANTICS_H

#include"node.h"

#define SYMBOL_TABLE_SIZE 0X3FFF
#define FREE(p) if(p){free(p);p = NULL;} //安全的释放动态内存

#define GET_STACK_HEAD(s)  s?s->stackArray[s->stackDepth]:NULL
#define SET_STACK_HEAD(s,i)  if(s){s->stackArray[s->stackDepth]=i;}
#define GET_HASH_HEAD(h,c) h?h->hashArray[c]:NULL
#define SET_HASH_HEAD(h,c,i) if(h){h->hashArray[c]=i;}
#define SET_FEILDLIST_NAME(f,n) if(f->name){FREE(f->name);f->name = newString(n);}else{f->name=newString(n);}

#ifdef DEBUGON
#define print(s) fprintf(stdout, "%d %s\n", __LINE__,s);
#else
#define print(s) 
#endif


typedef struct Type_ *pType;
typedef struct FieldList_ *pFieldList;
typedef struct HashTable_ *pHashTable;
typedef struct TableItem_ *pTableItem;
typedef struct Stack_ *pStack;
typedef struct SymbolTable_ *pSymbolTable;
typedef enum kind_ { BASIC, ARRAY, STRUCTURE } Kind;
typedef enum {
            INT_TYPE_,//和之前的定义冲突了，因此这里小小的修改一下
            FLOAT_TYPE_//和之前的定义冲突了，因此这里小小的修改一下
        } BasicType;
struct Type_
{
    Kind kind;
    union
    {
        BasicType basic;
        //  数组类型信息包括元素类型大小构成
        struct
        {
            pType elem;
            int size;
        } array;
        //  结构体类型信息是一个链表
        struct{
            char *name;
            pFieldList structureField;
        } structure;
        /*
        
        */
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
    unsigned unamedStructNum;//未被命名的结构体
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
pTableItem newTableItem(int depth,pFieldList feildList);
void freeTableItem(pTableItem tableItem);
bool isStructDef(pTableItem tableItem);

pType newType(Kind kind, ...);
pType copyType(pType srcType);
void printType(pType type);

pFieldList newFieldList(char *name, pType type);
void printFieldList(pFieldList fieldList);
void freeFieldList(pFieldList feildList);
pFieldList copyFieldList(pFieldList srcFeildList);


void startSemanticAnalysis(pNode currentNode);
void ExtDef(pNode currentNode);
pType Specifier(pNode currentNode);
pType StructSpecifier(pNode currentNode);
void DefList(pNode currentNode, pTableItem structureItem);
void Def(pNode currentNode, pTableItem structureItem);
void DecList(pNode currentNode, pType type, pTableItem structureItem);
void Dec(pNode currentNode, pType type, pTableItem structureItem);
pFieldList VarDec(pNode currentNode, pType type);
#endif