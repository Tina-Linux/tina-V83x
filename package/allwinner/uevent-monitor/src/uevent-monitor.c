#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>

#include <sys/socket.h>
#include <linux/netlink.h>
#include <sys/epoll.h>
#include <sys/mount.h>

#include <ev.h>

#if 0
#define DLOG(fmt, arg...)		printf("[%s:%u] "fmt"", __FUNCTION__, __LINE__, ##arg)
#else
#define DLOG(fmt, arg...)
#endif


/*#define FUNCTION_SELECT "mass_storage"*/
#define FUNCTION_SELECT "mass_storage,adb"

static char gSDCardDevPath[64] = {0};
static struct timeval gSysBootTime;
static char gSysStartWithSD = 0;
static char gSysStartWithUSB = 0;

static struct UsbState {
	int IgnoreUsbState;
	int UsbConnected;
}gUsbState;


static struct uevent_ev{
	int sockfd[2];
	struct ev_loop *loop;
	ev_timer usb_handle;
	ev_timer sdcard_handle;
	int usb_ev_state;
	int sdcard_ev_state;
}gEv;


#define E_TRUE		(1)
#define E_FALSE     	(0)

#define STATE_USB_CONNECT	"usb_connect"
#define STATE_USB_DISCONNECT	"usb_disconnect"
#define STATE_SDCARD_CONNECT	"sdcard_connect"
#define STATE_SDCARD_DISCONNECT "sdcard_disconnect"

#define USB_UPDATE_STATE_TIMEOUT	(0.8)//(2.0)
#define SDCARD_UPDATE_STATE_TIMEOUT	(0.8)//(2.0)

enum {
	EV_STATE_CONNECT = 1,
	EV_STATE_DISCONNECT = 2,
};
static void ev_send_cmd(const char *cmd);
static void mass_storage_enable(void);

static int sdcard_is_mount_correct(void)
{
	FILE *fp = NULL;
	char buf[256] = {0};
	char *p = NULL;
	int  ret = E_FALSE;

	fp = fopen("/etc/mtab", "r");
	if (fp == NULL) {
			fprintf(stderr, "open /etc/mtab file err\n");
			return -1;
	}
	memset(buf, 0, sizeof(buf));
	while(fgets(buf, sizeof(buf), fp) != NULL){
			p = strstr(buf, gSDCardDevPath);
			if(!p){
					memset(buf, 0, sizeof(buf));
					continue;
			}

			p = strstr(buf, "SDCARD");
			if(!p){
					memset(buf, 0, sizeof(buf));
					continue;
			}

			ret = E_TRUE;
			break;
	}

	fclose(fp);

	return ret;
}

static void set_ignore_usb_state(int value)
{
	DLOG("set gUsbState.IgnoreUsbState=%d\n", value);
	/* thread mutex */
	if ( value < 0)
		gUsbState.IgnoreUsbState = 0;
	else
		gUsbState.IgnoreUsbState = value;
}
static int get_ignore_usb_state(void)
{
	return gUsbState.IgnoreUsbState;
}
static void set_usb_state(int value)
{
	DLOG("set gUsbState.UsbState=%d\n", value);
	/* thread mutex */
	gUsbState.UsbConnected = value;
}
static int get_usb_state(void)
{
	return gUsbState.UsbConnected;
}

static int get_sdcard_state(void)
{
	if(access(gSDCardDevPath, F_OK) == 0)
		return 1;
	return 0;
}

