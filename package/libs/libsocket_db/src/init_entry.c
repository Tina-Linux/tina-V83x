#include "socket_db_debug.h"
#include "queue_main.h"
#include "socket_main.h"
#include "init_entry.h"
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/shm.h>

#define PACKET_HEADLEN  20
#define SHM_ID 87654

pthread_t thr_adbd2db;
pthread_t thr_db2adbd;
pthread_t thr_heart;
pthread_t thr_res2pc;

int id_num =100 ;
int msg_shmid;
pthread_mutex_t lock;
pthread_mutexattr_t mattr;

//20 byte head info
struct head_packet_t{
    unsigned int magic;
    unsigned int version;
    unsigned int type;
    unsigned int payloadsize;
    unsigned int externsize;
};

struct test_case_t{
    char name[20];
    int id;
};

unsigned int ltob(unsigned int value)
{
  return (value & 0x000000FFU) << 24 | (value & 0x0000FF00U) << 8 |
    (value & 0x00FF0000U) >> 8 | (value & 0xFF000000U) >> 24;
}

char *add_head(char* buf, int payloadsize,int externsize,int type)
{
    struct head_packet_t head;

    head.magic = MAGIC;
    head.externsize = externsize;
    head.payloadsize = payloadsize;
    head.type = 1;
    head.version = 0x00000001;

    memcpy(buf,&head,sizeof(struct head_packet_t));

    return buf;
}

int msgidInit()
{
    int ret = 0;
    int *addr = NULL;

    pthread_mutexattr_init(&mattr);
    pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&lock, &mattr);

    ret = shmget(SHM_ID, 4, (IPC_CREAT | 0666));
    if(ret < 0)
    {
        DEBUG("shmget failure\n");
        return ret;
    }
    msg_shmid = ret;

    if((addr = (int *)shmat(msg_shmid, 0, 0)) == (int *)-1)
    {
        DEBUG("shmat failure\n");
        return -1;
    }
    *addr = id_num;

    ret = shmdt((void *)addr);
    if(ret < 0)
    {
        DEBUG("shmdt failure\n");
        return ret;
    }

    return ret;
}

int msgidExit()
{
    pthread_mutex_destroy(&lock);
    shmctl(msg_shmid, IPC_RMID, 0);
}

int getMsgid()
{
    int ret = 0;
    int shmid;
    int *addr = NULL;
    int msgid;
    int value;

    shmid = shmget(SHM_ID, 0, 0);
    if(shmid < 0)
    {
        DEBUG("%s: shmget failure\n", __func__);
        return -1;
    }

    if((addr = (int *)shmat(shmid, 0, 0)) == (int *)-1)
    {
        DEBUG("%s: shmat failure\n", __func__);
        return -1;
    }

    pthread_mutex_lock(&lock);
    value = *addr;
    msgid = value;
    value += 1;
    *addr = value;
    pthread_mutex_unlock(&lock);

    ret = shmdt((void *)addr);
    if(ret < 0)
    {
        DEBUG("%s: shmdt failure\n", __func__);
        return ret;
    }

    return msgid;
}

void *threadfunc_res2pc()
{
    char buf[1024];
    int ret = 0;
    int id;
    char cmd[32];

    DEBUG("---threadfunc_res2pc\n");
    while(1)
    {
        if(get_res2pc_number() > 0)
        {
            memset(buf,0,sizeof(buf));
            ret = recv_res2pc(buf, &id);
            if(ret < 0)
            {
                ERROR("threadfunc_res2pc recv_res2pc fail\n");
                continue;
            }
      }
      else
      {
          continue;
      }

      ret = GetXmlNode(buf, "req", "cmd", cmd);
      if(ret < 0)
      {
          ERROR("threadfunc_res2pc get xml node fail\n");
          continue;
      }
      /* start req from pc */
      if(!strcmp(cmd, "start"))
      {
          sendStartRes2pc(id);
      }
      /* download req from pc */
      else if(!strcmp(cmd, "download"))
      {

      }
      else
      {

      }

      sleep(1);
    }

}

