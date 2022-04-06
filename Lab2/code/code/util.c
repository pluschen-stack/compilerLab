#include "util.h"
#include <stdio.h>

/**
 * @brief 将一个十六进制字符串转化为十进制整型数值
 * 
 * @param hexStr 十六进制的字符串
 * @return int 如果是十六进制字符串则返回正确的值，否则程序应该出错，这里感觉可以使用setjmp等函数来处理异常，但是我不会。
 */
int convertHexToDec(const char *hexStr)
{
    size_t length = strlen(hexStr);
    //先判断是不是十六进制数
    assert(!strncmp(hexStr, "0x", 2) || !strncmp(hexStr, "0X", 2) &&length >= 3);
    int decimal = 0,base = 1;
    int i = 0;
    for (i = --length; i >= 2; i--)
    {
        if (hexStr[i] >= '0' && hexStr[i] <= '9')
        {
            decimal += (hexStr[i] - 48) * base;
            base *= 16;
        }
        else if (hexStr[i] >= 'A' && hexStr[i] <= 'F')
        {
            decimal += (hexStr[i] - 55) * base;
            base *= 16;
        }
        else if (hexStr[i] >= 'a' && hexStr[i] <= 'f')
        {
            decimal += (hexStr[i] - 87) * base;
            base *= 16;
        }else{
            fprintf(stderr,"error hex input\n");
            assert(false);
        }
    }
    return decimal;
}

/**
 * @brief 将一个八进制字符串转化为十进制整型数值
 * 
 * @param octStr 八进制的字符串
 * @return int 如果是八进制字符串则返回正确的值，否则程序应该出错
 */
int convertOctToDec(const char *octStr){
    size_t length = strlen(octStr);
    int decimal = 0,base = 1;
    if(length == 1 && !strncmp(octStr,"0",1)){
        return 0;
    }else if(!strncmp(octStr,"0",1) && length >= 2){
        int i = 0;
        for (i = --length; i >= 1; i--)
        {
            if (octStr[i] >= '0' && octStr[i] <= '7')
            {
                decimal += (octStr[i] - 48) * base;
                base *= 8;
            }else{
                fprintf(stderr,"error oct input\n");
                assert(false);
            }
        }
    }
    return decimal;
}

int testintuilc(){
    // printf("%d\n",convertHexToDec("0x12"));
    // printf("%d\n",convertHexToDec("0X23"));
    // printf("%d\n",convertHexToDec("023"));
    // printf("%d\n",convertHexToDec("0x"));

    printf("%d\n",convertOctToDec("023"));
    printf("%d\n",convertOctToDec("03"));
    printf("%d\n",convertOctToDec("0"));
}