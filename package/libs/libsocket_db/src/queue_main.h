#ifndef __QUEUE_MAIN_H
#define __QUEUE_MAIN_H

/*head file for queue_main.c*/
int queue_init();
int queue_exit();
int send_db2adbd(char *string,int id_num);
int send_adbd2db(char *string,int id_num);
int send_res2pc(char *string,int id_num);
int recv_db2adbd(char *buf);
int recv_adbd2db(char *buf,int id);
int recv_res2pc(char *buf,int *id);
int get_db2adbd_number();
int get_adbd2db_number();
int get_res2pc_number();

#endif