void *threadfunc_db2adbd()
{
    char buf[1024];
    int ret = 0;

    DEBUG("---threadfunc_db2adbd\n");
    while(1)
    {
        if(get_db2adbd_number() > 0)
        {
            memset(buf,0,sizeof(buf));
            ret = recv_db2adbd(buf);
            if(ret < 0)
            {
                DEBUG("threadfunc_db2adbd recv_db2adbd fail\n");
                continue;
            }
        }
        else
        {
            DEBUG("get_db2adbd_number fail\n");
            continue;

        }
        DEBUG("[func = %s] buf = %s      len = %d \n",__func__,buf+20,strlen(buf+20));
        ret = send_to_sockt(buf,strlen(buf+20) + 20);
        if(ret < 0)
        {
            ERROR("threadfunc_db2adbd send_to_sockt fail\n");
            continue;
        }

        sleep(1);
    }
}


void *threadfunc_adbd2db()
{
    char buf[1024];
    char head_one[HEAD_SIZE];
    int ret;
    struct head_packet_t *head;

    DEBUG("---threadfunc_adbd2db\n");

    /*create thread for adbd receive data from queue_db2adbd and
    *write data to pc socket
    */
    DEBUG("----pthread_create3 start\n");
    ret = pthread_create(&thr_db2adbd,NULL,threadfunc_db2adbd,NULL);
    if(ret){
        ERROR("create thread db2adbd fail\n");
    }
    /*
    *create thread for pc req
    */
    DEBUG("----pthread_create4 start\n");
    ret = pthread_create(&thr_res2pc,NULL,threadfunc_res2pc,NULL);
    if(ret){
        ERROR("create thread res2pc fail\n");
    }

    while(1)
    {
        ret = recv_from_socket(head_one,HEAD_SIZE);
        head = (struct head_packet_t *)head_one;
        if(ret < 0)
        {
            ERROR("get data fail from socket\n");
            continue; //no date continue
        }
        DEBUG("magic = %x   len = %d\n",ltob(head->magic),sizeof(int));

        if(ltob(head->magic) != MAGIC)
        {
            ERROR("head info error\n");
            continue; //date is not right,continue
        }

        ret = recv_from_socket(buf,(ltob(head->payloadsize) + ltob(head->externsize)));
        if(ret < 0)
        {
            ERROR("get data info from socket fail\n");
            continue;
        }
        DEBUG("---recv buf = %s\n",buf);

        char temp[32];
        int id = 0;
        ret = GetXmlNode(buf, NULL, "msgid", temp);
        if(ret < 0)
        {
            continue;
        }
        id = atoi(temp);

        ret = GetXmlRootNodeName(buf, temp);
        if(ret < 0)
        {
            continue;
        }
        if(!strcmp(temp, "req"))
        {
            ret = send_res2pc(buf,id);
            if(ret < 0)
            {
                ERROR("send data to queue fail\n");
                continue;
            }
        }
        else
        {
            ret = send_adbd2db(buf,id);
            if(ret < 0)
            {
                ERROR("send data to queue fail\n");
                continue;
            }
        }
        sleep(1);
    }
}


int init_entry()
{
    int ret;

    DEBUG("---enter init_entry\n");
    ret = socket_init();
    if(ret == -1)
    {
        ERROR("socket init fail\n");
        return -1;
    }

    DEBUG("---before queue_init\n");
    ret = queue_init();
    if(ret == -1)
    {
        ERROR("queue init fail\n");
        return -1;
    }

    ret = msgidInit();
    if(ret < 0)
    {
        ERROR("init message failure\n");
        return -1;
    }

    DEBUG("----before init_entry return\n");
    return 0;
}


int waitSocketConnect()
{
    int ret = 0;
    DEBUG("wait for client...........\n");
    /*
    *create thread for heart
    */
    DEBUG("----pthread_create1 start\n");
    ret = pthread_create(&thr_heart,NULL,heart_socket,NULL);
    if(ret){
        ERROR("create thread thr_heart fail\n");
    }
    accept_pc();

    /*create thread for adbd receive data from pc socket
    *and write data to queue_adbd2db
    */
    DEBUG("----pthread_create2 start\n");
    ret = pthread_create(&thr_adbd2db,NULL,threadfunc_adbd2db,NULL);
    if(ret){
        ERROR("create thread adbd2db fail\n");
    }

    return 1;
}

int sendStartCase(const char *testname)
{
    char buf[1024];
    char header[128];
    int msgid;

    msgid = getMsgid();
    strcpy(buf + HEAD_SIZE,sendStartCase_packet(testname,msgid));
    int len = 0;
    len = strlen(buf + HEAD_SIZE);
    memcpy(buf, add_head(header, len, 0,1), HEAD_SIZE);
    send_db2adbd(buf,msgid);

    return msgid;
}

