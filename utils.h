#include <stdio.h>
#include <stdlib.h>

#define MAX_CLIENTS 100
#define BUFFER_SZ 2048
#define NAME_LEN 40

/* Client Structure */
typedef struct{
    struct sockaddr_in address;
    int sockfd;
    int uid;
    char name[NAME_LEN];
} client_t;

void str_overwrite_stdout();
void str_trim_lf(char *arr, int len);
