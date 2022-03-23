#ifndef __AW_BT_TEST_H
#define __AW_BT_TEST_H


#define COMMAND_SIZE        200
#define COMMAND_ARG_SIZE     50
#define MAX_ARGUMENTS        20
#define INVALID_FD (-1)
#define  NO_OF_COMMANDS(x)  (sizeof(x) / sizeof((x)[0]))

/**
 * list of supported commands
 */
typedef enum {
	SET_DISCOVERY_MODE,
    BT_ENABLE,
    BT_DISABLE,
    START_ENQUIRY,
    CANCEL_ENQUIRY,
    MAIN_EXIT,
    START_PAIR,
    INQUIRY_LIST,
    BONDED_LIST,
    GET_BT_NAME,
    GET_BT_ADDR,
    SET_BT_NAME,
    UNPAIR,
    GET_BT_STATE,
    TEST_MODE,
    GAP_OPTION,
    TEST_ON_OFF,
    A2DP_SINK,
    A2DP_SOURCE,
    CONNECT,
    DISCONNECT,
    PLAY,
    PAUSE,
    STOP,
    FASTFORWARD,
    REWIND,
    FORWARD,
    BACKWARD,
    VOL_UP,
    VOL_DOWN,
    TRACK_CHANGE,
    SET_ABS_VOL,
    SEND_VOL_UP_DOWN,
    VOL_CHANGED_NOTI,
    ADDR_PLAYER_CHANGE,
    AVAIL_PLAYER_CHANGE,
    BIGGER_METADATA,
    PAN_OPTION,
    CONNECTED_LIST,
    SET_TETHERING,
    GET_PAN_MODE,
    RSP_OPTION,
    RSP_INIT,
    RSP_START,
    HFP_CLIENT,
    CREATE_SCO_CONN,
    DESTROY_SCO_CONN,
    ACCEPT_CALL,
    REJECT_CALL,
    END_CALL,
    HOLD_CALL,
    RELEASE_HELD_CALL,
    RELEASE_ACTIVE_ACCEPT_WAITING_OR_HELD_CALL,
    SWAP_CALLS,
    ADD_HELD_CALL_TO_CONF,
    RELEASE_SPECIFIED_ACTIVE_CALL,
    PRIVATE_CONSULTATION_MODE,
    PUT_INCOMING_CALL_ON_HOLD,
    ACCEPT_HELD_INCOMING_CALL,
    REJECT_HELD_INCOMING_CALL,
    DIAL,
    REDIAL,
    DIAL_MEMORY,
    START_VR,
    STOP_VR,
    CALL_ACTION,
    QUERY_CURRENT_CALLS,
    QUERY_OPERATOR_NAME,
    QUERY_SUBSCRIBER_INFO,
    SCO_VOL_CTRL,
    MIC_VOL_CTRL,
    SPK_VOL_CTRL,
    SEND_DTMF,
    DISABLE_NREC_ON_AG,
    SEND_AT_CMD,
    HFP_AG,
    SPP_SERVER_OPTION,
    SPPS_START,
    SPPS_RECV,
    SPPS_SEND,
    SPP_CLIENT_OPTION,
    SPPC_SEND,
    SPPC_RECV,
    HID_CLIENT,
    HID_HOST,
    SET_PROTOCOL,
    GET_PROTOCOL,
    SET_REPORT,
    GET_REPORT,
    VIRTUAL_UNPLUG,
    HID_BONDED_LIST,
    BACK_TO_MAIN,
    END,
} command_list_t;


/**
 * Argument Count
 */
typedef enum {
    ZERO_PARAM,
    ONE_PARAM,
    TWO_PARAM,
    THREE_PARAM,
    FOUR_PARAM
} max_param_count_t;

typedef enum {
    MAIN_MENU,
    GAP_MENU,
    A2DP_SINK_MENU,
    HFP_CLIENT_MENU,
    A2DP_SOURCE_MENU,
    SPP_CLIENT_MENU
} menu_type_t;

/**
 * Default menu_type is Main Menu
 */
menu_type_t menu_type = MAIN_MENU;

/**
 * UserMenuList
 */
typedef struct {
    command_list_t cmd_id;
    const char cmd_name[COMMAND_SIZE];
    const char cmd_code[COMMAND_SIZE];
    max_param_count_t max_param;
    const char cmd_help[COMMAND_SIZE];
} user_menu_list_t;

/**
 * list of supported commands for GAP
 */
