#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "ms_common.h"


//#define DEAMON_WDG
#ifdef DEAMON_WDG
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/watchdog.h>
int wd_fd;
#endif


enum DEAMON_STATUS{
	IGNORANT = 0,

}deamon_status;

pid_t deamon_pid = 0;

pid_t uart_pid = 0;

pid_t local_net_pid = 0;

pid_t net_pid = 0;

pid_t mspeech_pid = 0;

pid_t bt_config_pid = 0;

bool dev_ready =false;
bool wifi_ready =false;
bool stat_change =false;
bool mspeech_stop = false;

#ifdef DEAMON_WDG
const char v = 'V';
static void wdg_init(){

	int flags;
	if (-1==(wd_fd= open("/dev/watchdog", O_WRONLY))) {
		MS_DBG_TRACE("Watchdog device not enabled.\n");
		exit(-1);
	}
    flags = WDIOS_ENABLECARD;
    ioctl(wd_fd, WDIOC_SETOPTIONS, &flags);
    ioctl(wd_fd, WDIOC_SETTIMEOUT, 10);
}
static void wdg_keep_alive(void)
{
    int dummy;
    ioctl(wd_fd, WDIOC_KEEPALIVE, &dummy);
}


static void wdg_term()
{
    int ret = write(wd_fd, &v, 1);

    close(wd_fd);
    if (ret < 0)
	MS_DBG_TRACE("\nStopping watchdog ticks failed (%d)...\n", errno);
    else
	MS_DBG_TRACE("\nStopping watchdog ticks...\n");
    exit(0);
}
#endif


void sigaction_process(int nsig)
{
	//================================================================
	//TODO:ADD YOU CODE
    MS_DBG_TRACE("signal:%d\n",nsig);
	//================================================================
#if 1 //add by 926 @20190617
    if (nsig == SIG_SYS_EVENT_DEV_READY) {
	dev_ready = true;
    }
    #ifdef DEAMON_WDG
    else if (nsig == SIG_UART_EVENT_WDG) {
	wdg_term();
    }
    #endif
    else if (nsig == SIG_SYS_EVENT_WIFI_READY) {
	wifi_ready = true;
    }
    else if (nsig == SIG_SYS_EVENT_SYSTEM_STATUS_CHANGE) {
	MS_DBG_TRACE("state change\n");
	stat_change = true;
    }
    else if (nsig == SIG_SYS_EVENT_STOP_MSPEECH) {
	MS_DBG_TRACE("stop mspeech");
	mspeech_stop = true;
	if(mspeech_pid > 0) {
	    kill(mspeech_pid, SIGKILL);  //SIGINT can not kill mspeech process, so use SIGKILL
	}
    }
     else if (nsig ==  SIG_SYS_EVENT_START_MSPEECH) {
	MS_DBG_TRACE("start mspeech");
	mspeech_stop = false;
    }
#else
    switch(nsig){
    case SIG_UART_EVENT_WDG:
	wdg_term();
	break;
    case SIG_SYS_EVENT_DEV_READY:
	dev_ready = true;
	break;
    case SIG_SYS_EVENT_WIFI_READY:
	wifi_ready = true;
	break;
    case SIG_SYS_EVENT_SYSTEM_STATUS_CHANGE:
	MS_DBG_TRACE("state change\n");
	stat_change = true;
	break;
	case SIG_SYS_EVENT_STOP_MSPEECH: {
		MS_DBG_TRACE("stop mspeech");
		mspeech_stop = true;
	if(mspeech_pid > 0) {
		kill(mspeech_pid, SIGKILL);  //SIGINT can not kill mspeech process, so use SIGKILL
		}
			break;
		}
		case SIG_SYS_EVENT_START_MSPEECH:{
			MS_DBG_TRACE("start mspeech");
			mspeech_stop = false;
		break;
		}
    }
#endif
}

