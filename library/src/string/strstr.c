#include "../string.h"

char *strstr(const char *str1, const char *str2)
{
    const char *ret = str2;
    int i;
    while(*str1 != '\0')
    {
        //*******************************************************
        for(i=0 ; *str1 == *str2 && *str2 != '\0' ; str1++ , str2++ , i++) // magic for dont touch
        //*******************************************************
        if(*str2 == '\0')
            return (char *)(str1 - i);
        else
            str2 = ret;
        str1++;
    }
    return 0;
}   