#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <sys/epoll.h>
#include <bt_manager.h>
#include <fcntl.h>
#include <signal.h>
#include <getopt.h>

#include "aw_bt_test.h"
#include "aw_bt_test_dev.h"

btmg_callback_t *bt_callback = NULL;
static const size_t MAX_EVENTS = 64;
static btmg_state_t bt_state = BTMG_STATE_OFF;
static char bt_name[MAX_BT_NAME_LEN + 1] = {0};
static char bt_addr[MAX_BT_ADDR_LEN + 1] = {0};
static bool pin_notification = false;
static bool ssp_notification = false;
static dev_list_t *bonded_devices = NULL;
static dev_list_t *discovered_devices = NULL;
static int song_playing_pos = 0;
static int song_playing_len = 0;

static void bt_test_manager_cb(int event_id)
{
	printf("aw bt test callback function enter, event_id: %d!\n", event_id);
}

static void bt_test_status_cb(btmg_state_t status)
{
    char bt_addr[18] = {0};
	char bt_name_buf[64] = {0};
    char bt_name[64] = {0};
    int len = sizeof(bt_addr);

	printf("\n*************************************************\n");
	if (status == BTMG_STATE_OFF) {
		printf("*****aw_bt_test: BT is OFF!*****\n");
	} else if (status == BTMG_STATE_ON) {
		printf("******aw_bt_test: BT is ON!******\n");
        bt_manager_get_address(bt_addr, len);
        if (bt_addr[0] != '\0') {
            snprintf(bt_name_buf, 14, "aw-bt-test-%s", (char *)(bt_addr + 12));
            sprintf(bt_name, "%s-%s", bt_name_buf, (char *)(bt_addr + 15));
            bt_manager_set_name(bt_name);
        } else {
            bt_manager_set_name("aw-bt-test-001");
        }
		bt_manager_set_auto_ssp_reply(true);
	} else if (status == BTMG_STATE_TURNING_ON) {
		printf("******aw_bt_test: BT is TURNING ON!******\n");
	} else if (status == BTMG_STATE_TURNING_OFF) {
		printf("******aw_bt_test: BT is TURNING OFF!******\n");
	}
	printf("\n*************************************************\n");
}

static void bt_test_discovery_status_cb(btmg_discovery_state_t status)
{
	printf("\n*************************************************\n");
	if (status == BTMG_DISC_STARTED) {
		printf("******aw_bt_test: discovery is started******\n");
	} else if (status == BTMG_DISC_STOPPED_AUTO) {
		printf("******aw_bt_test: discovery is stopped automatically******\n");
	} else if (status == BTMG_DISC_START_FAILED) {
		printf("******aw_bt_test: discovery start failed******\n");
	} else if (status == BTMG_DISC_STOPPED_BY_USER) {
		printf("******aw_bt_test: discovery is stopped by user******\n");
	}
	printf("\n*************************************************\n");

}

static void bt_test_dev_found_cb(const char *address,const char *name, unsigned int bt_class, int rssi)
{
	dev_node_t *dev_node = NULL;

	dev_node = dev_find_device(bonded_devices, address);

	if (dev_node != NULL) {
		return;
	}

	dev_node = dev_find_device(discovered_devices, address);
	if (dev_node != NULL) {
		return;
	}

	dev_add_device(discovered_devices, name, address);
	printf("============Device is found============\n");
	printf("address: %s, name: %s, class: %d, rssi: %d\n", address, name, bt_class, rssi);
	printf("=======================================\n");
}

static void bt_test_update_rssi_cb(const char *address,const char *name, int rssi)
{
	dev_node_t *dev_node = NULL;

	dev_node = dev_find_device(bonded_devices, address);
	if (dev_node != NULL) {
		printf("============Update RSSI for Bonded Device============\n");
		printf("address: %s, name: %s, rssi: %d\n", address, name, rssi);
		printf("=================================================\n");
		return;
	}

	dev_node = dev_find_device(discovered_devices, address);
	if (dev_node != NULL) {
		printf("============Update RSSI for Discovered Device============\n");
		printf("address: %s, name: %s, rssi: %d\n", address, name, rssi);
		printf("=================================================\n");
		return;
	}

}

