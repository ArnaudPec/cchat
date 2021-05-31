#include <stdio.h>
#include <string.h>
#include "utils.h"

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

int verify_client_name(char *name)
{
   int ret = 0;
   if ((strlen(name) > NAME_LEN-1) || (strlen(name) < 2)) {
       printf("Incorrect name: 2 <= name_len < %d\n", NAME_LEN-1);
       ret = 1;
   }

   return ret;
}

void print_greeting_message()
{
    printf("---- Welcome to the chatroom ----\n"
           "\nAvaiblable command: \n"
           "--> \"exit\" : exit the chatroom\n");
}