int sendStartCase_fix(const char *testname, char *buf)
{
    char header[128];
    int msgid;

    msgid = getMsgid();
    strcpy(buf + HEAD_SIZE,sendStartCase_packet(testname,msgid));
    int len = 0;
    len = strlen(buf + HEAD_SIZE);
    memcpy(buf, add_head(header, len, 0,1), HEAD_SIZE);

    return msgid;
}

int sendResult(const char *testname, const char *mark, int result)
{
    char buf[1024];
    char header[128];
    int msgid;

    msgid = getMsgid();
    strcpy(buf + HEAD_SIZE,sendResult_packet(testname, mark, result,msgid));
    int len = 0;
    len = strlen(buf + HEAD_SIZE);
    memcpy(buf, add_head(header, len, 0,1), HEAD_SIZE);
    send_db2adbd(buf,msgid);

    return msgid;
}

int sendResult_fix(const char *testname, const char *mark, int result, char *buf)
{
    char header[128];
    int msgid;

    msgid = getMsgid();
    strcpy(buf + HEAD_SIZE,sendResult_packet(testname, mark, result,msgid));
    int len = 0;
    len = strlen(buf + HEAD_SIZE);
    memcpy(buf, add_head(header, len, 0,1), HEAD_SIZE);

    return msgid;
}

int sendTip(const char *testname, const char *tip)
{
    char buf[1024];
    char header[128];
    int msgid;

    msgid = getMsgid();
    strcpy(buf + HEAD_SIZE,sendTip_packet(testname, tip,msgid));
    int len = 0;
    len = strlen(buf + HEAD_SIZE);
    memcpy(buf, add_head(header, len, 0,1), HEAD_SIZE);
    send_db2adbd(buf,msgid);

    return msgid;
}

int sendTip_fix(const char *testname, const char *tip, char *buf)
{
    char header[128];
    int msgid;
    char buffer[512];

    memset(buffer, 0, sizeof(buffer));
    strncpy(buffer, tip, strlen(tip));
    msgid = getMsgid();
    strcpy(buf + HEAD_SIZE,sendTip_packet(testname, buffer,msgid));
    int len = 0;
    len = strlen(buf + HEAD_SIZE);
    memcpy(buf, add_head(header, len, 0,1), HEAD_SIZE);

    return msgid;
}

int sendTestEnd(int result, const char *mark)
{
    char buf[1024];
    char header[128];
    int msgid;

    msgid = getMsgid();
    strcpy(buf + HEAD_SIZE,sendTestEnd_packet(result, mark,msgid));
    int len = 0;
    len = strlen(buf + HEAD_SIZE);
    memcpy(buf, add_head(header, len, 0,1), HEAD_SIZE);
    send_db2adbd(buf,msgid);

    return msgid;
}

int sendTestEnd_fix(int result, const char *mark, char *buf)
{
    char header[128];
    int msgid;

    msgid = getMsgid();
    strcpy(buf + HEAD_SIZE,sendTestEnd_packet(result, mark,msgid));
    int len = 0;
    len = strlen(buf + HEAD_SIZE);
    memcpy(buf, add_head(header, len, 0,1), HEAD_SIZE);

    return msgid;
}


int sendCMDselect(const char *testname, const char *tip, const char* mark, int timeout)
{
    char buf[1024];
    char header[128];
    int msgid;

    msgid = getMsgid();
    strcpy(buf + HEAD_SIZE,sendCMDselect_packet(testname, tip, mark, timeout, msgid));
    int len = 0;
    len = strlen(buf + HEAD_SIZE);
    memcpy(buf, add_head(header, len, 0,1), HEAD_SIZE);
    send_db2adbd(buf,msgid);

    return msgid;
}

int sendCMDscan_fix(const char *testname, const char *tip, const char* key, char *buf)
{
    char header[128];
    int msgid;

    DEBUG("fun = %s\n",__func__);
    msgid = getMsgid();
    strcpy(buf + HEAD_SIZE,sendCMDscan_packet(testname, tip, key, msgid));
    int len = 0;
    len = strlen(buf + HEAD_SIZE);
    memcpy(buf, add_head(header, len, 0,1), HEAD_SIZE);

    return msgid;
}


