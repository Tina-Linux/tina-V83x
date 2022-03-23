#ifndef __SOCKET_MAIN_H
#define __SOCKET_MAIN_H

/*head file for socket_main.c*/
int socket_init();
int socket_exit();
int recv_from_socket(char *buf,int length);
int send_to_sockt(char *buf,int length);
int accept_pc();
void *heart_socket();

#endif
