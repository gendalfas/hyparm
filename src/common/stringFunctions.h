#ifndef __COMMON__STRING_FUNCTIONS_H__
#define __COMMON__STRING_FUNCTIONS_H__

#include "common/types.h"


int stringncmp(char * str1, char * str2, int n);
int strcmp(const char *s1, const char *s2);
u32int strtoi(char * str);
u32int strlen(const char *s);
char * stringcpy(char * dest, char *src);

#endif
