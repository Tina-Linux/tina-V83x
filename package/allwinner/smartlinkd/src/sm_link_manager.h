#ifndef __SM_LINK_MANAGER_H__
#define __SM_LINK_MANAGER_H__

#include <stdbool.h>

#define	SM_LINK_QUIT              (-1)

#define	SM_LINK _AP_COOEE         (1<<0)
#define	SM_LINK_AP_NEEZE          (1<<1)
#define	SM_LINK_AP_AIRKISS        (1<<2)

#define	SM_LINK_XR_AIRKISS        (1<<3)
#define	SM_LINK_XR_SMARTCONFIG    (1<<4)

#define	SM_LINK_SOUND_WAVE        (1<<5)
#define	SM_LINK_SOFTAP            (1<<6)

#define	SM_LINK_PROTO_MAX         (7)
#define SM_LINK_ALL               (8)


#define SSID_LEN  48
#define PWD_LEN   64

#define SM_EVENT_TIMEOUT_MS       (1000*180)


enum wifi_key {
	SM_LINK_NONE,
	SM_LINK_WEP,
	SM_LINK_WPA,
	SM_LINK_WPA2,
};

struct net_info {
	char ssid[SSID_LEN];
	char password[PWD_LEN];
	enum wifi_key key;
};

struct pro_feedback {
	int protocol;
	bool force_quit_sm;
};

struct proto_params {
	void **argv;
	int argc;
};

typedef int (*proto_cb)(struct pro_feedback *pb_info,bool is_success,struct net_info *info);
typedef int (*proto_free)(void *arg);

struct pro_worker {
	proto_cb cb;

	proto_free free_cb;
	void *free_cb_arg;

	struct proto_params *params;
	pthread_t thread;
	bool enable;
	//TODO;
	bool thread_running;
	int type;
};

enum smg_state {
	SMG_IDEL,
	SMG_RUNNING,
	SMG_OFF,
};

struct sm_link {
	struct pro_worker *pro_list;
	int num;
	bool quilt;
	enum smg_state state;
};

int sm_link_init(int protocol_num);

int sm_link_protocol_enable(int type,struct proto_params *p,int protocol_num);

int sm_link_wait_get_results(int type,struct net_info *info);

int sm_link_deinit();

void force_stop_protocol();

#endif /* __UTIL_H__ */