static void usb_state_init()
{
	int ret, fd;
	char buf[32];

	fd = open("/sys/class/android_usb/android0/state", O_RDONLY);
	if (fd < 0)
		return;
	ret = read(fd, buf, sizeof(buf));
	if (strncmp(buf, "CONFIGURED", ret-1) == 0) {
		set_usb_state(1);
		gEv.usb_ev_state = EV_STATE_CONNECT;
	}
	close(fd);
}
#ifdef GADGET_CONFIGFS
static void mass_storage_file(const char *dev)
{
	int fd;
	char *path = "/sys/kernel/config/usb_gadget/g1/functions/mass_storage.usb0/lun.0/file";

	fd = open(path, O_RDWR);
	if (fd < 0)
		return;
	if (dev != NULL) {
		if (access(dev, F_OK) == 0)
			write(fd, dev, strlen(dev));
		close(fd);
	} else {
		char buf[16];
		ssize_t ret;

		memset(buf, 0, sizeof(buf));
		ret = read(fd, buf, sizeof(buf));
		buf[sizeof(buf)-1] = '\0';
		DLOG("read return %d, buf:%s\n", ret, buf);
		if (ret != 0) {
			mass_storage_enable();
			write(fd, "", 1);
		}
		close(fd);
	}
}
#else
static void mass_storage_file(const char *dev)
{
	int fd;
	char *path = "/sys/class/android_usb/android0/f_mass_storage/lun/file";

	fd = open(path, O_WRONLY);
	if (fd < 0)
		return;
	if (dev != NULL) {
		if (access(dev, F_OK) == 0)
			write(fd, dev, strlen(dev));
	} else {
		write(fd, "", 1);
	}
	close(fd);
}
#endif

static void mass_storage_enable(void)
{
	if (access("/mnt/SDCARD", F_OK) != 0)
		mkdir("/mnt/SDCARD", 0644);

	/* source:package/base-files/setusbconfig */
	system("setusbconfig "FUNCTION_SELECT);
}

static void perform_handler(void *(*ptr)(void *), void *arg)
{
#if 1
	pthread_t tid;
	int status = pthread_create(&tid, NULL, ptr, arg);
	if(status != 0) {
	  perror("pthread_create error");
	}
	pthread_detach(tid);
#else
	pid_t pid;
	pid = fork();
	if (pid == 0) {
		usleep(50*1000);
		(ptr)(arg);
		exit(0);
	}
#endif
	return;
}

static void add_dev(const char *path)
{
	int fd;
	fd = open(path, O_WRONLY);
	if (fd < 0)
		return;
	write(fd, "add", 3);
	close(fd);
	return ;
}

static void sdcard_init()
{
	char path[128];

	strcpy(path, "/sys/block/mmcblk0/mmcblk0p1/uevent");
	if(access(path, F_OK) == 0) {
		add_dev(path);
		strcpy(gSDCardDevPath, "/dev/mmcblk0p1");
		return;
	}
	strcpy(path, "/sys/block/mmcblk0/uevent");
	if(access(path, F_OK) == 0) {
		add_dev(path);
		strcpy(gSDCardDevPath, "/dev/mmcblk0");
	}
	return;
}

static int uevent_monitor_open_socket(void)
{
        struct sockaddr_nl nls;
        int nlbufsize = 64 * 1024;
        int s;

        memset(&nls,0,sizeof(struct sockaddr_nl));
        nls.nl_family = AF_NETLINK;
        nls.nl_pid = getpid();
        nls.nl_groups = -1;

        if ((s = socket(PF_NETLINK, SOCK_DGRAM | SOCK_CLOEXEC, NETLINK_KOBJECT_UEVENT)) == -1) {
                fprintf(stderr, "Failed to open hotplug socket: %s\n", strerror(errno));
                return -1;
        }

        if (setsockopt(s, SOL_SOCKET, SO_RCVBUFFORCE, &nlbufsize, sizeof(nlbufsize))) {
                fprintf(stderr, "Failed to resize receive buffer: %s\n", strerror(errno));
                close(s);
                return -1;
        }

        if (bind(s, (void *)&nls, sizeof(struct sockaddr_nl))) {
                fprintf(stderr, "Failed to bind hotplug socket: %s\n", strerror(errno));
                close(s);
                return -1;
        }

	fcntl(s, F_SETFL, O_NONBLOCK);

        return s;
}


typedef struct {
	char action[16];	/* ACTION */
	char subsystem[32];	/* SUBSYSTEM */
	char devtype[16];	/* DEVTYPE */
	char usb_state[16];	/* USB_STATE */
	char partn[4];		/* PART NUM*/
	char devpath[256];	/* DEVPATH */
}uevent_t;

#define UEVENT_MSG_LEN 1024
#define UEVENT_ACTION		"ACTION"
#define UEVENT_SUBSYSTEM	"SUBSYSTEM"
#define UEVENT_DEVTYPE		"DEVTYPE"
#define UEVENT_USBSTATE		"USB_STATE"
#define UEVENT_PARTN		"PARTN"
#define UEVENT_DEVPATH		"DEVPATH"