static void bt_test_bond_state_cb(btmg_bond_state_t state,const  char *bd_addr,const char *name)
{
	dev_node_t *dev_bonded_node = NULL;
	dev_node_t *dev_discovered_node = NULL;

	printf("bonded_device:state: %d, addr: %s, name: %s\n", state, bd_addr, name);
	dev_bonded_node = dev_find_device(bonded_devices, bd_addr);
	dev_discovered_node = dev_find_device(discovered_devices, bd_addr);

	if (state == BTMG_BOND_STATE_BONDED) {

        if (dev_bonded_node == NULL) {
			dev_add_device(bonded_devices, name, bd_addr);
        }

		if(dev_discovered_node != NULL) {
			dev_remove_device(discovered_devices, bd_addr);
        }

        fprintf(stdout, "\n*************************************************");
        fprintf(stdout, "\n Pairing state for %s is BONDED", name);
        fprintf(stdout, "\n*************************************************\n");
	} else if (state == BTMG_BOND_STATE_NONE) {
		if (dev_bonded_node != NULL) {
			dev_remove_device(bonded_devices, bd_addr);
        }
        fprintf(stdout, "\n*************************************************");
        fprintf(stdout, "\n Pairing state for %s is BOND NONE", name);
        fprintf(stdout, "\n*************************************************\n");
	}
}

static void bt_test_ssp_request_cb(const char *bd_addr,const char *name, uint32_t pass_key)
{
	printf("\n*************************************************");
	printf("\n BT pairing request::Device %s::%s, Pairing Code:: %d", name, bd_addr, pass_key);
	printf("\n*************************************************\n");
	printf(" ** Please enter yes / no **\n");
	ssp_notification = true;
	printf("\n*************************************************\n");
}

static void bt_test_pin_request_cb(const char *bd_addr,const char *name)
{
	printf("\n*************************************************");
	printf("\n BT Legacy pairing request::Device %s::%s", name, bd_addr);
	printf("\n*************************************************\n");
	printf(" ** Please enter valid PIN key **\n");
	pin_notification = true;
	printf("\n*************************************************\n");
}

static void bt_test_a2dp_sink_connection_state_cb(const char *bd_addr, btmg_a2dp_sink_connection_state_t state)
{

	printf("============A2dp sink connection state changed==============\n");
	if (state == BTMG_A2DP_SINK_DISCONNECTED) {
		printf("******A2dp sink disconnected with device: %s******\n", bd_addr);
	} else if (state == BTMG_A2DP_SINK_CONNECTING) {
		printf("******A2dp sink connecting with device: %s******\n", bd_addr);
	} else if (state == BTMG_A2DP_SINK_CONNECTED) {
		printf("******A2dp sink connected with device: %s******\n", bd_addr);
	} else if (state == BTMG_A2DP_SINK_DISCONNECTING) {
		printf("******A2dp sink disconnecting with device: %s******\n", bd_addr);
	}
	printf("=============================================================\n");
}

static void bt_test_a2dp_sink_audio_state_cb(const char *bd_addr, btmg_a2dp_sink_audio_state_t state)
{
	printf("============A2dp sink audio state changed==============\n");
	if (state == BTMG_A2DP_SINK_AUDIO_SUSPENDED) {
		printf("******A2dp sink audio suspended with device: %s******\n", bd_addr);
	} else if (state == BTMG_A2DP_SINK_AUDIO_STOPPED) {
		printf("******A2dp sink audio stopped with device: %s******\n", bd_addr);
	} else if (state == BTMG_A2DP_SINK_AUDIO_STARTED) {
		printf("******A2dp sink audio started with device: %s******\n", bd_addr);
	}
	printf("=======================================================\n");
}

