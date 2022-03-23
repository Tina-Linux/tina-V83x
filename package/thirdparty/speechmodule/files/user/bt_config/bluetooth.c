#include "bluetooth.h"
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <errno.h>
#include "socket_app.h"
#include "msm_ble_adapter.h"
/* as same as APP_BLE_WIFI_INTRODUCER_GATT_ATTRIBUTE_SIZE */
#define BLE_SOCKET_RECV_LEN 23
#define BT_STATUS_PATH "/data/bsa/config/bt_config.xml"
static char sock_path[] = "/data/bsa/config/socket_data_interaction";
extern void msm_ble_stack_callback(ms_ble_stack_event_t event);


typedef enum Bt_CONTROL_TYPE {
    BT_NONE = 0,
    BLE_WIFI_INTRODUCER
}BtControlType;

typedef struct {
    pthread_t tid;
    int socket_recv_done;
    bool is_bt_open;
    bool is_ble_open;
    BtControlType type;
    tSOCKET_APP socket_app;
} bt_control_t;

static bt_control_t bt_control = {0, 0, false, false, BT_NONE};

static bool ble_is_close()
{
    return bt_control.is_bt_open;
}

static bool ble_is_open()
{
    return bt_control.is_ble_open;
}

static int bt_socket_send(void *data, int len)
{
    /*
        printf("bt_socket_send, len: %d\n", len);
        for(int i = 0; i < len; i++)
            printf("%02x ", ((char*)data)[i]);
        printf("\n\n");
    */
    return socket_send(bt_control.socket_app.client_sockfd, (char *) data, len);
}

//接收bsa ble socket 发送过来的数据, 并进行处理
static void *bt_socket_recieve(void *arg)
{
    char data[BLE_SOCKET_RECV_LEN];
    int bytes = 0;

    printf("bt_socket_recieve\n");

    if (-1 == system("mkdir -p /data/bsa/config")) {
        perror("mkdir /data/bsa/config failed\n");
        goto exit;
    }

    strcpy(bt_control.socket_app.sock_path, sock_path);

    if ((setup_socket_server(&bt_control.socket_app)) < 0) {
        goto exit;
    }

    if (accpet_client(&bt_control.socket_app) < 0) {
        goto exit;
    }

    printf("Client connected\n");

    while (bt_control.socket_recv_done) {
        memset(data, 0, sizeof(data));
        bytes = socket_recieve(bt_control.socket_app.client_sockfd, data, sizeof(data));
        ms_ble_load_to_ring_buf(data,bytes);

        if (bytes <= 0) {
            printf("Client leaved, break\n");
            break;
        }


        printf("bt_socket_recieve, bytes: %d\n", bytes);
        for(int i = 0; i < bytes; i++)
            printf("%02x ", (data)[i]);
        printf("\n\n");


        if (bt_control.type == BLE_WIFI_INTRODUCER) {
            //receive bsa ble socket data, do something.
        }
    }

exit:
    printf("Exit socket recv thread\n");
    return NULL;
}

void socket_recieve_handle(int signal, siginfo_t *siginfo, void *u_contxt)
{
    printf("socket_recieve_handle exec for kill\n");
    pthread_exit(NULL);
    return;
}

static int bt_socket_thread_create(void)
{
    struct sigaction sigact;

    printf("bt_socket_thread_create\n");

    if (!bt_control.socket_recv_done) {
        bt_control.socket_recv_done = 1;

        sigact.sa_sigaction = socket_recieve_handle;
        sigact.sa_flags = SA_SIGINFO;
        sigemptyset(&sigact.sa_mask);
        sigaction(SIGUSR2, &sigact, NULL);

        if (pthread_create(&bt_control.tid, NULL, bt_socket_recieve, NULL)) {
            printf("Create ble pthread failed\n");
            return -1;
        }
    }

    return 0;
}

static void bt_socket_thread_delete(void)
{
    int ret;
    printf("bt_socket_thread_delete\n");

    if (bt_control.socket_recv_done) {
        bt_control.socket_recv_done = 0;
        teardown_socket_server(&bt_control.socket_app);

        if (bt_control.tid) {
            ret = pthread_kill(bt_control.tid, SIGUSR2);

            if (ret == 0) {
                printf("pthread_kill success\n");
            } else if (ret == ESRCH) {
                printf("The id = %lu thread has exited or does not exist\n", bt_control.tid);
            } else {
                printf("pthread_kill error, ret: %d\n", ret);
                return;
            }

            pthread_join(bt_control.tid, NULL);
            bt_control.tid = 0;
        }
    }
}