/*
 *	1. ACTION=change
 *	   SUBSYSTEM=android_usb
 *	   USB_STATE=CONNECTED
 *	2. ACTION=change
 *	   SUBSYSTEM=android_usb
 *	   USB_STATE=DISCONNECTED
 *	3. ACTION=add
 *	   SUBSYSTEM=block
 *	   DEVTYPE=partition
 *	4. ACTION=remove
 *	   SUBSYSTEM=block
 *	   DEVTYPE=partition
 *
 * */

static void *usb_connect_with_card_func(void *arg)
{
	DLOG("");
	mass_storage_file((char *)arg);
	DLOG("umount SDCARD!\n");
	umount("/mnt/SDCARD");
}

static void *usb_connect_without_card_func(void *arg)
{
	DLOG("");
	mass_storage_file(NULL);
}

static void *usb_disconnect_func(void *arg)
{
	DLOG("");
	mass_storage_file(NULL);
	if(access(gSDCardDevPath, F_OK) == 0) {
		DLOG("mount SDCARD [%s]!\n", (char *)arg);
		if (E_TRUE == sdcard_is_mount_correct())
			umount("/mnt/SDCARD");
		mount((char *)arg, "/mnt/SDCARD", "vfat", MS_NODEV | MS_NOEXEC | MS_NOSUID | MS_SYNCHRONOUS, "utf8");
	}
}

static void *udisk_connect_with_usb_func(void *dev)
{
	mass_storage_file((const char *)dev);
	if (E_TRUE == sdcard_is_mount_correct()) {
		DLOG("umount SDCARD!\n");
		umount("/mnt/SDCARD");
	}
	if (!gSysStartWithSD){

		printf("play tf plugin music(1).\n");
	}else{
		gSysStartWithSD = 0;
	}
}
static void *udisk_connect_without_usb_func(void *arg)
{
	DLOG("\n");
	mass_storage_file(NULL);

	if (!gSysStartWithSD){
		if (E_TRUE == sdcard_is_mount_correct())
			umount("/mnt/SDCARD");
		DLOG("mount SDCARD [%s]!\n", (char *)arg);
		mount((char *)arg, "/mnt/SDCARD", "vfat", MS_NODEV | MS_NOEXEC | MS_NOSUID | MS_SYNCHRONOUS, "utf8");
		printf("play tf plugin music(2).\n");
	}else{
		/* boot system with SD card, will be mounted in mdev, so no need mount here */
		gSysStartWithSD = 0;
	}
}
static void *udisk_disconnect_func(void *arg)
{
	DLOG("\n");
	mass_storage_file(NULL);
	if (E_TRUE == sdcard_is_mount_correct()) {
		DLOG("umount SDCARD!\n");
		umount("/mnt/SDCARD");
	}
	printf("play tf plugout music.\n");
}

static void uevent_dispatch(uevent_t *event)
{
	if (!strcmp(event->subsystem, "android_usb")) {
		if (!strcmp(event->usb_state, "CONNECTED")) {
			/* send usb connect command */
			set_usb_state(1);
			ev_send_cmd(STATE_USB_CONNECT);
		} else if (!strcmp(event->usb_state, "DISCONNECTED")) {
			/* send usb disconnect command */
			set_usb_state(0);
			ev_send_cmd(STATE_USB_DISCONNECT);
		}
	} else if (!strcmp(event->subsystem, "block")) {
		DLOG("subsystem:%s devtype:%s action:%s\n", event->subsystem, event->devtype, event->action);

		if (!strcmp(event->action, "add")) {
			if (!strcmp(event->devtype, "partition")) {
				char *p = NULL;

				if (atoi(event->partn) != 1)
					return;
				p = strrchr(event->devpath, '/');
				if (!p)
					return;
				snprintf(gSDCardDevPath, sizeof(gSDCardDevPath), "/dev/%s", p+1);
			} else if (!strcmp(event->devtype, "disk")) {
				char path[256];
				char *p = NULL;

				p = strrchr(event->devpath, '/');
				if (!p)
					return ;
				snprintf(path, sizeof(path), "%s/%sp1", event->devpath, p+1);
				if(access(path, F_OK) == 0)
					return;
				snprintf(gSDCardDevPath, sizeof(gSDCardDevPath), "/dev/%s", p+1);
			}
			/* send sdcard connect command */
			ev_send_cmd(STATE_SDCARD_CONNECT);
		} else if (!strcmp(event->action, "remove")) {
			char *p = NULL, *p1 = NULL;

			p = strrchr(event->devpath, '/');
			if (!p)
				return;
			p1 = strrchr(gSDCardDevPath, '/');
			if (!p1)
				return;
			if (strncmp(p1+1, p+1, strlen(p1+1)) != 0)
				return;
			/* send sdcard disconnect command */
			ev_send_cmd(STATE_SDCARD_DISCONNECT);
		}
	}
	return ;
}