static void bt_test_avrcp_play_state_cb(const char *bd_addr, btmg_avrcp_play_state_t state)
{
	printf("============AVRCP play state changed=======================\n");
	if (state == BTMG_AVRCP_PLAYSTATE_STOPPED) {
		printf("***BT playing music stopped with device: %s***\n", bd_addr);
	} else if (state == BTMG_AVRCP_PLAYSTATE_PLAYING) {
		printf("***BT palying music playing with device: %s***\n", bd_addr);
	} else if (state == BTMG_AVRCP_PLAYSTATE_PAUSED) {
		printf("***BT palying music paused with device: %s***\n", bd_addr);
	} else if (state == BTMG_AVRCP_PLAYSTATE_FWD_SEEK) {
		printf("***BT palying music FWD SEEK with device: %s***\n", bd_addr);
	} else if (state == BTMG_AVRCP_PLAYSTATE_REV_SEEK) {
		printf("***BT palying music REV SEEK with device: %s***\n", bd_addr);
	} else if (state == BTMG_AVRCP_PLAYSTATE_ERROR) {
		printf("***BT palying music ERROR with device: %s***\n", bd_addr);
	}
	printf("============================================================\n");

}
static void bt_test_avrcp_track_changed_cb(const char *bd_addr, btmg_track_info_t track_info)
{
	printf("==========================AVRCP play track changed======================\n");
	printf("BT playing device: %s\n", bd_addr);
	printf("BT playing music title: %s\n", track_info.title);
	printf("BT playing music artist: %s\n", track_info.artist);
	printf("BT playing music album: %s\n", track_info.album);
	printf("BT playing music track number: %s\n", track_info.track_num);
	printf("BT playing music total number of tracks: %s\n", track_info.num_tracks);
	printf("BT playing music genre: %s\n", track_info.genre);
	printf("BT playing music playing time: %s\n", track_info.playing_time);
	printf("=========================================================================\n");
}

static void bt_test_avrcp_audio_volume_cb(const char *bd_addr, unsigned int volume)
{
	printf("============Avrcp volume changed==============\n");
	printf("***Avrcp audio volume: %s : %d***\n", bd_addr, volume);
	printf("==================================================\n");
}

static void bt_test_avrcp_play_position_cb(const char *bd_addr, int song_len, int song_pos)
{
	if (song_playing_pos > song_pos && song_playing_len != song_len) {
		printf("==============AVRCP playing song switched================\n");
	}
	song_playing_len = song_len;
	song_playing_pos = song_pos;
	printf("=======================AVRCP play position changed======================\n");
	printf("\n \
		BT playing device: %s\n \
		playing song length: %d\n \
		playing position: %d\n", bd_addr, song_len, song_pos);
	printf("========================================================================\n");

}


static void print_discovered_devices()
{
	dev_node_t *dev_node = NULL;

	dev_node = discovered_devices->head;
	printf("\n**************************** Inquiry List *********************************\n");
	while (dev_node != NULL) {
		printf("%s %s\n", dev_node->dev_addr, dev_node->dev_name);
		dev_node = dev_node->next;
	}
    printf("****************************** End of List ***********************************\n");
}

static void print_bonded_devices()
{
	dev_node_t *dev_node = NULL;

	dev_node = bonded_devices->head;
	printf("\n**************************** Bonded List *********************************\n");
	while (dev_node != NULL) {
		printf("%s %s\n", dev_node->dev_addr, dev_node->dev_name);
		dev_node = dev_node->next;
	}
    printf("****************************** End of List *********************************\n");

}


static void exit_handler()
{
    if ( bt_manager_get_state()  == BTMG_STATE_ON) {
		bt_manager_enable(false);
    }

	while (bt_manager_get_state()  != BTMG_STATE_OFF) {
		sleep(1);
	}

	bt_manager_deinit(bt_callback);
	dev_list_free(discovered_devices);
	dev_list_free(bonded_devices);
	exit(0);
}

