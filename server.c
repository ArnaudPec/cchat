#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include "utils.h"

static unsigned int _client_count = 0;
static _Atomic int _uid = 0;

void queue_add(client_t *cl);
void queue_remove(int uid);

client_t *clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void queue_add(client_t *cl)
{
    int i;
    pthread_mutex_lock(&clients_mutex);

    for (i = 0; i < MAX_CLIENTS; ++i) {
        if (!clients[i]) {
            clients[i] = cl;
            break;
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

void queue_remove(int uid)
{
    int i;
    pthread_mutex_lock(&clients_mutex);

    for (i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i]) {
            if (clients[i]->uid == uid) {
                clients[i] = NULL;
                break;
            }
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

void send_message(char *msg, int uid)
{
    int i;
    int ret;
    pthread_mutex_lock(&clients_mutex);

    for (i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i]) {
            if (clients[i]->uid != uid) {
                ret = write(clients[i]->sockfd, msg, strlen(msg));
                if (ret < 0 ) {
                    printf("ERROR: write %s\n", strerror(errno));
                    break;
                }
            }
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

void print_ip(struct sockaddr_in addr)
{
    char *ip = inet_ntoa(addr.sin_addr);
    printf("%s\n", ip);
}

void *handle_client(void *arg)
{
    char buffer[BUFFER_SZ];
    char name[NAME_LEN];
    int leave_flag = 0;
    int ret;
    _client_count++;

    client_t *cli = (client_t*)arg;

    /* Name */
    ret = recv(cli->sockfd, name, NAME_LEN, 0);

    if (ret <= 0 || strlen(name) < 2 || strlen(name) >= NAME_LEN -1) {
       printf("Enter the name correctly\n");
       leave_flag = 1;
    } else {
        strcpy(cli->name, name);
        sprintf(buffer, "%s has joined\n", cli->name);
        printf("%s", buffer);
        send_message(buffer, cli->uid);
    }

    bzero(buffer, BUFFER_SZ);

    for (;;)
    {
        if (leave_flag) {
            break;
        }

        int receive = recv(cli->sockfd, buffer, BUFFER_SZ, 0);

        if (receive > 0) {
            if (strlen(buffer) > 0) {
                send_message(buffer, cli->uid);
                str_trim_lf(buffer, strlen(buffer));
                printf("%s -> %s", buffer, cli->name);
            }
        } else if (receive == 0 || strcmp(buffer, "exit") == 0) {
            sprintf(buffer, "%s has left\n", cli->name);
            printf("%s\n", buffer);
            send_message(buffer, cli->uid);
            leave_flag = 1;
        } else {
            printf("ERROR: -1\n");
            leave_flag = 1;
        }

        bzero(buffer, BUFFER_SZ);
    }
    close(cli->sockfd);
    queue_remove(cli->uid);

    _client_count--;
    free(cli);

    pthread_detach(pthread_self());

    return NULL;
}

int main(int argc, char *argv[])
{
    char *ip = "127.0.0.1";
    int port;
    int ret;
    int option = 1;
    int listenfd = 0, connfd =0;
    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;
    pthread_t tid;

    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }
    port = atoi(argv[1]);

    /* Socket parameters */
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons(port);

    /* Signals */
    signal(SIGPIPE, SIG_IGN);

    ret = setsockopt(listenfd, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR), (char*)&option, sizeof(option));
    if (ret < 0) {
       printf("ERROR: setsockopt %s\n", strerror(errno));
       return EXIT_FAILURE;
    }

    /* Bind */
    ret = bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if (ret < 0) {
       printf("ERROR: bind %s\n", strerror(errno));
       return EXIT_FAILURE;
    }

    /* Listen */
    ret = listen(listenfd, 10);
    if (ret < 0) {
       printf("ERROR: listen %s\n", strerror(errno));
       return EXIT_FAILURE;
    }

    printf("CHATROOM\n");

    for(;;)
    {
        socklen_t clilen = sizeof(cli_addr);
        connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen);

        if ((_client_count + 1) == MAX_CLIENTS) {
            printf("Max clients connected. Connection refused\n");

            print_ip(cli_addr);
            close(connfd);
            continue;
        }

        /* Client settings */
        client_t *cli = (client_t *)malloc(sizeof(client_t));
        cli->address = cli_addr;
        cli->sockfd = connfd;
        cli->uid = _uid++;

        printf("Accepted %d\n", cli->uid);

        /* Add client to the queue */
        queue_add(cli);
        pthread_create(&tid, NULL, &handle_client, (void*)cli);

        sleep(1);
    }

    return 0;
}
