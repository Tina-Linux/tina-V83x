#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#include <thread.h>
#include <locker.h>

//those macro only use for class interface and it's subclass,
//because of the m_socketname member
#define ipc_loge(fmt, ...) TLOGE("(%s)"fmt, m_socketname, ## __VA_ARGS__);
#define ipc_logd(fmt, ...) TLOGD("(%s)"fmt, m_socketname, ## __VA_ARGS__);
#define ipc_logi(fmt, ...) TLOGI("(%s)"fmt, m_socketname, ## __VA_ARGS__);
#define ipc_logw(fmt, ...) TLOGW("(%s)"fmt, m_socketname, ## __VA_ARGS__);
#define ipc_logv(fmt, ...) TLOGV("(%s)"fmt, m_socketname, ## __VA_ARGS__);

enum {
    TRANSACT_CODE_BASE = 0x0,
    USER_CODE_BASE = 0x20,
};

typedef struct data_t {
    void *buf;
    int len;
}data_t;

typedef struct request_t {
    unsigned char code;
    int client_handle;
}request_t;

typedef struct buf_t {
    unsigned char code;
    data_t data;
}buf_t;

class interface : public Thread
{
public:

    enum {
        TRANSACT_CODE_ERR = TRANSACT_CODE_BASE+0x0,
        TRANSACT_CODE_SERVER_CLOSE,
        TRANSACT_CODE_CLIENT_CONNECT,
        TRANSACT_CODE_CLIENT_DISCONNECT,
    };

public:
    interface();
    virtual ~interface(){};

    int init();

public:
    virtual void onTransact(request_t *request, data_t *data) = 0;
    virtual int transact(request_t *request, data_t *data) = 0;

protected:
    virtual void setSocketName(const char *name);

    virtual int loop() = 0;
    virtual int initSocket() = 0;
    virtual int read(int fd,char *buf, int size) = 0;

    int m_socketfd;
    int m_epollfd;
    int m_clientfd;
    char m_socketname[100];
    locker m_transact_lock;
};

class s_interface:public interface {
public:
    virtual ~s_interface(){};
    virtual void onTransact(request_t *request, data_t *data) = 0;
    int transact(request_t *request, data_t *data);

protected:
    virtual int initSocket();
    virtual int loop();
    virtual int read(int fd,char *buf, int size);



};

class c_interface:public interface {
public:
    virtual ~c_interface(){};
    virtual void onTransact(request_t *request, data_t *data) = 0;
    int transact(request_t *request, data_t *data);
protected:
    virtual int initSocket();
    virtual int loop();
    virtual int read(int fd,char *buf, int size);

};
#endif /*__INTERFACE_H__*/
