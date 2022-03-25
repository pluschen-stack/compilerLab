#ifndef NODE_H
#define NODE_H

#include <stdint.h>
#include <assert.h>
#include <stdarg.h> //可变参数使用的头文件
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <stdbool.h> //用于bool值

/**
 * @brief 定义节点值的类型，可能是整型，浮点型等
 * 
 */
typedef enum {
    FLOAT_TYPE,//浮点数类型
    INT_TYPE,//整数类型
    TYPE_TYPE,//类型的类型
    ID_TYPE,//标识符类型
    OPERATOR_TYPE,//运算符类型
    PUNCTUATION_TYPE,//标点符号等，包括括号
    KEYWORD_TYPE,//关键字类型
    NON_TERMINAL//非终结符，也就是语法单元
}nodeType;


typedef struct node{
    uint32_t lineno;//行号
    nodeType type;//类型

    char * name;//名字
    char * value;//值

    struct node* child;//儿子节点
    struct node* brother;//兄弟节点
}Node;

typedef Node* pNode;

pNode newTokenNode(uint32_t lineno,nodeType type,
    const char *name,const char *value);
pNode newNode(uint32_t lineno,nodeType type,
    const char *name,int argc,...);
void printSyntaxTree(pNode currentNode,int height);

#endif