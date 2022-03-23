/*
 *  Copyright (c) 2019 Allwinner Technology. All Rights Reserved.
 *
 *  Author: laumy <liumingyuan@allwinnertech.com>
 *
 *  date: 2019-3-15
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sm_link_manager.h>
#include <smg_log.h>
#include <string.h>

#ifdef ADT
#include <sound_wave.h>
#endif

#ifdef XRADIO_AIRKISS
#include <xradio_airkiss.h>
#endif

#ifdef XRADIO_SMARTCONFIG
#include <xradio_smartconfig.h>
#endif

#ifdef SOFT_AP
#include <sm_soft_ap.h>
#endif

#include <errno.h>
#include <smg_event.h>

#define VERSION ("3.0.0(19-3-16)")

#define PTHREAD_CLEANUP(f) ((void (*)(void *))(f))

typedef void* (*proto_main_loop)(void *arg);

struct pro_worker pro_workers[SM_LINK_PROTO_MAX];

struct sm_link sm_link_worker = {
	.pro_list = NULL,
	.num = 0,
	.state = SMG_OFF,
};

static void _protocol_main_loop_resource_free(void *arg)
{
	struct pro_worker *worker = (struct pro_worker *)arg;

	smg_printf(SMG_MSGDUMP,"protocol type: %d\n",worker->type);

	if(NULL != worker->free_cb) {
		smg_printf(SMG_DEBUG,"worker callback function\n");
		worker->free_cb(worker->free_cb_arg);
	}

	//TODO;
	worker->thread_running = false;

	smg_printf(SMG_MSGDUMP,"quilt, type: %d\n",worker->type);
}


static void* _ap_airkiss_main_loop(void *arg)
{
	smg_printf(SMG_MSGDUMP,"enter -+-\n");

	//TODO;

	smg_printf(SMG_MSGDUMP,"quilt -+-\n");
}

static void* _ap_cooee_main_loop(void *arg)
{
	smg_printf(SMG_MSGDUMP,"enter -+-\n");
	//TODO;
	smg_printf(SMG_MSGDUMP,"quilt -+-\n");
}

static void* _ap_neeze_main_loop(void *arg)
{
	smg_printf(SMG_MSGDUMP,"enter -+-\n");
	//TODO;
	smg_printf(SMG_MSGDUMP,"quilt -+-\n");
}

static void* _xr_smconfig_main_loop(void *arg)
{
	smg_printf(SMG_MSGDUMP,"enter-+-: pid:%lu tid :%lu \n",
			(unsigned long)getpid(),(unsigned long)pthread_self);
#ifdef XRADIO_SMARTCONFIG
	smg_printf(SMG_DEBUG,"start xradio smartconfig protocol\n");

	pthread_cleanup_push(PTHREAD_CLEANUP(_protocol_main_loop_resource_free),arg);

	xradio_smartconfig_protocol(arg);

	pthread_cleanup_pop(1);
#else
	struct pro_feedback info;
	struct pro_worker *worker = (struct pro_worker *)arg;
	info.force_quit_sm = false;
	info.protocol = worker->type;
	worker->cb(&info,false,NULL);
#endif
	smg_printf(SMG_MSGDUMP,"quilt -+-\n");

}

static void* _xr_airkiss_main_loop(void *arg)
{
	smg_printf(SMG_MSGDUMP,"enter-+-: pid:%lu tid :%lu \n",
			(unsigned long)getpid(),(unsigned long)pthread_self);
#ifdef XRADIO_AIRKISS
	smg_printf(SMG_DEBUG,"start xradio airkiss protocol\n");

	pthread_cleanup_push(PTHREAD_CLEANUP(_protocol_main_loop_resource_free),arg);

	xradio_airkiss_protocol(arg);

	pthread_cleanup_pop(1);
#else
	struct pro_feedback info;
	struct pro_worker *worker = (struct pro_worker *)arg;
	info.force_quit_sm = false;
	info.protocol = worker->type;
	worker->cb(&info,false,NULL);
#endif
	smg_printf(SMG_MSGDUMP,"quilt -+-\n");
	return NULL;
}

static void* _soft_ap_main_loop(void *arg)
{
	smg_printf(SMG_MSGDUMP,"enter: pid:%lu tid :%lu\n",
			(unsigned long)getpid(),(unsigned long)pthread_self);
#ifdef SOFT_AP

	pthread_cleanup_push(PTHREAD_CLEANUP(_protocol_main_loop_resource_free),arg);

	smg_printf(SMG_DEBUG,"start soft ap protocol\n");

	soft_ap_protocol(arg);

	pthread_cleanup_pop(1);
#else
	struct pro_feedback info;
	struct pro_worker *worker = (struct pro_worker *)arg;
	info.force_quit_sm = false;
	info.protocol = worker->type;
	worker->cb(&info,false,NULL);
#endif
	smg_printf(SMG_MSGDUMP,"quilt -+-\n");
	return NULL;
}

static void* _sound_wave_main_loop(void *arg)
{
	int ret = -1;

	smg_printf(SMG_MSGDUMP,"enter: pid:%lu tid :%lu\n",
			(unsigned long)getpid(),(unsigned long)pthread_self);
#ifdef ADT
	smg_printf(SMG_DEBUG,"start sound wave protocol\n");

	pthread_cleanup_push(PTHREAD_CLEANUP(_protocol_main_loop_resource_free),arg);

	sound_wave_protocol(arg);

	pthread_cleanup_pop(1);
#else
	struct pro_feedback info;
	struct pro_worker *worker = (struct pro_worker *)arg;
	info.force_quit_sm = false;
	info.protocol = worker->type;
	worker->cb(&info,false,NULL);
#endif
	return NULL;
}

proto_main_loop main_loop[SM_LINK_PROTO_MAX] = {
	_ap_cooee_main_loop,
	_ap_neeze_main_loop,
	_ap_airkiss_main_loop,
	_xr_airkiss_main_loop,
	_xr_smconfig_main_loop,
	_sound_wave_main_loop,
	_soft_ap_main_loop,
};


static int _int_log2 (unsigned int x)
{
    int l = -1;
    const unsigned char log_2[256] = {
        0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
        6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8
    };
    while (x >= 256) { l += 8; x >>= 8; }

    return l + log_2[x];
}

static struct net_info _net_info;

static int _sm_link_cb(struct pro_feedback *pb_info,bool is_success,struct net_info *info)
{
	smg_printf(SMG_INFO,"protocol %d\n",pb_info->protocol);
	if(is_success) {
		smg_printf(SMG_INFO,"receive success\n");
		strcpy(_net_info.ssid,info->ssid);
		strcpy(_net_info.password,info->password);
		sm_link_worker.quilt = true;
	} else {
		smg_printf(SMG_INFO,"receive failed\n");
		if(pb_info->force_quit_sm)
			sm_link_worker.quilt = true;
		else
			sm_link_worker.quilt = false;
	}
	sm_event_notice(pb_info->protocol);
	smg_printf(SMG_MSGDUMP,"quilt -+-\n");
	return 0;
}

static int _stop_proto_thread(struct pro_worker *pro)
{
	smg_printf(SMG_MSGDUMP,"enter -+-\n");
	if(pro->enable) {

		pthread_cancel(pro->thread);
		pro->enable = false;
		if(pthread_join(pro->thread,NULL) == -1) {
			smg_printf(SMG_ERROR,"failed to recollect\n");
		}

	}
	//TODO:
	while(pro->thread_running) {
		sleep(1);
	}

	smg_printf(SMG_MSGDUMP,"quilt -+-\n");
	return 0;
}

static int _start_proto_thread(struct pro_worker *pro)
{
	int ret = -1;
	int protocol_num = _int_log2(pro->type);

	smg_printf(SMG_DEBUG,"protocol number %d\n",protocol_num);

	pro->enable = false;
	pro->free_cb = NULL;
	pro->free_cb_arg = NULL;
	pro->thread_running = true;

	pthread_create(&pro->thread,NULL,main_loop[protocol_num],pro);
}

int sm_link_init(int protocol_num)
{
	smg_printf(SMG_MSGDUMP,"smartlink version:%s\n",VERSION);
	if(sm_link_worker.pro_list != NULL)
		free(sm_link_worker.pro_list);

	sm_link_worker.pro_list = (struct pro_worker *)malloc(sizeof(struct pro_worker) * protocol_num);
	if(sm_link_worker.pro_list == NULL) {
		smg_printf(SMG_ERROR,"sm_link init failed(%s)\n", strerror(errno));
		return -1;
	}

	sm_link_worker.num = protocol_num;

	sm_link_worker.quilt = false;

	sm_link_worker.state = SMG_IDEL;

	smg_printf(SMG_MSGDUMP,"quilt -+-\n");
	return 0;
}

int sm_link_deinit()
{
	if(sm_link_worker.pro_list = NULL) {
		free(sm_link_worker.pro_list);
	}

	sm_link_worker.state = SMG_OFF;
	return 0;
}
int sm_link_protocol_enable(int type,struct proto_params *p,int protocol_num)
{
	int i ;
	int cur_proto;
	int num= 0;

	if(sm_link_worker.state != SMG_IDEL) {
		smg_printf(SMG_ERROR,"smartlink is off or running state(%d)\n",sm_link_worker.state);
		return -1;
	}

	if(NULL == sm_link_worker.pro_list || sm_link_worker.num <= 0) {
		smg_printf(SMG_ERROR,"smartlink isn't initialized\n");
		return -1;
	}

	if(sm_event_init() < 0) {
		smg_printf(SMG_ERROR,"sm event init failed\n");
		return -1;
	}

	for(i=0;i< SM_LINK_PROTO_MAX ; i++) {

		cur_proto = type & (1<<i);

		if(cur_proto) {
			(sm_link_worker.pro_list+num)->params = p+num;
			(sm_link_worker.pro_list+num)->cb = _sm_link_cb;
			(sm_link_worker.pro_list+num)->type = cur_proto;

			(sm_link_worker.pro_list+num)->free_cb = NULL;
			(sm_link_worker.pro_list+num)->free_cb_arg = NULL;

			sm_link_worker.state = SMG_RUNNING;

			_start_proto_thread(sm_link_worker.pro_list+num);

			num++;
		}
	}
	return 0;
}

static int _sm_link_protocol_disable(int type)
{
	int i,j;
	int cur_proto;

	smg_printf(SMG_MSGDUMP,"enter -+-\n");
	sm_event_deinit();

	if(type == SM_LINK_PROTO_MAX) {
		for(i=0;i < sm_link_worker.num; i++) {
			if(sm_link_worker.pro_list+i)
				_stop_proto_thread(sm_link_worker.pro_list+i);
		}
	}else {
		for(i=0;i< SM_LINK_PROTO_MAX ; i++) {
			cur_proto = type & (1<<i);
			if(cur_proto) {
				for(j=0;j < sm_link_worker.num; j++) {
					if(sm_link_worker.pro_list+j) {
						if((sm_link_worker.pro_list+j)->type == cur_proto) {
							smg_printf(SMG_DEBUG,"stop :%d protocol thread\n",cur_proto);
							_stop_proto_thread(sm_link_worker.pro_list+j);
						}
					}
				}
			}
		}
	}
	sm_link_worker.state = SMG_IDEL;
	smg_printf(SMG_MSGDUMP,"quilt -+-\n");
}

static int _is_protocol_working()
{
	int i;
	int ret = -1;

	if(sm_link_worker.pro_list == NULL) {
		smg_printf(SMG_DEBUG,"dosn't protocol worker\n");
		return ret;
	}
	for(i=0;i< sm_link_worker.num; i++) {

		smg_printf(SMG_DEBUG,"protocol(%d),enable(%d)\n",(sm_link_worker.pro_list+i)->type,
				(sm_link_worker.pro_list+i)->enable);

		if((sm_link_worker.pro_list+i)->enable) {
			ret = (sm_link_worker.pro_list+i)->type;
			return ret;
		}
	}
	smg_printf(SMG_DEBUG,"not protocol working\n");
	return ret;
}

int sm_link_wait_get_results(int type,struct net_info *info)
{
	int ret = -1;

	smg_printf(SMG_MSGDUMP,"enter -+-\n");
	if(sm_link_worker.state != SMG_RUNNING) {
		smg_printf(SMG_ERROR,"sm link dosn't protocol running\n");
		return -1;
	}
	while(1) {
		smg_printf(SMG_DEBUG,"waiting receive......\n");
		ret = sm_event_wait(SM_EVENT_TIMEOUT_MS);
		if(ret == 0) {
			smg_printf(SMG_ERROR,"wait receive timeout\n");
			break;
		}
		if(sm_link_worker.quilt) {
			break;
		}
		if((ret = _is_protocol_working()) < 0)
			break;
		else {
			smg_printf(SMG_DEBUG,"protocol(%d) is working now\n",ret);
		}
	}

	if(sm_link_worker.quilt) {
		strcpy(info->ssid,_net_info.ssid);
		strcpy(info->password,_net_info.password);
	}

	_sm_link_protocol_disable(type);

	smg_printf(SMG_MSGDUMP,"quilt -+-\n");
	return ret;
}

void force_stop_protocol()
{
	struct pro_feedback info = {
		.protocol = SM_LINK_QUIT,
		.force_quit_sm = true,
	};

	_sm_link_cb(&info,true,NULL);
}
