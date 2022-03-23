#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h> //ca
#include "socket_db_debug.h"

#define MSGKEY_DB2ADBD 123
#define MSGKEY_ADBD2DB 124
#define MSGKEY_RES2PC  125

static int msqid_db2adbd = 0;
static int msqid_adbd2db = 0;
static int msqid_res2pc = 0;

static int  count_db2adbd;
static int  count_adbd2db;
static int  count_res2pc;

struct msgstru
{
    long msgtype;
    char msgtext[2048];
};

int queue_init()
{
    DEBUG("----enter queue2_init\n");
    msqid_adbd2db=msgget(MSGKEY_ADBD2DB,IPC_EXCL);/*check whether queue is exited*/
    if(msqid_adbd2db < 0){
        DEBUG("---queue2 not exited\n");
        msqid_adbd2db = msgget(MSGKEY_ADBD2DB,IPC_CREAT|0666);/*create queue*/
        if(msqid_adbd2db <0){
            ERROR("failed to create msq_adbd2db\n");
            return -1;
        }
    }
    DEBUG("[func = %s], [LINE = %d],msqid_adbd2db = %d\n",__func__,__LINE__,msqid_adbd2db);
    DEBUG("----queue2 create success\n");
    count_adbd2db = 0;/*record queue_adbd2db msg number*/

    DEBUG("----enter queue1_init\n");
    msqid_db2adbd=msgget(MSGKEY_DB2ADBD,IPC_EXCL);/*check whether queue is exited*/
    if(msqid_db2adbd < 0){
        DEBUG("---queue1 not exited\n");
        msqid_db2adbd = msgget(MSGKEY_DB2ADBD,IPC_CREAT|0666);/*create queue*/
        if(msqid_db2adbd <0){
            ERROR("failed to create msq_db2adbd\n");
            return -1;
        }
    }
    DEBUG("[func = %s], [LINE = %d],msqid_db2adbd = %d\n",__func__,__LINE__,msqid_db2adbd);
    DEBUG("----queue1 create success\n");
    count_db2adbd = 0;/*record queue_db2adbd msg number*/

    DEBUG("----enter queue3_init\n");
    msqid_res2pc=msgget(MSGKEY_RES2PC,IPC_EXCL);/*check whether queue is exited*/
    if(msqid_res2pc < 0){
        DEBUG("---queue3 not exited\n");
        msqid_res2pc = msgget(MSGKEY_RES2PC,IPC_CREAT|0666);/*create queue*/
        if(msqid_res2pc <0){
            ERROR("failed to create msq_db2adbd\n");
            return -1;
        }
    }
    DEBUG("[func = %s], [LINE = %d],msqid_db2adbd = %d\n",__func__,__LINE__,msqid_res2pc);
    DEBUG("----queue1 create success\n");
    count_res2pc = 0;/*record queue_res2pc msg number*/

    DEBUG("----before queue_init return\n");
    return 0;
}


int send_db2adbd(char *string,int id_num)
{
    struct msgstru msgs;
    int ret_value;
    char buf[1024];
    memset(buf,0,sizeof(buf));
    strcpy(buf,(string+20));
    memset(&msgs,0,sizeof(msgs));
    memcpy(msgs.msgtext,string,strlen(string + 20)+20);

    msgs.msgtype = id_num;
    DEBUG("[func = %s] buf = %s  id_num = %d\n",__func__,buf,id_num);
    //send msg to queue
    int msgid;
     msgid = msgget(MSGKEY_DB2ADBD,IPC_EXCL );/*check msg queue whether is exited */
    if(msgid < 0){
        ERROR("msq not existed!\n");
        return -1;
    }

    DEBUG("[func = %s], [LINE = %d],msgid = %d\n",__func__,__LINE__,msgid);
    ret_value = msgsnd(msgid,&msgs,sizeof(struct msgstru),0);
    if ( ret_value < 0 ) {
        ERROR("msgsnd() write msg failed  ret_value = %d  err = %d\n",ret_value,errno);
        return -1;
    }

    return 1;
}

int send_adbd2db(char *string,int id_num)
{
    struct msgstru msgs;
    int ret_value;
    memset(&msgs,0,sizeof(msgs));
    memcpy(&msgs.msgtext,string,strlen(string + 20) + 20);
    msgs.msgtype = id_num;
    int msgid;
    msgid = msgget(MSGKEY_ADBD2DB,IPC_EXCL );/*check msg queue whether is exited */
    DEBUG("[func = %s], [LINE = %d],msgid = %d\n",__func__,__LINE__,msgid);
    if(msgid < 0){
        ERROR("msq not existed! errno\n");
        return -1;
    }
    //send msg to queue
    DEBUG("[func = %s], [LINE = %d],id_num = %d\n",__func__,__LINE__,id_num);
    ret_value = msgsnd(msgid,&msgs,sizeof(struct msgstru),0);
    if ( ret_value < 0 ) {
        ERROR("msgsnd() write msg failed\n");
        return -1;
    }

    return 0;
}