/*
 * uevent handler
 * read uevent msg from w->fd, and parse it.
 *
 * */
static void uevent_monitor_handler(EV_P_ struct ev_io *w, int revents)
{
	char buf[UEVENT_MSG_LEN];
	int len, i=0;
	uevent_t event;

	len = recv(w->fd, buf, sizeof(buf), MSG_DONTWAIT);
	if (len < 1)
		return ;
	buf[len] = '\0';

#if 0
{
        int l=0;
        printf("recv uevent:\n");
        while(l < len) {
                printf("%s\n", &buf[l]);
                l += strlen(&buf[l]) + 1;
        }
        printf("=========================\n");
}
#endif

	memset(&event, 0, sizeof(uevent_t));
	while (i < len) {
		int l;
		char *ptr = NULL;
		l = strlen(buf + i) + 1;
		ptr = strchr(&buf[i], '=');
		if (ptr != NULL) {
			if (!strncmp(&buf[i], UEVENT_ACTION, strlen(UEVENT_ACTION)))
				strcpy(event.action, ptr+1);
			else if (!strncmp(&buf[i], UEVENT_SUBSYSTEM, strlen(UEVENT_SUBSYSTEM)))
				strcpy(event.subsystem, ptr+1);
			else if (!strncmp(&buf[i], UEVENT_DEVTYPE, strlen(UEVENT_DEVTYPE)))
				strcpy(event.devtype, ptr+1);
			else if (!strncmp(&buf[i], UEVENT_PARTN, strlen(UEVENT_PARTN)))
				strcpy(event.partn, ptr+1);
			else if (!strncmp(&buf[i], UEVENT_DEVPATH, strlen(UEVENT_DEVPATH)))
				snprintf(event.devpath, sizeof(event.devpath), "/sys%s", ptr+1);
			else if (!strncmp(&buf[i], UEVENT_USBSTATE, strlen(UEVENT_USBSTATE)))
				strcpy(event.usb_state, ptr+1);
		}
		i += l;
	}

	uevent_dispatch(&event);
	return ;
}


static void ev_send_cmd(const char *cmd)
{
	send(gEv.sockfd[1], cmd, strlen(cmd), 0);
}

static void usb_connect_function(void)
{
	if(access(gSDCardDevPath, F_OK) == 0)  {
		perform_handler(&usb_connect_with_card_func, (void *)gSDCardDevPath);
	}
	else
	{
		perform_handler(&usb_connect_without_card_func, NULL);
	}
}
static void usb_disconnect_function(void)
{
	perform_handler(&usb_disconnect_func, (void *)gSDCardDevPath);
}
static void sdcard_connect_function(void)
{
	if (get_usb_state() != 0)  {
		perform_handler(&udisk_connect_with_usb_func, (void *)gSDCardDevPath);
	} else
		perform_handler(&udisk_connect_without_usb_func, (void *)gSDCardDevPath);
}
static void sdcard_disconnect_function(void)
{
	perform_handler(&udisk_disconnect_func, NULL);
}