int sendCMDselect_fix(const char *testname, const char *tip, const char* mark, int timeout, char *buf)
{
    char header[128];
    int msgid;

    DEBUG("fun = %s\n",__func__);
    msgid = getMsgid();
    strcpy(buf + HEAD_SIZE,sendCMDselect_packet(testname, tip, mark, timeout, msgid));
    int len = 0;
    len = strlen(buf + HEAD_SIZE);
    memcpy(buf, add_head(header, len, 0,1), HEAD_SIZE);

    return msgid;
}

int sendCMDOperator(const char *testname, const char *plugin, const char * datatype, char * data)
{
     char buf[1024];
     char header[128];
     char database64[518];
     int ret = 0;
     int msgid;

     memset(database64,0x0,sizeof(database64));
     ret = Encbase64(data,strlen(data),database64);
     if(ret < 0){
         ERROR("str change base64 fail");
         return -1;
     }

     msgid = getMsgid();
     strcpy(buf + HEAD_SIZE,sendCMDOperator_packet(testname, plugin,  datatype, database64,msgid));
     int len = 0;
     len = strlen(buf + HEAD_SIZE);
     memcpy(buf, add_head(header, len, 0,1),HEAD_SIZE);
     send_db2adbd(buf,msgid);

     return msgid;
}

int sendCMDOperator_fix(const char *testname, const char *plugin, const char * datatype, char * data, char *buf)
{
    char header[128];
    char database64[518];
    int ret = 0;
    int msgid;

    memset(database64,0x0,sizeof(database64));
    ret = Encbase64(data,strlen(data),database64);
    if(ret < 0){
        ERROR("str change base64 fail");
        return -1;
    }

    msgid = getMsgid();
    strcpy(buf + HEAD_SIZE,sendCMDOperator_packet(testname, plugin,  datatype, database64,msgid));
    int len = 0;
    len = strlen(buf + HEAD_SIZE);
    memcpy(buf, add_head(header, len, 0,1),HEAD_SIZE);

    return msgid;
}

int sendEdit(const char *testname, const char *tip, const char *editvalue, const char *mark, int timeout)
{
  char buf[1024];
  char header[128];
  int msgid;

  msgid = getMsgid();
  strcpy(buf + HEAD_SIZE,sendEdit_packet(testname, tip, editvalue, mark, timeout, msgid));
  int len = 0;
  len = strlen(buf + HEAD_SIZE);
  memcpy(buf, add_head(header, len, 0,1), HEAD_SIZE);
  send_db2adbd(buf,msgid);

  return msgid;
}

int sendEdit_fix(const char *testname, const char *tip, const char *editvalue, const char *mark, int timeout, char *buf)
{
  char header[128];
  int msgid;

  DEBUG("fun = %s\n",__func__);
  msgid = getMsgid();
  strcpy(buf + HEAD_SIZE,sendEdit_packet(testname, tip, editvalue, mark, timeout, msgid));
  int len = 0;
  len = strlen(buf + HEAD_SIZE);
  memcpy(buf, add_head(header, len, 0,1), HEAD_SIZE);

  return msgid;
}

int sendStartTestResponse(struct devInfo *dev, int result, const char *mark, int msgid)
{
    char buf[1024];
    char header[128];

    strcpy(buf + HEAD_SIZE,sendStartTestResponse_packet(dev, result, mark,msgid));
    int len = 0;
    len = strlen(buf + HEAD_SIZE);
    memcpy(buf, add_head(header, len, 0,1),HEAD_SIZE);
    send_db2adbd(buf,msgid);

    return msgid;
}

int sendStartTestResponse_fix(struct devInfo *dev, int result, const char *mark, int msgid, char *buf)
{
    char header[128];

    strcpy(buf + HEAD_SIZE,sendStartTestResponse_packet(dev, result, mark, msgid));
    int len = 0;
    len = strlen(buf + HEAD_SIZE);
    memcpy(buf, add_head(header, len, 0,1),HEAD_SIZE);

    return msgid;
}

int sendCommonResponse(int result, const char *mark, int msgid)
{
    char buf[1024];
    char header[128];

    strcpy(buf + HEAD_SIZE,sendCommonResponse_packet(result, mark, msgid));
    int len = 0;
    len = strlen(buf + HEAD_SIZE);
    memcpy(buf, add_head(header, len, 0,1),HEAD_SIZE);
    send_db2adbd(buf,msgid);

    return msgid;
}