void    sigaction_uart_wdg(void)
{
	struct sigaction act,oldact;

	act.sa_handler  = sigaction_process;
	act.sa_flags = 0;
	sigaction(SIG_UART_EVENT_WDG,&act,&oldact);
}

void    sigaction_system_ready(void)
{
	struct sigaction act,oldact;

	act.sa_handler  = sigaction_process;
	act.sa_flags = 0;
	sigaction(SIG_SYS_EVENT_DEV_READY,&act,&oldact);
}

void    sigaction_wifi_ready(void)
{
	struct sigaction act,oldact;

	act.sa_handler  = sigaction_process;
	act.sa_flags = 0;
	sigaction(SIG_SYS_EVENT_WIFI_READY,&act,&oldact);
}

void  sigaction_system_status_change(void)
{
	struct sigaction act,oldact;

	act.sa_handler  = sigaction_process;
	act.sa_flags = 0;
	sigaction(SIG_SYS_EVENT_SYSTEM_STATUS_CHANGE,&act,&oldact);
}

void sigaction_stop_mspeech(void)
{
	struct sigaction act,oldact;

	act.sa_handler	= sigaction_process;
	act.sa_flags = 0;
	sigaction(SIG_SYS_EVENT_STOP_MSPEECH,&act,&oldact);
}

void sigaction_start_mspeech(void)
{
	struct sigaction act,oldact;

	act.sa_handler	= sigaction_process;
	act.sa_flags = 0;
	sigaction(SIG_SYS_EVENT_START_MSPEECH,&act,&oldact);
}

pid_t exe_proc(char *path){
	char tmp[8];
	char *argv[3]= {path,tmp,(char *)0};;

	pid_t cpid = fork();


	if(cpid == 0){
		umask(0);
		sprintf(tmp,"%d",deamon_pid);
	    execv(path,argv);
	}
	 return cpid;
}

