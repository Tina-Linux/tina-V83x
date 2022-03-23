#ifndef P2P_MANAGER_H
#define P2P_MANAGER_H

#include "wpa_cli.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SET_DEVICE_TYPE_CMD  "set device_type"
#define SET_DEVICE_NAME_CMD  "set device_name"
#define WFD_SUBELEM_SET_CMD  "wfd_subelem_set 0"
#define SET_WIFI_DISPLAY_CMD "set wifi_display"

#define P2P_FIND_CMD       "p2p_find"
#define P2P_STOP_FIND      "p2p_stop_find"
#define P2P_PEERS          "p2p_peers"
#define P2P_PEER           "p2p_peer"
#define P2P_GROUP_REMOVE   "p2p_group_remove"
#define P2P_FLUSH          "p2p_flush"
#define P2P_CONNECT_FIND   "p2p_connect"
#define P2P_DISCONNECT     "disconnect"

#define QUIT "quit"

// set p2p subElem
// Group owner  should set "000600101c440032"
// Group client should set "0006001100000032"
int AwP2PSetElem(char *subElem);

// set this p2p device whether show or not
// Group owner  should set show whit 1
// Group client should set whit 0
int AwP2PSetDisplay(int mDisp);

// search p2p device persistent untill do stop p2p find
// when find p2p device, then will callback info with wfdP2PCallback
int AwP2PFind();

// stop search p2p device
//atomatic do stop p2p find after do p2p connect, so you neednot care this unless you want do it.
int AwP2PStopFind();

//List P2P device addresses of all the P2P peers have found;
//need to do;
//int AwP2PPeers();

//Fetch information about a known P2P peer;
// mac: peer mac address
//reply: char arry store reply
//reply_len: sizeof char arry reply
//need to do;
//int AwP2PPeer(char* mac,char* reply, size_t reply_len);

// connect p2p device with mac
// mac: peer mac address
int AwP2PConnect(const char* mac);

// Disconnect p2p device
int AwP2PDisConnect();

// get peer ip
// mac: peer mac address
char* AwP2PGetPeerIP (char const* mac,char *ip);

//Terminate a P2P group
//[ifname] = remove P2P group interface (terminate group if GO)
//ifname default get by p2pManager
int AwP2PGroupRemove(char* ifname);

//Flush P2P peer table and state
int AwP2PFlush();

// close p2pManager component
int AwP2PClose();

// p2pManager Useage:
// sample:
//     1. AwP2PCreate() init p2pManager componnent  with wfdP2PCallback
//     2. do AwP2PSetElem() and AwP2PSetDisplay() base role(owner/client)
//     3. do AwP2PFind() search p2p device
//     4. then p2pManager will callbake deviceInfo with wfdP2PCallback(WFD_CB_DEVICE_FOUND),
//       if p2p device lost will wfdP2PCallback(WFD_CB_DEVICE_LOST)
//       deviceInfo sample: P2P-DEVICE-FOUND 32:74:96:e0:a1:87 p2p_dev_addr=32:74:96:e0:a1:87\
//                          pri_dev_type=10-0050F204-5 name='Honor 8 Lite' config_methods=0x188\
//                          dev_capab=0x25 group_capab=0x0 wfd_dev_info=0x00101c440032 vendor_elems=1 new=1
//       application should record active_p2pdevie when WFD_CB_DEVICE_FOUND
//                          and remove inactive_p2pdevice when WFD_CB_DEVICE_LOST
//       you can get p2p devieinfo from deviceInfo, and do what you like as your role
//     5.as group client: wait group owner do connct; if group owner do connect this device and success link
//                        then p2pManager will callbake with key WFD_CB_CONNECTED_CLIENT
//                        client device can get peer ip with AwP2pGetPeerIP() then do communication.
//     5.as group owner: do AwP2PConnect() with mac (choice the one which you want to connect and get mac from deviceInfo)
//                       then p2pManager will connect to p2p_device
//                       when p2p linked, whill callback with key WFD_CB_CONNECTED_GO
//                       owner device can get peer ip with AwP2pGetPeerIP() then do communication.
//     6.ternimation: AwP2PFlush Flush peer table and state
//                    AwP2PGroupRemove remove p2p net interface
//                    AwP2PDisConnect discannect p2p device
//                    AwP2PClose close p2pManager componnent
int AwP2PCreate(wfdP2PCallback callback);

#ifdef __cplusplus
}
#endif

#endif /* P2P_MANAGER_H */