void main_loop_stop(int sig)
{
	struct sigaction sigact = { .sa_handler = SIG_DFL };
	sigaction(sig, &sigact, NULL);
    if ( bt_manager_get_state()  == BTMG_STATE_ON) {
		bt_manager_enable(false);
    }

	while (bt_manager_get_state()  != BTMG_STATE_OFF) {
		sleep(1);
	}

	bt_manager_deinit(bt_callback);
	dev_list_free(discovered_devices);
	dev_list_free(bonded_devices);
	exit(0);

}
int daemon_init(int debug)
{
    int i ;
    pid_t pid;
    if((pid = fork()) < 0)
        return -1;
    else if(pid)
        _exit(0);
    if(setsid() < 0)
        return -1;
    signal(SIGHUP,SIG_IGN);
    if((pid = fork()) < 0)
        return -1;
    else if(pid)
        _exit(0);
    chdir("/");
    if(!debug) {
        for(i = 0; i < 64; i++)
            close(i);
        open("/dev/null",O_RDONLY);
        open("/dev/null",O_RDWR);
        open("/dev/null",O_RDWR);
    }
    return 0;
}

int main(int argc, char **argv)
{
	int ret = -1;
	int choice;
	static bool bt_enable_flag = false;
	bool background = false;
	fd_set readfds;
	int debug_level = BTMG_LOG_LEVEL_INFO;
	int opt;
	const char *opts = "hd:";
	const struct option longopts[] = {
		{ "help", no_argument,NULL, 'h' },
		{ "loglevel",no_argument,NULL, 'd' },
		{ 0,0,0,0},
	};
	optind = 0; opterr = 1;
	while ((opt = getopt_long(argc, argv, opts, longopts, NULL)) != -1) {
		switch(opt) {
			case 'h'/*help*/:
				printf("Usage:\n"
						"  -h, --help\t\t print this help and exit\n"
						"  -d, --debug\t\tdebug level 0~5\n");
					return EXIT_SUCCESS;
			case 'd'/*debug level*/:
				debug_level = atoi(optarg);
				break;
			default:
				fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
				return EXIT_FAILURE;
		}
	}
	ret = bt_manager_set_loglevel(debug_level);
	if (0 != ret)
		printf("%d:set log level fault!\n", __LINE__);

	daemon_init(1);

	ret = bt_manager_preinit(&bt_callback);
	if (ret != 0) {
		printf("bt preinit failed!\n");
		exit(-1);
	}

	//printf("bt_callback's addr: %llu", bt_callback);
	bt_callback->btmg_manager_cb.bt_mg_cb = bt_test_manager_cb;
	bt_callback->btmg_gap_cb.gap_status_cb = bt_test_status_cb;
	bt_callback->btmg_gap_cb.gap_disc_status_cb = bt_test_discovery_status_cb;
	bt_callback->btmg_gap_cb.gap_dev_found_cb = bt_test_dev_found_cb;
	bt_callback->btmg_gap_cb.gap_update_rssi_cb =	bt_test_update_rssi_cb;
	bt_callback->btmg_gap_cb.gap_bond_state_cb = bt_test_bond_state_cb;
	bt_callback->btmg_gap_cb.gap_ssp_request_cb = bt_test_ssp_request_cb;
	bt_callback->btmg_gap_cb.gap_pin_request_cb = bt_test_pin_request_cb;
	bt_callback->btmg_a2dp_sink_cb.a2dp_sink_connection_state_cb = bt_test_a2dp_sink_connection_state_cb;
	bt_callback->btmg_a2dp_sink_cb.a2dp_sink_audio_state_cb = bt_test_a2dp_sink_audio_state_cb;
	bt_callback->btmg_avrcp_cb.avrcp_play_state_cb = bt_test_avrcp_play_state_cb;
	bt_callback->btmg_avrcp_cb.avrcp_play_position_cb = bt_test_avrcp_play_position_cb;
	bt_callback->btmg_avrcp_cb.avrcp_track_changed_cb = bt_test_avrcp_track_changed_cb;
	bt_callback->btmg_avrcp_cb.avrcp_audio_volume_cb = bt_test_avrcp_audio_volume_cb;
	bt_manager_init(bt_callback);


	bonded_devices = dev_list_new();
	discovered_devices = dev_list_new();


	struct sigaction sigact = { .sa_handler = main_loop_stop };
	sigaction(SIGTERM, &sigact, NULL);
	sigaction(SIGINT, &sigact, NULL);

	bt_manager_enable(true);

	while(1) {
		printf("bt test backgroud run.\n");
		select(0,NULL,NULL,NULL,NULL);
	}
	return 0;
}
