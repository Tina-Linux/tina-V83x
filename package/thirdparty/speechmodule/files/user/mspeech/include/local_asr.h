#ifndef __LOCAL_ASR_H__
#define __LOCAL_ASR_H__
#include <pthread.h>
#include "kfifo.h"
#include "kmsg.h"
#include <stdbool.h>
#include <semaphore.h>


#define LOCAL_ASR_CONFIRM    0.60   //matched Threshold
//#define LOCAL_ASR_CONFIRM    0.55   //matched Thresholdgrammar_fsmnfix_res_kaoxiang_opt_v4 should be set to 0.55


//Voice Threshold
//Between this thresthold and LOCAL_ASR_CONFIRM will offer retry record.
//Between this thresthold and LOCAL_ASR_NOISE will reply error.
//Disable-0.60, Enable-0.51(reference).
//Disable this because of the unreliable conf value.
#define LOCAL_ASR_NOVOICE    0.60

//Noise Threshold, under this threshold reply nothing.
//Disable-0, Enable-0.40(reference).
//Disable this to avoid none reply, and noise can be filtered by retanscation
#define LOCAL_ASR_NOISE      0

#define MAX_RECORD_NUM 2

enum{
    LOCAL_FAIL_STILLNESS=0x0a,
	LOCAL_FAIL_NOISE,
	LOCAL_FAIL_VOICE,
	LOCAL_FAIL_RECNUMOUT
};

struct localcmd{
	void *device;
	bool isfull;
	bool hasreceived;
	int cmdtype;

};
struct localasr_proc {

    void *asr;
    struct localcmd cmd;
    pthread_t tid;
	unsigned short sessionid;
	unsigned short recordid;
	unsigned short recordiddo;
	char wknum;
    int running;
    void *user;
    int isstart;
	sem_t asrsem;
	void *userasr;
	char *config;
    struct kfifo *in;
    struct kfifo *out;
    struct kmsg *msg;

    FILE *local_in_fd;
    int debug_save_audio;
    int lmax_save_files;
    int lcur_file_num;
};
typedef int(*LocalCallBack)(void *user_data, int type, char *msg, int len);
typedef int(*LocalFailBack)(void *user_data, int type);


int local_asr_changfirst(struct localasr_proc *proc);
int local_asr_init(char *config,struct localasr_proc *proc);
int local_asrfirst_init(struct localasr_proc *proc);
int local_asr_release(struct localasr_proc *proc);
int local_asr_start(struct localasr_proc *proc);
int local_asr_restart(struct localasr_proc *proc);
int local_asr_stop(struct localasr_proc *proc);


void local_asr_send_reset(struct localasr_proc *proc);
int local_asr_register(LocalCallBack result,LocalFailBack fail);

#endif
