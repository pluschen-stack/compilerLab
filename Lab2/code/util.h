#ifndef UTIL_H
#define UTIL_H

#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

int convertHexToDec(const char *hexStr);
int convertOctToDec(const char *octStr);
char* newString(char* src);
#endif