int main()
{
        int uart_exitcode=0,mspeech_exitcode=0,net_exitcode = 0,local_net_exitcode = 0,ble_exitcode = 0;
        long net_ret = 0,local_net_ret = 0, uart_ret = 0,mspeech_ret = 0;
        pid_t pri_bt_config_pid = 0;

#ifdef DEAMON_WDG
        wdg_init();
#endif
        signal(SIGTTOU,SIG_IGN);
        signal(SIGTTIN,SIG_IGN);
        signal(SIGTSTP,SIG_IGN);
        signal(SIGHUP,SIG_IGN);

#ifdef DEAMON_WDG
        sigaction_uart_wdg();
#endif
        sigaction_system_ready();
        sigaction_wifi_ready();
        sigaction_system_status_change();
        sigaction_stop_mspeech();
        sigaction_start_mspeech();
		//remove("/data/cfg/.mspeech_silence_reboot");
		remove("/mnt/app/cfg/.mspeech_silence_reboot");
        deamon_pid = getpid();


        //uart_pid = exe_proc("/oem/msmart_uart");
        uart_pid = exe_proc("/mnt/app/msmart_uart");
	MS_DBG_TRACE("start uart process:%d\n",uart_pid);

        //local_net_pid = exe_proc("/oem/msmart_local_net");
        local_net_pid = exe_proc("/mnt/app/msmart_local_net");
	MS_DBG_TRACE("start local net process:%d\n",local_net_pid);
        //MS_TRACE("play on init, url: /oem/tts/network_config_connecting.pcm");
        //system("aplay /oem/tts/network_config_connecting.pcm &");
        //mspeech_pid = exe_proc("/oem/mspeech");
        mspeech_pid = exe_proc("/mnt/app/mspeech");
	MS_DBG_TRACE("start mspeech process:%d\n",mspeech_pid);

        //net_pid = exe_proc("/oem/msmart_net");
	net_pid = exe_proc("/mnt/app/msmart_net");
	MS_DBG_TRACE("start net process:%d\n",net_pid);

        MS_DBG_TRACE("WDG:%d,SYSTEM_READY:%d,,WIFI_READY:%d,STATUS_CHANGE:%d",SIG_UART_EVENT_WDG,SIG_SYS_EVENT_DEV_READY,SIG_SYS_EVENT_WIFI_READY,SIG_SYS_EVENT_SYSTEM_STATUS_CHANGE);

        while(1)
        {

            uart_ret = waitpid(uart_pid,&uart_exitcode,WNOHANG);

            local_net_ret = waitpid(local_net_pid,&local_net_exitcode,WNOHANG);
			if(!mspeech_stop){
				mspeech_ret = waitpid(mspeech_pid,&mspeech_exitcode,WNOHANG);
			}

            net_ret = waitpid(net_pid,&net_exitcode,WNOHANG);

            waitpid(pri_bt_config_pid, &ble_exitcode, WNOHANG); //Recovering zombie process of bluetooth

            sleep(1);
#ifdef DEAMON_WDG
            wdg_keep_alive();
#endif
            if(0 != uart_ret){
		//uart_pid = exe_proc("/oem/msmart_uart");
		uart_pid = exe_proc("/mnt/app/msmart_uart");
		MS_DBG_TRACE("Restart uart process:%d\n",uart_pid);
            }
            if(0 != local_net_ret){
		//local_net_pid = exe_proc("/oem/msmart_local_net");
		local_net_pid = exe_proc("/mnt/app/msmart_local_net");
		MS_DBG_TRACE("Restart local net process:%d\n",local_net_pid);
            }

			if(!mspeech_stop){
	            if(0 != mspeech_ret){
					//FILE * fd = fopen("/data/cfg/.mspeech_silence_reboot", "wb+");
					FILE * fd = fopen("/mnt/app/cfg/.mspeech_silence_reboot", "wb+");
					fclose(fd);
					sync();
			//mspeech_pid = exe_proc("/oem/mspeech");
			mspeech_pid = exe_proc("/mnt/app/mspeech");
			MS_DBG_TRACE("Restart mspeech process:%d\n",mspeech_pid);
	            }
			}

             if(0 != net_ret){
		//net_pid = exe_proc("/oem/msmart_net");
		net_pid = exe_proc("/mnt/app/msmart_net");
		MS_DBG_TRACE("Restart net process:%d\n",net_pid);
            }
            if(stat_change == true){
                // clear the existing bt process
                if(bt_config_pid > 0){
                    pri_bt_config_pid = bt_config_pid;
                    kill(bt_config_pid,SIGKILL);
                    bt_config_pid = 0;
                    sleep(4);
                }

                if(bt_config_pid <= 0){
                    //bt_config_pid = exe_proc("/oem/msmart_bt_config");
		   bt_config_pid = exe_proc("/mnt/app/msmart_bt_config");
                }

		stat_change =false;
//		MS_DBG_TRACE("statue change,restart net ,localnet and mspeech");
//		if(net_pid>0)
//		{
//			kill(net_pid,SIGINT);
//		}
		if(local_net_pid>0)
		{
			kill(local_net_pid,SIGINT);
		}
          /*	if(mspeech_pid>0)
		{
			kill(mspeech_pid, SIGKILL);  //SIGINT can not kill mspeech process, so use SIGKILL
		}
		*/
		sleep(1);
            }
            if(wifi_ready == true){
		wifi_ready = false;
//		if(net_pid>0)
//		{
//			kill(net_pid,SIGINT);
//		}
		if(local_net_pid>0)
		{
			kill(local_net_pid,SIGINT);
		}

                /* kill msmart_bt_config process when network configuration finish */
                if(bt_config_pid > 0)
                {
                    //kill(bt_config_pid, SIGTERM);
                    pri_bt_config_pid = bt_config_pid;
                    bt_config_pid = 0;
                }

                sleep(1);
            }

        }

        MS_DBG_TRACE(" success for wait for child to exit. \r\n");
#ifdef DEAMON_WDG
        wdg_term();
#endif
        return 0;
}