static void usb_state_update(EV_P_ struct ev_timer *w, int revents)
{
	struct timeval curTime;
	long time_diff = 0;
	DLOG("---usb state update!---\n");

	/*  current state is connected */
	if (get_usb_state() != 0) {
		/* last state isn't connected */
		if (gEv.usb_ev_state != EV_STATE_CONNECT) {
			/* detect plug-in USB*/
			printf("usb connected\n");
			if (gEv.usb_ev_state == 0) {
				/* enter here only once, for init state */
				gettimeofday(&curTime, NULL);
				printf("cur:%ld %ld  boot:%ld %ld\n", curTime.tv_sec, curTime.tv_usec, gSysBootTime.tv_sec, gSysBootTime.tv_usec);
				time_diff = 1000L * (curTime.tv_sec - gSysBootTime.tv_sec )
								+ (curTime.tv_usec - gSysBootTime.tv_usec)/1000;
				printf("usb connect enter here only once. %ld\n", time_diff);

				if (time_diff < (long)(1000*USB_UPDATE_STATE_TIMEOUT+1000)){
					printf("System boot with usb\n");
					gSysStartWithUSB = 1;
				}
			}
			usb_connect_function();
			gEv.usb_ev_state = EV_STATE_CONNECT;
		} else {
			DLOG("last state is CONNECT, so ignore this connect event\n");
		}
	} else { /* current state is disconnected */
		/* last state isn't disconnected */
		if (gEv.usb_ev_state != EV_STATE_DISCONNECT) {
			/* detect plug-out USB*/
			printf("usb disconnected\n");
			if (gEv.usb_ev_state == 0) {
				/* enter here only once, for init state */
				printf("usb disconnect enter here only once\n");
			}

			gSysStartWithUSB = 0;
			usb_disconnect_function();
			gEv.usb_ev_state = EV_STATE_DISCONNECT;
		} else {
			DLOG("last state is DISCONNECT, so ignore this disconnect event\n");
		}
	}
}

static void sdcard_state_update(EV_P_ struct ev_timer *w, int revents)
{
	struct timeval curTime;
	int time_diff = 0;

	DLOG("---sdcard state update!---\n");
	/*  current state is connected */
	if (get_sdcard_state() != 0) {
		/* last state isn't connected */
		if (gEv.sdcard_ev_state != EV_STATE_CONNECT) {
			/* detect plug-in SDCARD*/
			printf("sdcard connected\n");
			if (gEv.sdcard_ev_state == 0) {
				/* enter here only once, for init state */
				gettimeofday(&curTime, NULL);
				time_diff = 1000L * (curTime.tv_sec - gSysBootTime.tv_sec )
								+ (curTime.tv_usec - gSysBootTime.tv_usec)/1000;
				printf("sdcard connect enter here only once. %d\n", time_diff);
				if (time_diff < (long)(1000*SDCARD_UPDATE_STATE_TIMEOUT + 1000)){
					printf("System boot with sdcard\n");
					gSysStartWithSD = 1;
				}
			}

			sdcard_connect_function();
			gEv.sdcard_ev_state = EV_STATE_CONNECT;
		} else {
			DLOG("last state is CONNECT, so ignore this connect event\n");
		}
	} else { /* current state is disconnected */
		/* last state isn't disconnected */
		if (gEv.sdcard_ev_state != EV_STATE_DISCONNECT) {
			/* detect plug-out SDCARD*/
			printf("sdcard disconnected\n");
			if (gEv.sdcard_ev_state == 0) {
				/* enter here only once, for init state */
				printf("sd disconnect enter here only once\n");
			}

			gSysStartWithSD = 0;
			sdcard_disconnect_function();
			gEv.sdcard_ev_state = EV_STATE_DISCONNECT;
		} else {
			DLOG("last state is DISCONNECT, so ignore this disconnect event\n");
		}
	}
}

