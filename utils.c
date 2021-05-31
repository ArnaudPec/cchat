#include <stdio.h>

void str_overwrite_stdout() {
    printf("\r%s", "> ");
    fflush(stdout);
}

void str_trim_lf(char *arr, int len)
{
   for (int i = 0; i < len; ++i) {
       if (arr[i] == '\n') {
           arr[i] = '\0';
           break;
       }
   }
}