static int bt_bsa_server_open(void)
{
    if (ble_is_close())
        return 0;

    if (-1 == system("bsa_server.sh start")) {
        printf("Start bsa server failed\n");
        return -1;
    }

    bt_control.is_bt_open = true;

    return 0;
}

static int bt_bsa_server_close(void)
{
    if (!ble_is_close())
        return 0;

    if (-1 == system("ble_wifi_introducer.sh stop")) {
        printf("Start bsa ble failed, errno: %d\n", errno);
        return -1;
    }

    if (-1 == system("bsa_server.sh stop")) {
        printf("Stop bsa server failed\n");
        return -1;
    }

    bt_control.is_bt_open = false;

    return 0;
}

static int ble_wifi_introducer_server_open(void)
{
    if (-1 == system("bsa_ble_wifi_introducer.sh start")) {
        printf("Start bsa ble failed, errno: %d\n", errno);
        return -1;
    }

    bt_control.is_ble_open = true;
    printf("ble_wifi_introducer_server_open\n");
    return 0;
}

static int ble_wifi_introducer_server_close(void)
{
    if (-1 == system("/usr/bin/bsa_ble_wifi_introducer.sh stop")) {
        printf("Stop bsa ble failed, errno: %d\n", errno);
        return -1;
    }

    bt_control.is_ble_open = false;
    printf("ble_wifi_introducer_server_close\n");

    return 0;
}

static int ble_open_server()
{
    int ret = 0;

    if (ble_is_open()) {
        return ret;
    }

    bt_control.type = BLE_WIFI_INTRODUCER;

    if (bt_socket_thread_create() < 0) {
        goto error;
    }

    if (ble_wifi_introducer_server_open() < 0) {
        goto error;
    }

    return ret;

error:
    bt_control.type = BT_NONE;
    return -1;
}

static int ble_close_server()
{
    int ret = 0;

    if (!ble_is_open()) {
        return ret;
    }

    bt_socket_thread_delete();

    if (ble_wifi_introducer_server_close() < 0) {
        ret = -1;
    }


    bt_control.type = BT_NONE;

    return ret;
}

int get_bt_mac(char *bt_mac)
{
    char reply[128] = {"\0"};
    FILE *fp = NULL;
    char *p = NULL;

    fp = fopen(BT_STATUS_PATH, "r");

    if (!fp) {
        printf("open get bt_mac file error!");
        return -1;
    }

    memset(reply, 0, sizeof(reply));

    while (fgets(reply, sizeof(reply), fp)) {
        p = strstr(reply, "<bd_addr>");

        if (!p) {
            continue;
        }

        p = p + strlen("<bd_addr>");
        memcpy(bt_mac, p, 17);
        bt_mac[17] = 0;
        p = NULL;
        break;
    }

    fclose(fp);

    printf("the bt mac : %s, len %d\n", bt_mac, strlen(bt_mac));

    return 0;
}

//void ble_stack_event_handler(ms_ble_stack_event_t event){
//	if(bt_control.is_ble_open){
//
//	}
//}
int bluetooth_control(BtControl cmd, void *data, int len)
{
    printf("controlBt, cmd: %d\n", cmd);

    int ret = 0;

    switch (cmd) {
    case BLE_IS_OPENED:
        ret = ble_is_open();
        break;

    case BLE_OPEN_SERVER: //启动ble配网
//        ret = bt_bsa_server_open();

//        if (!ret)
            ret = ble_open_server();
            if(ble_is_open()){
                msm_ble_stack_callback(MS_BLE_STACK_EVENT_STACK_READY);
//                msm_ble_stack_callback(MS_BLE_STACK_EVENT_ADV_OK);
//                msm_ble_stack_callback(MS_BLE_STACK_EVENT_CONNECTED);
            }
        break;

    case BLE_CLOSE_SERVER: //关闭ble配网
        ret = ble_close_server();

       // if (!ret)
       //     ret = bt_bsa_server_close();
        if(ble_is_close()){
		msm_ble_stack_callback(MS_BLE_STACK_EVENT_DISCONNECT);
            msm_ble_stack_callback(MS_BLE_STACK_EVENT_STACK_FAIL);
        }
        break;

    case GET_BT_MAC: //获取蓝牙mac地址
        if (get_bt_mac((char *)data) <= 0)
            ret = -1;

        break;

    case BLE_SERVER_SEND: //通过socket给bsa ble 发送数据
        if (bt_socket_send(data, len) < 0) {
            printf("Bt socket send data failed\n");
            ret = -1;
        }

        break;

    default:
        printf("cmd <%d> is not implemented.\n",cmd);
        break;
    }

    return ret;
}