int sendCommonResponse_fix(int result, const char *mark, int msgid, char *buf)
{
    char header[128];

    strcpy(buf + HEAD_SIZE,sendCommonResponse_packet(result, mark, msgid));
    int len = 0;
    len = strlen(buf + HEAD_SIZE);
    memcpy(buf, add_head(header, len, 0,1),HEAD_SIZE);

    return msgid;
}

int downloadFileRequest(struct fileInfo *file, const char *testname, const char *mark)
{
    char buf[1024];
    char header[128];
    int msgid;

    msgid = getMsgid();
    strcpy(buf + HEAD_SIZE,downloadFileRequest_packet(file, testname, mark, msgid));
    int len = 0;
    len = strlen(buf + HEAD_SIZE);
    memcpy(buf, add_head(header, len, 0,1), HEAD_SIZE);
    send_db2adbd(buf,msgid);
    printf("\n");

    return msgid;
}

int downloadFileRequest_fix(struct fileInfo *file, const char* testname, const char *mark, char *buf)
{
    char header[128];
    int msgid;

    msgid = getMsgid();
    strcpy(buf + HEAD_SIZE,downloadFileRequest_packet(file, testname, mark, msgid));
    int len = 0;
    len = strlen(buf + HEAD_SIZE);
    memcpy(buf, add_head(header, len, 0,1), HEAD_SIZE);

    return msgid;
}

int uploadFileRequest(struct fileInfo *file, const char *testname, const char *mark)
{
    char buf[1024];
    char header[128];
    int msgid;

    msgid = getMsgid();
    strcpy(buf + HEAD_SIZE,uploadFileRequest_packet(file, testname, mark, msgid));
    int len = 0;
    len = strlen(buf + HEAD_SIZE);
    memcpy(buf, add_head(header, len, file->size,1), HEAD_SIZE);
    send_db2adbd(buf,msgid);

    return msgid;
}

int uploadFileRequest_fix(struct fileInfo *file, const char *testname, const char *mark, char *buf)
{
    char header[128];
    int msgid;

    msgid = getMsgid();
    strcpy(buf + HEAD_SIZE,uploadFileRequest_packet(file, testname, mark, msgid));
    int len = 0;
    len = strlen(buf + HEAD_SIZE);
    memcpy(buf, add_head(header, len, file->size,1), HEAD_SIZE);

    return msgid;
}

int sendShowResRequest(struct fileInfo *file, const char *type, const char *testname, const char *mark)
{
    char buf[1024];
    char header[128];
    int msgid;

    msgid = getMsgid();
    strcpy(buf + HEAD_SIZE,sendShowRes_packet(file, type, testname, mark, msgid));
    int len = 0;
    len = strlen(buf + HEAD_SIZE);
    memcpy(buf, add_head(header, len, file->size,1), HEAD_SIZE);
    send_db2adbd(buf,msgid);

    return msgid;
}

int sendShowResRequest_fix(struct fileInfo *file, const char *type, const char *testname, const char *mark, char *buf)
{
    char header[128];
    int msgid;

    msgid = getMsgid();
    strcpy(buf + HEAD_SIZE,sendShowRes_packet(file, type, testname, mark, msgid));
    int len = 0;
    len = strlen(buf + HEAD_SIZE);
    memcpy(buf, add_head(header, len, file->size,1), HEAD_SIZE);

    return msgid;
}

int getReturn(int  msgid)
{
    int ret;
    int temp_num;
    char buf[1024];
    char szVal[1024];

    ret = recv_adbd2db(buf,msgid);
    if(ret == -1)
    {
        ERROR("get return fail\n");
        return -1;
    }
    DEBUG("get return buf:%s\n",buf);

    GetXmlNode(buf, "response", "result", szVal);

    if (strcasecmp(szVal, "ok") != 0)
        return 0;
    else
        return 1;
}

int getReturn_fix(char *buf)
{
    int ret;
    char szVal[1024];

    GetXmlNode(buf, "response", "result", szVal);

    if (strcasecmp(szVal, "ok") != 0)
        return 0;
    else
        return 1;
}


