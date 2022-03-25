#include "node.h"

/**
 * @brief 建立词法分析时的Token项的值
 * 
 * @param lineno 行号
 * @param type 值类型
 * @param name 词法属性
 * @param value 值
 * @return pNode 建立好的Token项的值
 */
static pNode newTokenNode(uint32_t lineno,nodeType type,
    const char *name,const char *value){

        pNode tokenNode = malloc(sizeof(Node));
        assert(tokenNode != NULL);

        tokenNode->lineno = lineno;
        tokenNode->type = type;

        size_t nameLength = 0;
        size_t valueLength = 0;

        /*因为strlen返回不带字符串的\0，所以加一*/
        if(name != NULL){
            nameLength = strlen(name) + 1;
            tokenNode->name = malloc(nameLength*sizeof(char));
            strncpy(tokenNode->name,name,nameLength);
        }
        
        if(value != NULL){
            valueLength = strlen(value) + 1;
            tokenNode->value = malloc(valueLength*sizeof(char));
            strncpy(tokenNode->value,value,valueLength);
        }else{
            tokenNode->value = NULL;
        }

        //此处并不使用，所以有一点空间浪费
        tokenNode->child = NULL;
        tokenNode->brother = NULL;

        return tokenNode;
}

/**
 * @brief 创建语法节点
 * 
 * @param lineno 行号
 * @param type 值类型
 * @param name 词法属性
 * @param argc 儿子节点数量
 * @param ... 儿子节点
 * @return pNode 建立好的语法节点
 */
static pNode newNode(uint32_t lineno,nodeType type,
    const char *name,int argc,...){
        /*此时是语法节点，不再需要节点的值了*/
        pNode currentNode = newTokenNode(lineno,type,name,"");

        va_list vaList;
        va_start(vaList, argc);

        pNode tempNode = va_arg(vaList, pNode);

        currentNode->child = tempNode;

        for (int i = 1; i < argc; i++) {
            tempNode->brother = va_arg(vaList, pNode);
            if (tempNode->brother != NULL) {
                tempNode = tempNode->brother;
            }
        }

        va_end(vaList);
        return currentNode;
}

/**
 * @brief 按照先根遍历打印语法树
 * 
 * @param currentNode 语法节点
 * @param height 树深度
 */
static void printSyntaxTree(pNode currentNode,int height){
    if(currentNode != NULL){
        /*能够比较清晰的展示深度*/
        
        if(currentNode->type == NON_TERMINAL && currentNode->child != NULL){
            for (int i = 0; i < height; i++) {
                printf("  ");
            }
            /*按要求打印第一个儿子的行号*/
            fprintf(stdout,"%s (%d)\n",currentNode->name,currentNode->child->lineno);
        }else if(currentNode->type == ID_TYPE){
            for (int i = 0; i < height; i++) {
                printf("  ");
            }
            fprintf(stdout,"%s: %s\n",currentNode->name,currentNode->value);
        }else if(currentNode->type == INT_TYPE){
            for (int i = 0; i < height; i++) {
                printf("  ");
            }
            fprintf(stdout,"%s: %d\n",currentNode->name,atoi(currentNode->value));
        }else if(currentNode->type == FLOAT_TYPE){
            for (int i = 0; i < height; i++) {
                printf("  ");
            }
            fprintf(stdout,"%s: %f\n",currentNode->name,atof(currentNode->value));
        }
        printSyntaxTree(currentNode->child,height+1);
        printSyntaxTree(currentNode->brother,height);
    }else{
        return;
    }
}