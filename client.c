#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include "utils.h"


volatile sig_atomic_t flag = 0;
int sockfd = 0;
char name[NAME_LEN];

void catch_sigint_exit() {
    flag = 1;
}

void recv_msg_handler()
{
    char message[BUFFER_SZ] = {};
    for (;;)
    {
        int receive = recv(sockfd, message, BUFFER_SZ, 0);

        if (receive > 0) {
           printf("%s\n", message);
           str_overwrite_stdout();
        } else if(receive == 0){
            break;
        }
        bzero(message, BUFFER_SZ);
    }
}

void send_msg_handler()
{
    char buffer[BUFFER_SZ] = {};
    char message[BUFFER_SZ + NAME_LEN + 2] = {};
    for (;;)
    {
        str_overwrite_stdout();
        fgets(buffer, BUFFER_SZ, stdin);
        str_trim_lf(buffer, BUFFER_SZ);

        if (strcmp(buffer, "exit") == 0) {
           break;
        } else {
            sprintf(message, "%s: %s\n", name, buffer);
            send(sockfd, message, strlen(message), 0);
        }

        bzero(buffer, BUFFER_SZ);
        bzero(message, BUFFER_SZ + NAME_LEN + 2);
    }

    catch_sigint_exit(2);
}

int main(int argc, char *argv[])
{
    char *ip = "127.0.0.1";
    int port;
    int ret;
    struct sockaddr_in server_addr;

    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }
    port = atoi(argv[1]);

    signal(SIGINT, catch_sigint_exit);
    printf("Name?: ");
    fgets(name, NAME_LEN, stdin);
    str_trim_lf(name, strlen(name));

    ret = verify_client_name(_name);
    if (ret) {
        return EXIT_FAILURE;
    }


    /* Socket parameters */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);

    /* Connect to the server */
    ret = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (ret == -1) {
        printf("Error: connect %s\n", strerror(errno));

    }

    /* Send client name */
    send(sockfd, name, NAME_LEN, 0);

    printf("CHATROOM\n");

    pthread_t send_msg_thread;
    ret = pthread_create(&send_msg_thread, NULL, (void*)send_msg_handler, NULL);

    if (ret) {
        printf("ERROR: pthread %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    pthread_t recv_msg_thread;
    ret = pthread_create(&recv_msg_thread, NULL, (void*)recv_msg_handler, NULL);

    if (ret) {
        printf("ERROR: pthread %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    for (;;)
    {
        if (flag) {
            printf("\nExit\n");
            break;
        }
    }

    close(sockfd);

    return EXIT_SUCCESS;
}