int getValue(int msgid, const char* szKey, char*szValue, int *nLen)
{
    int ret;
    char value[512];
    char buf[1024];
    char node[32];

    memset(buf,0x0,sizeof(buf));
    memset(value,0x0,sizeof(value));

    ret = recv_adbd2db(buf,msgid);
    if(ret <= 0)
    {
        ERROR("getValue buf fail\n");
        return -1;
    }

    strcpy(node, szKey);
    ret = GetXmlNode(buf, "response", node, value);
    if(ret == -1)
    {
        DEBUG("get value fialure\n");
        return ret;
    }

    strcpy(szValue, value);
    DEBUG("szValue : %s\n",szValue);

    *nLen = strlen(szValue);

    return 1;
}

int getValue_fix(char* buf, const char* szKey, char*szValue, int *nLen)
{
    int ret;
    char value[512];
    char node[32];

    memset(value,0x0,sizeof(value));
    strcpy(node, szKey);
    ret = GetXmlNode(buf, "response", node, value);
    if(ret == -1)
    {
        ERROR("get value fialure\n");
        return ret;
    }

    strcpy(szValue, value);
    DEBUG("szValue : %s\n",szValue);

    *nLen = strlen(szValue);

    return 1;
}

int getPacketType(int msgid, char *type)
{
    int ret;
    char value[16];
    char buf[1024];

    memset(buf,0x0,sizeof(buf));
    memset(value,0x0,sizeof(value));

    ret = recv_adbd2db(buf,msgid);
    if(ret <= 0)
    {
        ERROR("getValue buf fail\n");
        return -1;
    }

    ret = GetXmlRootNodeName(buf, value);
    if(ret == -1)
    {
        DEBUG("get value fialure\n");
        return ret;
    }

    strcpy(type, value);
    DEBUG("type : %s\n",type);

    return 1;
}

int getPacketType_fix(char *buf, char *type)
{
    int ret;
    char value[16];

    memset(value,0x0,sizeof(value));
    ret = GetXmlRootNodeName(buf, value);
    if(ret == -1)
    {
        ERROR("get value fialure\n");
        return ret;
    }

    strcpy(type, value);
    DEBUG("type : %s\n",type);

    return 1;
}

int getPacketBuff(int msgid, char *xmlBuffer, char *extBuffer, int *xmlLen, int *extLen)
{
    int ret;
    char value[1024];
    char buf[1024];

    memset(buf,0x0,sizeof(buf));
    memset(value,0x0,sizeof(value));

    ret = recv_adbd2db(buf,msgid);
    if(ret <= 0)
    {
        ERROR("getValue buf fail\n");
        return -1;
    }

    ret = ExtractXmlContent(buf, value, xmlLen);
    if(ret == -1)
    {
        printf("get value fialure\n");
        return ret;
    }

    *extLen = strlen(buf) - (*xmlLen);
    strcpy(xmlBuffer, value);
    strncpy(extBuffer, buf + (*xmlLen), *extLen);

    DEBUG("xmlBuffer : %s\n", xmlBuffer);
    DEBUG("extBuffer : %s\n", extBuffer);

    return 1;
}

int getPacketBuff_fix(char *buf, char *xmlBuffer, char *extBuffer, int *xmlLen, int *extLen)
{
    int ret;
    char value[1024];

    memset(value,0x0,sizeof(value));

    ret = ExtractXmlContent(buf, value, xmlLen);
    if(ret == -1)
    {
        ERROR("get value fialure\n");
        return ret;
    }

    *extLen = strlen(buf) - (*xmlLen);
    strcpy(xmlBuffer, value);
    strncpy(extBuffer, buf + (*xmlLen), *extLen);

    DEBUG("xmlBuffer : %s\n", xmlBuffer);
    DEBUG("extBuffer : %s\n", extBuffer);

    return 1;
}


int sendStartRes2pc(int id)
{
    int ret = 0;
    struct devInfo dev;

    //GetDevInfo();
    strcpy(dev.device, "R16");

    ret = sendStartTestResponse(&dev, 1, "test sample", id);
    if(ret == -1)
    {
        DEBUG("send start response to pc failure\n");
        return ret;
    }

    return ret;
}

#if 0
int main()
{
    int ret = 0;
    printf("enter main\n");

    ret = init_entry();
    if(ret < 0)
    {
        printf("init entry fail  ret = %d\n",ret);
    }
    return 0;
}
#endif