user_menu_list_t gap_menu[] = {
	{SET_DISCOVERY_MODE,	"set_disc_mode",	"101",	ONE_PARAM,		"(101)set_disc_mode<space>none \
	/NONE/connectable/CONNECTABLE/connectable_discoverable/CONNECTABLE_DISCOVERABLE"},
    {BT_ENABLE,             "enable",			"102",	ZERO_PARAM,    "(102)enable"},
    {BT_DISABLE,            "disable",			"103",	ZERO_PARAM,    "(103)disable"},
    {START_ENQUIRY,         "inquiry",			"104",	ZERO_PARAM,    "(104)inquiry"},
    {CANCEL_ENQUIRY,        "cancel_inquiry",	"105",	ZERO_PARAM,    "(105)cancel_inquiry"},
    {START_PAIR,            "pair",				"106",	ONE_PARAM,    "(106)pair<space><bt_address> \
    eg. pair 00:11:22:33:44:55"},
    {UNPAIR,                "unpair",			"107",	ONE_PARAM,    "(107)unpair<space><bt_address> \
    eg. unpair 00:11:22:33:44:55"},
    {INQUIRY_LIST,          "inquiry_list",		"108",	ZERO_PARAM,    "(108)inquiry_list"},
    {BONDED_LIST,           "bonded_list",		"109",	ZERO_PARAM,    "(109)bonded_list"},
    {GET_BT_STATE,          "get_state",		"110",	ZERO_PARAM,    "(110)get_state"},
    {GET_BT_NAME,           "get_bt_name",		"111",	ZERO_PARAM,    "(111)get_bt_name"},
    {GET_BT_ADDR,           "get_bt_address",	"112",	ZERO_PARAM,    "(112)get_bt_address"},
    {SET_BT_NAME,           "set_bt_name",		"113",	ONE_PARAM,    "(113)set_bt_name<space><bt name> \
    eg. set_bt_name AW_BT_TEST"},
    {BACK_TO_MAIN,          "main_menu",		"000",	ZERO_PARAM,    "(000)main_menu"},
};
/**
 * list of supported commands for Main Menu
 */
user_menu_list_t main_menu[] = {
    {GAP_OPTION,            "gap_menu",			"1",	ZERO_PARAM,   "(1)gap_menu"},
    {A2DP_SINK,             "a2dp_sink_menu",	"2",	ZERO_PARAM,   "(2)a2dp_sink_menu"},
    {HFP_CLIENT,            "hfp_client_menu",	"3",	ZERO_PARAM,   "(3)hfp_client_menu"},
    {A2DP_SOURCE,           "a2dp_source_menu",	"4",	ZERO_PARAM,   "(4)a2dp_source_menu"},
    {SPP_CLIENT_OPTION,     "spp_client_menu",	"5",	ZERO_PARAM,   "(5)spp_client_menu"},
    {MAIN_EXIT,             "exit",				"00",	ZERO_PARAM,   "(00)exit"},
};

/**
 * list of supported commands for A2DP_SINK Menu
 */
user_menu_list_t a2dp_sink_menu[] = {
    {CONNECT,               "connect",			"201",	ONE_PARAM,    "(201)connect<space><bt_address>"},
    {DISCONNECT,            "disconnect",		"202",	ONE_PARAM,    "(202)disconnect<space><bt_address>"},
    {PLAY,                  "play",				"203",	ONE_PARAM,    "(203)play<space><bt_address>"},
    {PAUSE,                 "pause",			"204",	ONE_PARAM,    "(204)pause<space><bt_address>"},
    {STOP,                  "stop",				"205",	ONE_PARAM,    "(205)stop<space><bt_address>"},
    {REWIND,                "rewind",			"206",	ONE_PARAM,    "(206)rewind<space><bt_address>"},
    {FASTFORWARD,           "fastforward",		"207",	ONE_PARAM,    "(207)fastforward<space><bt_address>"},
    {FORWARD,               "forward",			"208",	ONE_PARAM,    "(208)forward<space><bt_address>"},
    {BACKWARD,              "backward",			"209",	ONE_PARAM,    "(209)backward<space><bt_address>"},
#if 1
    {VOL_UP,                "volup",			"210",	ONE_PARAM,    "(210)volup<space><bt_address>"},
    {VOL_DOWN,              "voldown",			"211",	ONE_PARAM,    "(211)voldown<space><bt_address>"},
    {VOL_CHANGED_NOTI,      "volchangednoti",	"212",	ONE_PARAM,    "(212)volchangednoti<space><vol level>"},
#endif
    {BACK_TO_MAIN,          "main_menu",		"000",	ZERO_PARAM,   "(000)main_menu"},
};