static void focus_state_update(EV_P_ struct ev_io *w, int revents)
{
	char buf[128];
	ssize_t size;

	DLOG("receive command !\n");

	memset(buf, 0, sizeof(buf));
	size = read(w->fd, buf, sizeof(buf));
	DLOG("read %dbytes, buf:%s\n", size, buf);
	if (!strcmp(buf, STATE_USB_CONNECT) || !strcmp(buf, STATE_USB_DISCONNECT)) {
		/* usb connect or disconnect*/
		ev_tstamp t;
		t = ev_timer_remaining(gEv.loop, &gEv.usb_handle);
		DLOG("stamp:%0.6f\n", t);
		if (t > 0.0)
			ev_timer_stop(gEv.loop, &gEv.usb_handle);
		DLOG("active usb-state-debounce timer\n");
		ev_timer_set(&gEv.usb_handle, USB_UPDATE_STATE_TIMEOUT, 0.0);
		ev_timer_start(gEv.loop, &gEv.usb_handle);
	} else if (!strcmp(buf, STATE_SDCARD_CONNECT) || !strcmp(buf, STATE_SDCARD_DISCONNECT)) {
		/* sdcard connect or disconnect*/
		ev_tstamp t;
		t = ev_timer_remaining(gEv.loop, &gEv.sdcard_handle);
		DLOG("stamp:%0.6f\n", t);
		if (t > 0.0)
			ev_timer_stop(gEv.loop, &gEv.sdcard_handle);
		DLOG("active sdcard-state-debounce timer\n");
		ev_timer_set(&gEv.sdcard_handle, SDCARD_UPDATE_STATE_TIMEOUT, 0.0);
		ev_timer_start(gEv.loop, &gEv.sdcard_handle);
	}
}


#if 0

void *ev_thread(void *arg)
{
	ev_run(gEv.loop, 0);
}

int ev_test()
{
	pthread_t tid;

	ev_io usb_watcher;

	gEv.loop = ev_default_loop(0);

	ev_timer_init(&gEv.usb_handle, usb_state_update, USB_UPDATE_STATE_TIMEOUT, 0);
	ev_timer_init(&gEv.sdcard_handle, sdcard_state_update, SDCARD_UPDATE_STATE_TIMEOUT, 0);

	/* io init */
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, gEv.sockfd) < 0) {
		printf("socketpair failed\n");
		return -1;
	}
	ev_io_init(&usb_watcher, focus_state_update, gEv.sockfd[0], EV_READ);
	ev_io_start(gEv.loop, &usb_watcher);

	pthread_create(&tid, NULL, ev_thread, NULL);
#if 0
	while(1) {
		char *cmd[] = {"usb_connect", "usb_disconnect", "sdcard_connect", "sdcard_disconnect"};
		static int count = 0;
		sleep(2);
		count %= 4;
		send(gEv.sockfd[1], cmd[count], strlen(cmd[count]), 0);
		count++;

		send(gEv.sockfd[1], cmd[0], strlen(cmd[0]), 0);
		getchar();
	}
#endif

}
#endif

static void disable_mdev(void)
{
	int fd;
	fd = open("/proc/sys/kernel/hotplug", O_WRONLY);
	if (fd < 0)
		return ;
	write(fd, "", 1);
	close(fd);
	return ;
}


int main()
{
	int socket;
	ev_io focus_watcher;
	ev_io uevent_watcher;

	//signal(SIGCHLD, SIG_IGN);
	usb_state_init();

	gEv.loop = ev_default_loop(0);

	/* init usb-state-debounce timer */
	ev_timer_init(&gEv.usb_handle, usb_state_update, USB_UPDATE_STATE_TIMEOUT, 0);
	/* init sdcard-state-debounce timer */
	ev_timer_init(&gEv.sdcard_handle, sdcard_state_update, SDCARD_UPDATE_STATE_TIMEOUT, 0);

	/* io init */
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, gEv.sockfd) < 0) {
		printf("socketpair failed\n");
		return -1;
	}

	/* focus-state-watcher */
	/* uevent handler will send msg by socketpair (ev_send_cmd) */
	ev_io_init(&focus_watcher, focus_state_update, gEv.sockfd[0], EV_READ);
	ev_io_start(gEv.loop, &focus_watcher);


	socket = uevent_monitor_open_socket();
	/* uevent-state-watcher */
	ev_io_init(&uevent_watcher, uevent_monitor_handler, socket, EV_READ);
	ev_io_start(gEv.loop, &uevent_watcher);

	/* get current time */
	gettimeofday(&gSysBootTime, NULL);

	/* default enable usb gadget mass_storage function
	 * comment it if useless
	 **/
	mass_storage_enable();

	/* ensure receive usb connect uevent first, so sleep 500ms */
	usleep(500*1000);

	/* disable mdev */
	disable_mdev();

	/* receive sdcard uevent again */
	sdcard_init();

	/* event loop */
	ev_run(gEv.loop, 0);

	return 0;

}
