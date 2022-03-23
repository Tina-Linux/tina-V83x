/*

 */

#ifndef _RESPONSECODE_H
#define _RESPONSECODE_H

typedef enum{
    // Keep in sync with
    // frameworks/base/services/java/com/android/server/NetworkManagementService.java
    // 100 series - Requestion action was initiated; expect another reply
    // before proceeding with a new command.
    ACTION_INITIATED           = 100,
    INTERFACE_LIST_RESULT       = 110,
    TETHER_INTERFACE_LIST_RESULT = 111,
    TETHER_DNS_FWD_TGT_LIST_RESULT = 112,
    TTY_LIST_RESULT             = 113,
    TETHERING_STATS_LIST_RESULT  = 114,

    // 200 series - Requested action has been successfully completed
    COMMAND_OKAY               = 200,
    TETHER_STATUS_RESULT        = 210,
    IP_FWD_STATUS_RESULT         = 211,
    INTERFACE_GET_CFG_RESULT     = 213,
    SOFTAP_STATUS_RESULT        = 214,
    USB_RNDIS_STATUS_RESULT      = 215,
    INTERFACE_RX_COUNTER_RESULT  = 216,
    INTERFACE_TX_COUNTER_RESULT  = 217,
    INTERFACE_RX_THROTTLE_RESULT = 218,
    INTERFACE_TX_THROTTLE_RESULT = 219,
    QUOTA_COUNTER_RESULT        = 220,
    TETHERING_STATS_RESULT      = 221,
    DNS_PROXY_QUERY_RESULT       = 222,
    CLATD_STATUS_RESULT         = 223,
    INTERFACE_GET_MTU_RESULT     = 224,
    GET_MARK_RESULT             = 225,

    // 400 series - The command was accepted but the requested action
    // did not take place.
    OPERATION_FAILED           = 400,
    DNS_PROXY_OPERATION_FAILED   = 401,
    SERVICE_START_FAILED        = 402,
    SERVICE_STOP_FAILED         = 403,

    // 500 series - The command was not accepted and the requested
    // action did not take place.
    COMMAND_SYNTAX_ERROR = 500,
    COMMAND_PARAMETER_ERROR = 501,

    // 600 series - Unsolicited broadcasts
    INTERFACE_CHANGE                = 600,
    BANDWIDTH_CONTROL               = 601,
    SERVICE_DISCOVERY_FAILED         = 602,
    SERVICE_DISCOVERY_SERVICE_ADDED   = 603,
    SERVICE_DISCOVERY_SERVICE_REMOVED = 604,
    SERVICE_REGISTRATION_FAILED      = 605,
    SERVICE_REGISTRATION_SUCCEEDED   = 606,
    SERVICE_RESOLVE_FAILED           = 607,
    SERVICE_RESOLVE_SUCCESS          = 608,
    SERVICE_SET_HOSTNAME_FAILED       = 609,
    SERVICE_SET_HOSTNAME_SUCCESS      = 610,
    SERVICE_GET_ADDR_INFO_FAILED       = 611,
    SERVICE_GET_ADDR_INFO_SUCCESS      = 612,
    INTERFACE_CLASS_ACTIVITY         = 613,
    INTERFACE_ADDRESS_CHANGE         = 614,
}tRESPONSE_CODE;
#endif
