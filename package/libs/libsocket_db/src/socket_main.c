#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <net/if.h>
#include "socket_db_debug.h"

#define MYPORT  8887
#define HEART_PORT  8888
#define QUEUE   20
#define BUFFER_SIZE 1024

int server_sockfd = 0;
int heart_sockfd = 0;
int server_conn = 0;
int heart_conn = 0;

int socket_init()
{
    int optval = 1;
    //define sockfd
    server_sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(server_sockfd == -1){
        perror("socket:\n");
        exit(1);
    }

    if(setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
    {
        DEBUG("setsockopt error\n");
        return -1;
    }
    //define sockaddr_in
    struct sockaddr_in server_sockaddr;
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(MYPORT);
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    DEBUG("--before bind\n");
    //bind, success return 0, fail return -1
    if(bind(server_sockfd,(struct sockaddr *)&server_sockaddr,sizeof(server_sockaddr))==-1)
    {
        ERROR("bind socket fail\n");
        return -1;
    }

    DEBUG("--before listen\n");
    //listen, success return 0, fail return -1
    if(listen(server_sockfd,QUEUE) == -1)
    {
        ERROR("listen socket fail\n");
        return -1;
    }

    //accept, success return val > 0, fail return -1
    //  struct sockaddr_in client_addr;
    //  socklen_t length = sizeof(client_addr);
    //  printf("--before accept\n");

    //  int flags = 0;
    //  flags = fcntl(server_sockfd, F_GETFL, 0);
    //  fcntl(server_sockfd, F_SETFL, flags | O_NONBLOCK);

    //  server_conn = accept(server_sockfd, (struct sockaddr*)&client_addr, &length);
    //  if(server_conn < 0)
    //  {
    //    printf("accept fail\n");
    //    return -1;
    //  }
    DEBUG("--before socket_init return\n");
    return 0;
}


int accept_pc()
{
    //accept, success return val > 0, fail return -1
    struct sockaddr_in client_addr;
    socklen_t length = sizeof(client_addr);
    DEBUG("--before accept\n");
    server_conn = accept(server_sockfd, (struct sockaddr*)&client_addr, &length);
    if(server_conn < 0)
    {
        ERROR("accept fail\n");
        return -1;
    }
}

void *heart_socket()
{
    int optval = 1;
    ///定义sockfd
    heart_sockfd = socket(AF_INET,SOCK_STREAM, 0);
    if(heart_sockfd == -1){
        perror("socket:\n");
        exit(1);
    }

    if(setsockopt(heart_sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
    {
        perror("setsockopt:\n");
        exit(1);
    }

    ///定义sockaddr_in
    struct sockaddr_in server_sockaddr;
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(HEART_PORT);
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    ///bind，成功返回0，出错返回-1
    if(bind(heart_sockfd,(struct sockaddr *)&server_sockaddr,sizeof(server_sockaddr))==-1)
    {
        ERROR("%s:bind fail\n", __func__);
        perror("bind:\n");
        exit(1);
    }
    printf("%s:bind\n", __func__);

    ///listen，成功返回0，出错返回-1
    if(listen(heart_sockfd,QUEUE) == -1)
    {
        ERROR("%s:listen fail\n", __func__);
        exit(1);
    }
    printf("%s:listen\n", __func__);

    ///客户端套接字
    char buffer[BUFFER_SIZE];
    struct sockaddr_in client_addr;
    socklen_t length = sizeof(client_addr);

    ///成功返回非负描述字，出错返回-1
    int heart_conn = accept(heart_sockfd, (struct sockaddr*)&client_addr, &length);
    if(heart_conn<0)
    {
        ERROR("%s:heart_connect\n", __func__);
        exit(1);
    }

    printf("%s:accept\n", __func__);

    while(1)
    {
        memset(buffer,0,sizeof(buffer));
        int len = recv(heart_conn, buffer, sizeof(buffer),0);
        //printf("recv---buffer = %s  len = %d\n",buffer,len);
        //fputs(buffer, stdout);
        len = send(heart_conn, buffer, len, 0);
        //printf("send---buffer = %s  len = %d\n",buffer,len);
        //sleep(1);
    }
}

int recv_from_socket(char *buf,int length)
{
    int len = 0;
    len = recv(server_conn, buf, length,0);

    return len;
}

int send_to_sockt(char *buf,int length)
{
    int len = 0;
    int i = 0;
    DEBUG("send size=%d\n", length);
    DEBUG("[%s:%s] line=%d\n",__FILE__, __func__, __LINE__);
    DEBUG("send_to_sockt buf=%s\n",buf + 20);
    len = send(server_conn, buf, length, 0);

    return len;
}

int socket_exit()
{
    close(server_conn);
    close(server_sockfd);

    return 0;
}
