#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <sys/epoll.h>
#include <bt_manager.h>
#include "aw_bt_test.h"
#include "aw_bt_test_dev.h"

btmg_callback_t *bt_callback = NULL;
static const size_t MAX_EVENTS = 64;
static btmg_state_t bt_state = BTMG_STATE_OFF;
static char bt_name[MAX_BT_NAME_LEN + 1] = {0};
static char bt_addr[MAX_BT_ADDR_LEN + 1] = {0};
static bool pin_notification = false;
static bool ssp_notification = false;

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
            snprintf(bt_name_buf, 14, "AW_BT_TEST_%s", (char *)(bt_addr + 12));
            sprintf(bt_name, "%s_%s", bt_name_buf, (char *)(bt_addr + 15));
            bt_manager_set_name(bt_name);
        } else {
            bt_manager_set_name("AW_BT_TEST");
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

	dev_node = dev_find_device(discovered_devices, address);
	if (dev_node != NULL) {
			printf("============Update RSSI for Discovered Device============\n");
			printf("address: %s, name: %s, rssi: %d\n", address, name, rssi);
			printf("=================================================\n");
			return;
	}

	printf("address: %s, name: %s, rssi: %d\n", address, name, rssi);
}

static void bt_test_bond_state_cb(btmg_bond_state_t state,const  char *bd_addr,const char *name)
{
	printf("bonded_device:state: %d, addr: %s, name: %s\n", state, bd_addr, name);
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

static void bonded_devices_list()
{

		bt_paried_device *devices = NULL;
		int count;

		bt_manager_get_paired_devices(&devices,&count);
		bt_paried_device *iter = devices;
		if(iter) {
			while(iter) {
				printf("\n \
				addres: %s,\n \
				name:%s \n \
				rssi:%d \n \
				bonded:%d \n \
				connected:%d \n",iter->remote_address,
				iter->remote_name,iter->rssi,
				iter->is_bonded,iter->is_connected);

				iter = iter->next;
			}
			bt_manager_free_paired_devices(devices);
		} else {
			printf("bonded devices is empty\n");
		}
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
	exit(0);
}

static void display_menu(menu_type_t menu_type)
{

    user_menu_list_t *menu = NULL;
    int index = 0, num_cmds = 0;

    switch(menu_type) {
        case GAP_MENU:
            menu = &gap_menu[0];
            num_cmds  = NO_OF_COMMANDS(gap_menu);
            break;
        case MAIN_MENU:
            menu = &main_menu[0];
            num_cmds  = NO_OF_COMMANDS(main_menu);
            break;
        case A2DP_SINK_MENU:
            menu = &a2dp_sink_menu[0];
            num_cmds  = NO_OF_COMMANDS(a2dp_sink_menu);
            break;
        case A2DP_SOURCE_MENU:
            menu = &a2dp_source_menu[0];
            num_cmds  = NO_OF_COMMANDS(a2dp_source_menu);
            break;
        case HFP_CLIENT_MENU:
            menu = &hfp_client_menu[0];
            num_cmds  = NO_OF_COMMANDS(hfp_client_menu);
            break;
       case SPP_CLIENT_MENU:

            break;

/*TODO: to be implemented in future plan*/
/*
            switch(pSppClient->getState())
                {
                    case STATE_SPP_CLIENT_IDLE:
                       {
                           menu = &SppClientIdleMenu[0];
                           num_cmds  = NO_OF_COMMANDS(SppClientIdleMenu);
                       }
                       break;

                    case STATE_SPP_CLIENT_CONNECTING:
                       {
                           menu = &SppClientConnectingMenu[0];
                           num_cmds  = NO_OF_COMMANDS(SppClientConnectingMenu);
                       }
                       break;


                    case STATE_SPP_CLIENT_CONNECTED:
                       {
                           menu = &SppClientConnectedMenu[0];
                           num_cmds  = NO_OF_COMMANDS(SppClientConnectedMenu);
                       }
                       break;


                    case STATE_SPP_CLIENT_SEND_FILE:
                       {
                           menu = &SppClientSendMenu[0];
                           num_cmds  = NO_OF_COMMANDS(SppClientSendMenu);
                       }
                       break;

                    case STATE_SPP_CLIENT_RECEIVE_FILE:
                       {
                           menu = &SppClientReceiveMenu[0];
                           num_cmds  = NO_OF_COMMANDS(SppClientReceiveMenu);
                       }
                       break;

                    case STATE_SPP_CLIENT_DISCONNECTED:
                       {
                           menu = &SppClientDisconnectedMenu[0];
                           num_cmds  = NO_OF_COMMANDS(SppClientDisconnectedMenu);
                       }
                       break;

                   default: fprintf (stdout, "\n*** Error SPP Client Invalid state ***\n");


                }
*/
	} /*end of switch(menu_type)*/
    fprintf (stdout, "\n******************** AW BT TEST DEMO Based on Bluez**********************\n");
    fprintf (stdout, "\n***************************** Menu **************************************\n");
    for (index = 0; index < num_cmds; index++)
        fprintf (stdout, "\t %s\n",  menu[index].cmd_help);
    fprintf (stdout, " ***************************************************************************\n");

}


static bool handle_user_input (int *cmd_id, char input_args[][COMMAND_ARG_SIZE], menu_type_t menu_type)
{
    char user_input[COMMAND_SIZE] = {'\0'};
    int index = 0 , found_index = -1, num_cmds;
    char *temp_arg = NULL;
    char delim[] = " ";
    char *ptr1;
    int param_count = 0;
    bool status = false;
    int max_param = 0;
    user_menu_list_t *menu = NULL;

    // validate the input string
    if(((fgets (user_input, sizeof (user_input), stdin)) == NULL) ||
       (user_input[0] == '\n')) {
        return status;
    }
    // remove trialing \n character
    user_input[strlen(user_input) - 1] = '\0';

    // According to the current menu assign Command menu
    switch(menu_type) {
        case GAP_MENU:
            menu = &gap_menu[0];
            num_cmds  = NO_OF_COMMANDS(gap_menu);
            break;
        case A2DP_SINK_MENU:
            menu = &a2dp_sink_menu[0];
            num_cmds  = NO_OF_COMMANDS(a2dp_sink_menu);
            break;
        case A2DP_SOURCE_MENU:
            menu = &a2dp_source_menu[0];
            num_cmds  = NO_OF_COMMANDS(a2dp_source_menu);
            break;
        case HFP_CLIENT_MENU:
            menu = &hfp_client_menu[0];
            num_cmds  = NO_OF_COMMANDS(hfp_client_menu);
            break;
        case SPP_CLIENT_MENU:
/*
            switch(pSppClient->getState())
			{
                case STATE_SPP_CLIENT_IDLE:
                   {
                       menu = &SppClientIdleMenu[0];
                       num_cmds  = NO_OF_COMMANDS(SppClientIdleMenu);
                   }
                   break;

                case STATE_SPP_CLIENT_CONNECTING:
                   {
                       menu = &SppClientConnectingMenu[0];
                       num_cmds  = NO_OF_COMMANDS(SppClientConnectingMenu);
                   }
                   break;


                case STATE_SPP_CLIENT_CONNECTED:
                   {
                       menu = &SppClientConnectedMenu[0];
                       num_cmds  = NO_OF_COMMANDS(SppClientConnectedMenu);
                   }
                   break;


                case STATE_SPP_CLIENT_SEND_FILE:
                   {
                       menu = &SppClientSendMenu[0];
                       num_cmds  = NO_OF_COMMANDS(SppClientSendMenu);
                   }
                   break;

                case STATE_SPP_CLIENT_RECEIVE_FILE:
                   {
                       menu = &SppClientReceiveMenu[0];
                       num_cmds  = NO_OF_COMMANDS(SppClientReceiveMenu);
                   }
                   break;

                case STATE_SPP_CLIENT_DISCONNECTED:
                   {
                       menu = &SppClientDisconnectedMenu[0];
                       num_cmds  = NO_OF_COMMANDS(SppClientDisconnectedMenu);
                   }
                   break;

               default: fprintf (stdout, "\n*** Error SPP Client Invalid state ***\n");
            }
            break;
*/
        case MAIN_MENU:
        // fallback to default main menu
        default:
            menu = &main_menu[0];
            num_cmds  = NO_OF_COMMANDS(main_menu);
            break;
    } /*end of switch*/

    if ( (temp_arg = strtok_r(user_input, delim, &ptr1)) != NULL ) {
        // find out the command name
        for (index = 0; index < num_cmds; index++) {
            if(!strcasecmp (menu[index].cmd_name, temp_arg) || !strcasecmp (menu[index].cmd_code, temp_arg)) {
                *cmd_id = menu[index].cmd_id;
                found_index = index;
                strncpy(input_args[param_count], temp_arg, COMMAND_ARG_SIZE);
                input_args[param_count++][COMMAND_ARG_SIZE - 1] = '\0';
                break;
            }
        }

        // validate the command parameters
        if (found_index != -1 ) {
            max_param = menu[found_index].max_param;
            while ((temp_arg = strtok_r(NULL, delim, &ptr1)) &&
                    (param_count < max_param + 1)) {
                strncpy(input_args[param_count], temp_arg, COMMAND_ARG_SIZE);
                input_args[param_count++][COMMAND_ARG_SIZE - 1] = '\0';
            }

            // consider command as other param
            if(param_count == max_param + 1) {
                if(temp_arg != NULL) {
                    fprintf( stdout, " Maximum params reached\n");
                    fprintf( stdout, " Refer help: %s\n", menu[found_index].cmd_help);
                } else {
                    status = true;
                }
            } else if(param_count < max_param + 1) {
                fprintf( stdout, " Missing required parameters\n");
                fprintf( stdout, " Refer help: %s\n", menu[found_index].cmd_help);
            }
        } else {
            // to handle the paring inputs
            if(temp_arg != NULL) {
                strncpy(input_args[param_count], temp_arg, COMMAND_ARG_SIZE);
                input_args[param_count++][COMMAND_ARG_SIZE - 1] = '\0';
            }
        }
    }
    return status;
}

static void handle_main_command(int cmd_id, char user_cmd[][COMMAND_ARG_SIZE])
{
    switch (cmd_id) {
        case GAP_OPTION:
            menu_type = GAP_MENU;
            display_menu(menu_type);
            break;
        case A2DP_SINK:
            menu_type = A2DP_SINK_MENU;
            display_menu(menu_type);
            break;
        case A2DP_SOURCE:
            menu_type = A2DP_SOURCE_MENU;
            display_menu(menu_type);
            break;
        case HFP_CLIENT:
            menu_type = HFP_CLIENT_MENU;
            display_menu(menu_type);
            break;
        case SPP_CLIENT_OPTION:
          //  menu_type = SPP_CLIENT_MENU;
           // display_menu(menu_type);
			printf("NOT support at present, please try other functions!\n");
            menu_type = MAIN_MENU;
            display_menu(menu_type);
            break;
        case MAIN_EXIT:
            printf("Self exit of Main thread\n");
/*TODO: to be implemented at last*/
            exit_handler();
            break;
         default:
            printf("Command not handled\n");
            break;
    }


}


static void handle_gap_command(int cmd_id, char user_cmd[][COMMAND_ARG_SIZE])
{
	int ret = 0;

    switch (cmd_id) {
        case BACK_TO_MAIN:
            menu_type = MAIN_MENU;
			display_menu(menu_type);
            break;
		case SET_DISCOVERY_MODE:
			if (!strcmp(user_cmd[ONE_PARAM], "none") || !strcmp(user_cmd[ONE_PARAM], "NONE"))
				bt_manager_set_discovery_mode(BTMG_SCAN_MODE_NONE);
			else if (!strcmp(user_cmd[ONE_PARAM], "connectable") || !strcmp(user_cmd[ONE_PARAM], "CONNECTABLE"))
				bt_manager_set_discovery_mode(BTMG_SCAN_MODE_CONNECTABLE);
			else if (!strcmp(user_cmd[ONE_PARAM], "connectable_discoverable") || !strcmp(user_cmd[ONE_PARAM], "CONNECTABLE_DISCOVERABLE"))
				bt_manager_set_discovery_mode(BTMG_SCAN_MODE_CONNECTABLE_DISCOVERABLE);
			break;
		case BT_ENABLE:
			bt_manager_enable(true);
			break;
		case BT_DISABLE:
			bt_manager_enable(false);
			break;
		case START_ENQUIRY:
			bt_manager_start_discovery();
			break;
		case CANCEL_ENQUIRY:
			bt_manager_cancel_discovery();
			break;
		case START_PAIR:
			ret = bt_manager_pair(user_cmd[ONE_PARAM]);
			if (ret)
				printf("\nERROR in start pair\n");
			break;
		case UNPAIR:
			bt_manager_unpair(user_cmd[ONE_PARAM]);
			break;
		case INQUIRY_LIST:
			if (bt_manager_is_discovering()) {
				printf("BT is still in discovering, please try later\n");
				break;
			} else {
				print_discovered_devices();
				break;
			}
		case BONDED_LIST:
			bonded_devices_list();
			break;
		case GET_BT_STATE:
			bt_state = bt_manager_get_state();
			if (bt_state == BTMG_STATE_OFF)
				printf("\n The BT's state is OFF\n");
			else if (bt_state == BTMG_STATE_ON)
				printf("\n The BT's state is ON\n");
			else if (bt_state == BTMG_STATE_TURNING_ON)
				printf("\n The BT's state is turning ON\n");
			else if (bt_state == BTMG_STATE_TURNING_OFF)
				printf("\n The BT's state is turning OFF\n");
			break;
		case GET_BT_NAME:
			bt_manager_get_name(bt_name, sizeof(bt_name));
			printf("\n\tThe BT's name is %s\n", bt_name);
			break;
		case GET_BT_ADDR:
			ret = bt_manager_get_address(bt_addr, sizeof(bt_addr));
			if (!ret)
				printf("\n\tThe BT's name is %s\n", bt_addr);
			else
				printf("\nERROR in get_address\n");
			break;
		case SET_BT_NAME:
			ret = bt_manager_set_name(user_cmd[ONE_PARAM]);
			if (ret)
				printf("\nERROR in set bt name\n");
			break;
		default:
			printf("\ncomand is not supported\n");
			break;
	}
}

static void handle_a2dp_sink_command(int cmd_id, char user_cmd[][COMMAND_ARG_SIZE])
{
	switch	(cmd_id) {
        case BACK_TO_MAIN:
            menu_type = MAIN_MENU;
            display_menu(menu_type);
            break;
		case CONNECT:
			bt_manager_a2dp_sink_connect(user_cmd[ONE_PARAM]);
			break;
		case DISCONNECT:
			bt_manager_a2dp_sink_disconnect(user_cmd[ONE_PARAM]);
			break;
		case PLAY:
			bt_manager_avrcp_command(user_cmd[ONE_PARAM], BTMG_AVRCP_PLAY);
			break;
		case PAUSE:
			bt_manager_avrcp_command(user_cmd[ONE_PARAM], BTMG_AVRCP_PAUSE);
			break;
		case STOP:
			bt_manager_avrcp_command(user_cmd[ONE_PARAM], BTMG_AVRCP_STOP);
			break;
		case FASTFORWARD:
			bt_manager_avrcp_command(user_cmd[ONE_PARAM], BTMG_AVRCP_FASTFORWARD);
			break;
		case REWIND:
			bt_manager_avrcp_command(user_cmd[ONE_PARAM], BTMG_AVRCP_REWIND);
			break;
		case FORWARD:
			bt_manager_avrcp_command(user_cmd[ONE_PARAM], BTMG_AVRCP_FORWARD);
			break;
		case BACKWARD:
			bt_manager_avrcp_command(user_cmd[ONE_PARAM], BTMG_AVRCP_BACKWARD);
			break;
		case VOL_UP:
			bt_manager_avrcp_command(user_cmd[ONE_PARAM], BTMG_AVRCP_VOL_UP);
			break;
		case VOL_DOWN:
			bt_manager_avrcp_command(user_cmd[ONE_PARAM], BTMG_AVRCP_VOL_DOWN);
			break;
		case VOL_CHANGED_NOTI:
			bt_manager_vol_changed_noti(user_cmd[ONE_PARAM]);
			break;
		default:
			printf("\nThe command is not supported\n");
			break;
	}

}

static void handle_a2dp_source_command(int cmd_id, char user_cmd[][COMMAND_ARG_SIZE])
{
	switch	(cmd_id) {
        case BACK_TO_MAIN:
            menu_type = MAIN_MENU;
            display_menu(menu_type);
            break;
		case CONNECT:
		case DISCONNECT:
		case PLAY:
		case PAUSE:
		case STOP:
		case TRACK_CHANGE:
		case SET_ABS_VOL:
		case SEND_VOL_UP_DOWN:
		case ADDR_PLAYER_CHANGE:
		case AVAIL_PLAYER_CHANGE:
		case BIGGER_METADATA:
		default:
			printf("\nThe A2dp_source is not supported at present\n");
			break;
	}
}


static void handle_hfp_client_command(int cmd_id, char user_cmd[][COMMAND_ARG_SIZE])
{

	switch	(cmd_id) {
        case BACK_TO_MAIN:
            menu_type = MAIN_MENU;
            display_menu(menu_type);
            break;
		case CONNECT:
		case DISCONNECT:
		case CREATE_SCO_CONN:
		case DESTROY_SCO_CONN:
		case ACCEPT_CALL:
		case REJECT_CALL:
		case END_CALL:
		case HOLD_CALL:
		case RELEASE_HELD_CALL:
		case RELEASE_ACTIVE_ACCEPT_WAITING_OR_HELD_CALL:
		case SWAP_CALLS:
		case ADD_HELD_CALL_TO_CONF:
		case RELEASE_SPECIFIED_ACTIVE_CALL:
		case PRIVATE_CONSULTATION_MODE:
		case PUT_INCOMING_CALL_ON_HOLD:
		case ACCEPT_HELD_INCOMING_CALL:
		case REJECT_HELD_INCOMING_CALL:
		case DIAL:
		case REDIAL:
		case DIAL_MEMORY:
		case START_VR:
		case STOP_VR:
		case CALL_ACTION:
		case QUERY_CURRENT_CALLS:
		case QUERY_OPERATOR_NAME:
		case QUERY_SUBSCRIBER_INFO:
		case SCO_VOL_CTRL:
		case MIC_VOL_CTRL:
		case SPK_VOL_CTRL:
		case SEND_DTMF:
		case DISABLE_NREC_ON_AG:
		default:
			printf("\nThe Hfp client is not supported at present\n");
			break;
	}

}

static void handle_spp_client_command(int cmd_id, char user_cmd[][COMMAND_ARG_SIZE])
{

	switch	(cmd_id) {
        case BACK_TO_MAIN:
            menu_type = MAIN_MENU;
            display_menu(menu_type);
            break;
		case CONNECT:
		case SPPC_SEND:
		case SPPC_RECV:
		case DISCONNECT:
		default:
			printf("\nThe spp client is not supported at present\n");
			break;
	}
}

bool handle_ssp_input(char user_cmd[][COMMAND_ARG_SIZE])
{
	bool accept = false;

	if (!strcasecmp (user_cmd[ZERO_PARAM], "yes")) {
        accept = true;
    }
    else if (!strcasecmp (user_cmd[ZERO_PARAM], "no")) {
        accept = false;
    } else {
        fprintf( stdout, " Wrong option selected\n");
        return false;
    }

	bt_manager_ssp_reply(accept);

	return true;
}

bool handle_pin_input(char user_cmd[][COMMAND_ARG_SIZE])
{
	int ret = 0;

	ret = bt_manager_pin_reply(true, user_cmd[ZERO_PARAM]);

	if (ret)
		return false;
	else
		return true;
}

static void bt_cmd_handler (void *context)
{

    int cmd_id = -1;
    char user_cmd[MAX_ARGUMENTS][COMMAND_ARG_SIZE];
    int index = 0;

    memset( (void *) user_cmd, '\0', sizeof(user_cmd));
    if (handle_user_input (&cmd_id, user_cmd, menu_type)) {
        switch(menu_type) {
            case MAIN_MENU:
                handle_main_command(cmd_id,user_cmd);
                break;
            case GAP_MENU:
                handle_gap_command(cmd_id,user_cmd);
                break;
            case A2DP_SINK_MENU:
                handle_a2dp_sink_command(cmd_id,user_cmd);
                break;
            case A2DP_SOURCE_MENU:
                handle_a2dp_source_command(cmd_id, user_cmd);
                break;
            case HFP_CLIENT_MENU:
                handle_hfp_client_command(cmd_id,user_cmd);
                break;
            case SPP_CLIENT_MENU:
                handle_spp_client_command(cmd_id, user_cmd);
                break;
        }
    } else if (ssp_notification && user_cmd[0][0] &&
                        (!strcasecmp (user_cmd[ZERO_PARAM], "yes") ||
                        !strcasecmp (user_cmd[ZERO_PARAM], "no"))
                        && handle_ssp_input(user_cmd)) {
        // validate the user input for SSP
        ssp_notification = false;
    } else if (pin_notification && user_cmd[0][0] &&
                        (strcasecmp (user_cmd[ZERO_PARAM], "yes") &&
                        strcasecmp (user_cmd[ZERO_PARAM], "no") &&
                        strcasecmp (user_cmd[ZERO_PARAM], "accept") &&
                        strcasecmp (user_cmd[ZERO_PARAM], "reject"))
                        && handle_pin_input(user_cmd)) {
        // validate the user input for PIN
        pin_notification = false;
    } else {
        fprintf( stdout, " Wrong option selected\n");
        display_menu(menu_type);
        // TODO print the given input string
        return;
    }
}


void *main_user_input_handler(void *arg)
{
	int ret = 0, i = 0;
	int epoll_fd = INVALID_FD;
	int event_fd = INVALID_FD;
	struct epoll_event event;
	struct epoll_event events[MAX_EVENTS];

	epoll_fd = epoll_create(MAX_EVENTS);
	if (epoll_fd == INVALID_FD) {
        printf("unable to create epoll instance: %s", strerror(errno));
        exit(-1);
    }
/*
    event_fd = eventfd(0, 0);
    if (ret->event_fd == INVALID_FD)
    {
        BTMG_ERROR("unable to create eventfd: %s", strerror(errno));
        goto error;
    }
*/
    memset(&event, 0, sizeof(event));
	event.events |= (EPOLLIN | EPOLLRDHUP | EPOLLHUP | EPOLLERR);
    event.data.ptr = NULL;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &event) == -1) {
        printf("unable to register eventfd with epoll set: %s",strerror(errno));
		close(epoll_fd);
		exit(-1);
    }

	while (1) {
        do {
            ret = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        } while (errno == EINTR);


        if (ret == -1) {
            printf("error in epoll_wait: %s", strerror(errno));
			close(epoll_fd);
			exit(-1);
        }

		for (i = 0; i < ret; i++) {
			if (events[i].events & EPOLLERR)
				printf("epoll error happened\n");

			if (events[i].events & EPOLLHUP)
				printf("epoll hang up happened\n");

			if (events[i].events & EPOLLRDHUP)
				printf("peer writing shut down\n");

			if (events[i].events & EPOLLIN)
						bt_cmd_handler(NULL);
		}

	}
}