/**
 * list of supported commands for A2DP_SOURCE Menu
 */
user_menu_list_t a2dp_source_menu[] = {
    {CONNECT,               "connect",			"401",	ONE_PARAM,    "(401)connect<space><bt_address>"},
    {DISCONNECT,            "disconnect",		"402",	ONE_PARAM,    "(402)disconnect<space><bt_address>"},
    {PLAY,                  "start",			"403",	ZERO_PARAM,   "(403)start"},
    {PAUSE,                 "suspend",			"404",	ZERO_PARAM,   "(404)suspend"},
    {STOP,                  "stop",				"405",	ZERO_PARAM,   "(405)stop"},
    {TRACK_CHANGE,          "trackchange",		"406",	ZERO_PARAM,   "(406)trackchange"},
    {SET_ABS_VOL,           "setabsolutevol",	"407",	ONE_PARAM,
            "(407)setabsolutevol<space><volstep>  eg: setabsolutevol 10 (range 0-15)"},
    {SEND_VOL_UP_DOWN,      "sendvolupdown",	"408",	ONE_PARAM,
            "(408)sendvolupdown<space><1/0>  eg: sendvolupdown 1 (1-up, 0-down)"},
    {ADDR_PLAYER_CHANGE,    "addrplayerchange",	"409",	ONE_PARAM,
            "(409)addrplayerchange<space><1/0>  eg: addrplayerchange 1 "},
    {AVAIL_PLAYER_CHANGE,   "availplayerchange",	"410",	ZERO_PARAM,   "(410)availplayerchange"},
    {BIGGER_METADATA,       "biggermetadata",	"411",	ZERO_PARAM,   "(411)biggermetadata"},
    {BACK_TO_MAIN,          "main_menu",		"000",	ZERO_PARAM,   "(000)main_menu"},
};

/**
 * list of supported commands for HFP_CLIENT Menu
 */
user_menu_list_t hfp_client_menu[] = {
    {CONNECT,               "connect",			"301",	ONE_PARAM,    "(301)connect<space><bt_address>"},
    {DISCONNECT,            "disconnect",		"302",	ONE_PARAM,    "(302)disconnect<space><bt_address>"},
    {CREATE_SCO_CONN,       "create_sco",		"303",	ONE_PARAM,    "(303)create_sco<space><bt_address>"},
    {DESTROY_SCO_CONN,      "destroy_sco",		"304",	ONE_PARAM,    "(304)destroy_sco<space><bt_address>"},
    {ACCEPT_CALL,           "accept_call",		"305",	ZERO_PARAM,   "(305)accept_call"},
    {REJECT_CALL,           "reject_call",		"306",	ZERO_PARAM,   "(306)reject_call"},
    {END_CALL,              "end_call",			"307",	ZERO_PARAM,   "(307)end_call"},
    {HOLD_CALL,             "hold_call",		"308",	ZERO_PARAM,   "(308)hold_call"},
    {RELEASE_HELD_CALL,     "release_held_call",	"309",	ZERO_PARAM,   "(309)release_held_call"},
    {RELEASE_ACTIVE_ACCEPT_WAITING_OR_HELD_CALL,	"release_active_accept_waiting_or_held_call",	"310",
      ZERO_PARAM,	"(310)release_active_accept_waiting_or_held_call"},
    {SWAP_CALLS,            "swap_calls",		"311",	ZERO_PARAM,   "(311)swap_calls"},
    {ADD_HELD_CALL_TO_CONF, "add_held_call_to_conference",	"312",	ZERO_PARAM,   "(312)add_held_call_to_conference"},
    {RELEASE_SPECIFIED_ACTIVE_CALL, "release_specified_active_call",	"313",	ONE_PARAM,
      "(313)release_specified_active_call<space><index of the call>"},
    {PRIVATE_CONSULTATION_MODE, "private_consultation_mode",	"314",	ONE_PARAM,
      "(314)private_consultation_mode<space><index of the call>"},
    {PUT_INCOMING_CALL_ON_HOLD, "put_incoming_call_on_hold",	"315",	ZERO_PARAM,   "(315)put_incoming_call_on_hold"},
    {ACCEPT_HELD_INCOMING_CALL, "accept_held_incoming_call",	"316",	ZERO_PARAM,   "(316)accept_held_incoming_call"},
    {REJECT_HELD_INCOMING_CALL, "reject_held_incoming_call",	"317",	ZERO_PARAM,   "(317)reject_held_incoming_call"},
    {DIAL,                  "dial",				"318",	ONE_PARAM,    "(318)dial<space><phone_number>"},
    {REDIAL,                "redial",			"319",	ZERO_PARAM,   "(319)redial"},
    {DIAL_MEMORY,           "dial_memory",		"320",	ONE_PARAM,    "320()dial_memory<space><memory_location>"},
    {START_VR,              "start_vr",			"321",	ZERO_PARAM,   "(321)start_vr"},
    {STOP_VR,               "stop_vr",			"322",	ZERO_PARAM,   "(322)stop_vr"},
    {QUERY_CURRENT_CALLS,   "query_current_calls",	"323",	ZERO_PARAM,   "(323)query_current_calls"},
    {QUERY_OPERATOR_NAME,   "query_operator_name",	"324",	ZERO_PARAM,   "(324)query_operator_name"},
    {QUERY_SUBSCRIBER_INFO, "query_subscriber_info",	"325",	ZERO_PARAM, "(325)query_subscriber_info"},
    {MIC_VOL_CTRL,          "mic_volume_control",	"326",	ONE_PARAM,   "(326)mic_volume_control<space><value>"},
    {SPK_VOL_CTRL,          "speaker_volume_control",	"327",	ONE_PARAM,   "(327)speaker_volume_control<space><value>"},
    {SEND_DTMF,             "send_dtmf",		"328",	ONE_PARAM,    "(328)send_dtmf<space><code>"},
    {DISABLE_NREC_ON_AG,    "disable_nrec_on_AG",		"329",	ZERO_PARAM,   "(329)disable_nrec_on_AG"},
    {BACK_TO_MAIN,          "main_menu",		"000",	ZERO_PARAM,   "(000)main_menu"},
};