int send_res2pc(char *string,int id_num)
{
    struct msgstru msgs;
    int ret_value;
    memset(&msgs,0,sizeof(msgs));
    memcpy(&msgs.msgtext,string,strlen(string + 20) + 20);
    msgs.msgtype = id_num;
    int msgid;
    msgid = msgget(MSGKEY_RES2PC,IPC_EXCL );/*check msg queue whether is exited */
    DEBUG("[func = %s], [LINE = %d],msgid = %d\n",__func__,__LINE__,msgid);
    if(msgid < 0){
        ERROR("msq not existed! errno\n");
        return -1;
    }
    //send msg to queue
    DEBUG("[func = %s], [LINE = %d],id_num = %d\n",__func__,__LINE__,id_num);
    ret_value = msgsnd(msgid,&msgs,sizeof(struct msgstru),0);
    if ( ret_value < 0 ) {
        ERROR("msgsnd() write msg failed\n");
        return -1;
    }

    return 0;
}

int get_db2adbd_number() //ca
{
    return 1;
}

int get_adbd2db_number() //ca
{
    return 1;
}

int get_res2pc_number() //ca
{
    return 1;
}

int recv_db2adbd(char *buf)
{
    struct msgstru msgs;
    int msgid,ret_value;;

    if(get_db2adbd_number() < 1)
    return 0;

    msgid = msgget(MSGKEY_DB2ADBD,IPC_EXCL );/*check msg queue whether is exited */
    if(msgid < 0){
        ERROR("msq not existed!\n");
    }
    DEBUG("[func = %s], [LINE = %d],msgid = %d\n",__func__,__LINE__,msgid);
    /*receive msg from queue*/
    ret_value = msgrcv(msgid,&msgs,sizeof(struct msgstru),0,0);
    if(ret_value < 0){
        ERROR("[func = %s], [LINE = %d], fail %d\n",__func__,__LINE__,ret_value);
        return -1;
    }
    memcpy(buf,msgs.msgtext, strlen(msgs.msgtext + 20) + 20);

    return 0;
}

int recv_adbd2db(char *buf,int id)
{
    struct msgstru msgs;
    int msgid,ret_value;

    if(get_adbd2db_number() < 1){
        return -1;
    }
    msgid = msgget(MSGKEY_ADBD2DB,IPC_EXCL );/*check msg queue whether is exited */
    DEBUG("[func = %s], [LINE = %d],id = %d\n",__func__,__LINE__,id);
    if(msgid < 0){
        ERROR("msq not existed! errno\n");
        return -1;
    }

    /*receive msg from queue*/
    DEBUG("[func = %s], [LINE = %d],msgid = %d\n",__func__,__LINE__,msgid);
    DEBUG("[func = %s], [LINE = %d],id = %d\n",__func__,__LINE__,id);
    ret_value = msgrcv(msgid,&msgs,sizeof(struct msgstru),id,0);
    if(ret_value < 0){
        ERROR("recv_adbd2db ret_value=%d,,,error=%d, id=%d\n",ret_value,errno,id);
        return -1;
    }else{
        DEBUG("[func = %s], [LINE = %d],buf= %s\n",__func__,__LINE__,msgs.msgtext);
    }
    strcpy(buf,msgs.msgtext);

    return 1;
}

int recv_res2pc(char *buf,int *id)
{
    struct msgstru msgs;
    int msgid,ret_value;

    if(get_res2pc_number() < 1){
        return -1;
    }
    msgid = msgget(MSGKEY_RES2PC,IPC_EXCL );/*check msg queue whether is exited */
    DEBUG("[func = %s], [LINE = %d]\n",__func__,__LINE__);
    if(msgid < 0){
        ERROR("msq not existed! errno\n");
        return -1;
    }

    /*receive msg from queue*/
    DEBUG("[func = %s], [LINE = %d],msgid = %d\n",__func__,__LINE__,msgid);
    DEBUG("[func = %s], [LINE = %d]\n",__func__,__LINE__);
    ret_value = msgrcv(msgid,&msgs,sizeof(struct msgstru),0,0);
    if(ret_value < 0){
        ERROR("recv_adbd2db ret_value=%d,,,error=%d\n",ret_value,errno);
        return -1;
    }else{
        DEBUG("[func = %s], [LINE = %d],buf= %s\n",__func__,__LINE__,msgs.msgtext);
    }
    strcpy(buf,msgs.msgtext);
    *id = msgs.msgtype;
    count_res2pc = count_res2pc - 1;

    return 1;
}

int queue_exit()
{
    //delete MSG_DB2ADBD
    msgctl(msqid_db2adbd,IPC_RMID,0);

    //delete MSG_ADBD2DB
    msgctl(msqid_adbd2db,IPC_RMID,0);

    //delete MSG_RES2PC
    msgctl(msqid_res2pc,IPC_RMID,0);

    return 0;
}
