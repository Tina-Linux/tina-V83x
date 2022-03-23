#include <stdlib.h>
#include <unistd.h>
#include "socket_app.h"

/********************SERVER API***************************/
int setup_socket_server(tSOCKET_APP *app_socket)
{
    unlink (app_socket->sock_path);
    if ((app_socket->server_sockfd = socket (AF_UNIX, SOCK_STREAM, 0)) < 0) {
        printf("fail to create socket\n");
        perror("socket");
        return -1;
    }
    app_socket->server_address.sun_family = AF_UNIX;
    strcpy (app_socket->server_address.sun_path, app_socket->sock_path);
    app_socket->server_len = sizeof (app_socket->server_address);
    app_socket->client_len = sizeof (app_socket->client_address);
    if ((bind (app_socket->server_sockfd, (struct sockaddr *)&app_socket->server_address, app_socket->server_len)) < 0) {
        printf("fail to bind socket\n");
        perror("bind");
        return -1;

    }
    if (listen (app_socket->server_sockfd, 10) < 0) {
        printf("fail to listen socket\n");
        perror("listen");
        return -1;
    }
    printf ("Server is ready for client connect...\n");

    return 0;
}

int accpet_client(tSOCKET_APP *app_socket)
{
    app_socket->client_sockfd = accept (app_socket->server_sockfd, (struct sockaddr *)&app_socket->server_address, (socklen_t *)&app_socket->client_len);
    if (app_socket->client_sockfd == -1) {
        printf("fail to accept socket\n");
        perror ("accept");
        return -1;
    }
    return 0;
}

void teardown_socket_server(tSOCKET_APP *app_socket)
{
    unlink (app_socket->sock_path);

    if(app_socket->server_sockfd) {
        close(app_socket->server_sockfd);
        app_socket->server_sockfd = 0;
    }

    if(app_socket->client_sockfd) {
        close(app_socket->client_sockfd);
        app_socket->client_sockfd = 0;
    }
}

/********************CLIENT API***************************/
int setup_socket_client(char *socket_path)
{
    struct sockaddr_un address;
    int sockfd,  len;

    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        printf("can not creat socket\n");
        return -1;
    }

    address.sun_family = AF_UNIX;
    strcpy (address.sun_path, socket_path);
    len = sizeof (address);

    if (connect (sockfd, (struct sockaddr *)&address, len) == -1) {
        printf("can not connect to socket\n");
        return -1;
    }

    return sockfd;
}

void teardown_socket_client(int sockfd)
{
    if(sockfd) {
        close(sockfd);
        sockfd = 0;
    }
}

/********************COMMON API***************************/
int socket_send(int sockfd, char *msg, int len)
{
    int bytes;
    if (sockfd < 0) {
        printf("invalid sockfd\n");
        return -1;
    }
    if ((bytes = send(sockfd, msg, len, 0)) < 0) {
        printf("fail to send\n");
        perror ("send");
    }
    return bytes;
}

int socket_recieve(int sockfd, char *msg, int len)
{
    int bytes;

    if (sockfd < 0) {
        printf("invalid sockfd\n");
        return -1;
    }
    printf("Humble add for debug\n");
    if ((bytes = recv(sockfd, msg, len, 0)) < 0)
    {
        printf("fail to recv\n");
        perror ("recv");
    }
    return bytes;
}