/**
 * list of supported commands for SPP Client
 */

user_menu_list_t spp_client_idle_menu[] = {
    {CONNECT,              "connect",		"501",	ONE_PARAM,    "(501)connect<space><bt_address>"},
    {SPPC_SEND,            "send",			"502",	ONE_PARAM,    "(502)send<space><file_name>"},
    {SPPC_RECV,            "receive",		"503",	ONE_PARAM,    "(503)receive<space><file_name>"},
    {BACK_TO_MAIN,         "main_menu",		"000",	ZERO_PARAM,   "(000)main_menu"},
};

user_menu_list_t spp_client_connecting_menu[] = {
    {BACK_TO_MAIN,         "main_menu",		"000",	ZERO_PARAM,   "(000)Connecting... , Go back to main_menu"},
};

user_menu_list_t spp_client_connected_menu[] = {
    {SPPC_SEND,            "send",			"504",	ONE_PARAM,    "(504)send<space><file_name>"},
    {SPPC_RECV,            "receive",		"505",	ONE_PARAM,    "(505)receive<space><file_name>"},
    {DISCONNECT,           "disconnect",	"506",	ZERO_PARAM,   "(506)disconnect"},
    {BACK_TO_MAIN,         "main_menu",		"000",	ZERO_PARAM,   "(000)main_menu"},
};

user_menu_list_t spp_client_send_menu[] = {
    {DISCONNECT,           "disconnect",	"507",	ZERO_PARAM,   "(507)disconnect ( Sending file...)"},
    {BACK_TO_MAIN,         "main_menu",		"000",	ZERO_PARAM,   "(000)main_menu"},
};


user_menu_list_t spp_client_receive_menu[] = {
    {DISCONNECT,           "disconnect",	"508",	ZERO_PARAM,   "(508)disconnect ( Receiving file...)"},
    {BACK_TO_MAIN,         "main_menu",		"000",		ZERO_PARAM,   "(000)main_menu"},
};

user_menu_list_t spp_client_disconnected_menu[] = {
    {CONNECT,              "connect",		"509",	ONE_PARAM,    "(509)connect<space><bt_address>"},
    {SPPC_SEND,            "send",			"510",	ONE_PARAM,    "(510)send<space><file_name>"},
    {SPPC_RECV,            "receive",		"511",	ONE_PARAM,    "(511)receive<space><file_name>"},
    {BACK_TO_MAIN,         "main_menu",		"000",	ZERO_PARAM,   "(000)main_menu"},
};

#endif