int main()
{
	int ret = 0, i = 0;
	int epoll_fd = INVALID_FD;
	int event_fd = INVALID_FD;
	struct epoll_event event;
	struct epoll_event events[MAX_EVENTS];

	printf("Hello! This is Aw bt test!\n");
	ret = bt_manager_set_loglevel(BTMG_LOG_LEVEL_INFO);
	if (0 != ret)
		printf("%d:set log level fault!\n", __LINE__);

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

	discovered_devices = dev_list_new();

	pthread_t tid;

	pthread_create(&tid, NULL, main_user_input_handler, NULL);

	while(1)
		sleep(1);

//	while(1) {
//		printf("==========start to epoll_wait==========\n");
//        do {
//            ret = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
//        } while (errno == EINTR);
//
//
//        if (ret == -1) {
//            printf("error in epoll_wait: %s", strerror(errno));
//			close(epoll_fd);
//            return -1;
//        }
//
//		for (i = 0; i < ret; i++) {
//			if (events[i].events & EPOLLERR)
//				printf("epoll error happened\n");
//
//			if (events[i].events & EPOLLHUP)
//				printf("epoll hang up happened\n");
//
//			if (events[i].events & EPOLLRDHUP)
//				printf("peer writing shut down\n");
//
//			if (events[i].events & EPOLLIN)
//						bt_cmd_handler(NULL);
//		}
//	}

	bt_manager_deinit(bt_callback);
	dev_list_free(discovered_devices);
	return 0;
}
