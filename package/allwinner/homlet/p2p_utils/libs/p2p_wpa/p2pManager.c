#include <string.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

#include <wpa_cli.h>
#include <p2pManager.h>
#include <wfd_log.h>

#define ARP_NODEFILE "/proc/net/arp"
static pthread_t mP2PThreadId = 0;
static int mP2PComponentThreadCreated = 0;
static int mWFDCBQuit = 0;
typedef struct p2pCtx
{
   void* ctrl_iface;
   wfdP2PCallback callback;
}p2pCtx;

char* AwP2PGetPeerIP (char const* mac,char* mPeerIP)
{
    FILE* fd = NULL;
    char (*tables)[32] = (char (*)[32])malloc(32*6);
    memset(tables, '\0', 32*6);

    fd = fopen(ARP_NODEFILE, "r");
    if(fd == NULL)
    {
        wfd_loge("open %s fail, error: %s", ARP_NODEFILE, strerror(errno));
        return NULL;
    }

    rewind(fd);

    char* line = (char*)malloc(256);
    memset(line, '\0', 256);
    int length = 0;
    int ignore = 1;
    int end = 0;
    int8_t tmp = '\0';

    while(!end)
    {
        tmp = fgetc(fd);
        if(!feof(fd) && tmp != EOF && tmp != '\n')
        {
            line[length++] = tmp;
        }
        else //get a line
        {
            if(length > 0)
                wfd_logd("this line: %s", line);

            if(!ignore && length > 0)
            {
                if(sscanf(line, "%256s %256s %256s %256s %256s %256s", tables[0], tables[1], tables[2], tables[3], tables[4], tables[5]) == 6)
                {
                    if(strstr(tables[5], "p2p"))
                    {
                        /*
                        if((!strncmp(tables[3], mac, 12) && !strcmp(&tables[3][13], &mac[13])) ||
                            (!strncmp(tables[3], mac, 10) && !strcmp(&tables[3][11], &mac[11])) ||
                            (!strncmp(tables[3], mac, 1) && !strncmp(&tables[3][2], &mac[2], 14))) //TODO
                        */
                        {
                            memcpy(mPeerIP, tables[0], strlen(tables[0]));
                            end = 1;
                        }
                    }
                }
            }
            if(feof(fd) || tmp == EOF)
            {
                end = 1;
            }
            length = 0;
            memset(line, '\0', 256);
            memset(tables, '\0', 32*6);
            ignore = 0;
        }
    }

    if(line)
    {
        free(line);
        line = NULL;
    }
    if(fd)
    {
        fclose(fd);
        fd = NULL;
    }
    if(tables)
    {
        free(tables);
        tables = NULL;
    }

    return mPeerIP;
}

//	mac = "32:74:96:e0:a1:87";
int AwP2PConnect(const char* mac)
{
	char cmdString[4096] = "";
    memset(cmdString, '\0', sizeof(cmdString));
	sprintf(cmdString, "%s %s %s %s", P2P_CONNECT_FIND, mac, "pbc persistent go_intent=15:", mac);
    wfd_logd("cmd: %s", cmdString);

	return  wpa_cli_cmd_handler(cmdString);
}

// Disconnect p2p device
int AwP2PDisConnect()
{
	char cmdString[4096] = "";
    memset(cmdString, '\0', sizeof(cmdString));
    sprintf(cmdString, "%s", P2P_DISCONNECT);
    wfd_logd("cmd: %s", cmdString);

    return wpa_cli_cmd_handler(cmdString);
}

int AwP2PFind()
{
	char cmdString[4096] = "";
    memset(cmdString, '\0', sizeof(cmdString));
    sprintf(cmdString, "%s", P2P_FIND_CMD);
    wfd_logd("cmd: %s", cmdString);

	return wpa_cli_cmd_handler(cmdString);
}

// stop search p2p device
int AwP2PStopFind()
{
	char cmdString[4096] = "";
    memset(cmdString, '\0', sizeof(cmdString));
    sprintf(cmdString, "%s", P2P_STOP_FIND);
    wfd_logd("cmd: %s", cmdString);

    return wpa_cli_cmd_handler(cmdString);

}

//List P2P device addresses of all the P2P peers have found;
int AwP2PPeers()
{
	char cmdString[4096] = "";
    memset(cmdString, '\0', sizeof(cmdString));
    sprintf(cmdString, "%s",P2P_PEERS);
    wfd_logd("cmd: %s", cmdString);

    return wpa_cli_cmd_handler(cmdString);

}

//Fetch information about a known P2P peer;
// mac: peer mac address
int AwP2PPeer(char* mac,char* reply, size_t reply_len)
{
	char cmdString[4096] = "";
    memset(cmdString, '\0', sizeof(cmdString));
    sprintf(cmdString, "%s %s",P2P_PEER,mac);
    wfd_logd("cmd: %s", cmdString);
    return wpa_cli_cmd_with_reply(cmdString, reply, &reply_len);
}

//"0006001100000032" ,"000600101c440032"
int AwP2PSetElem(char *subElem)
{
	char cmdString[4096] = "";
	memset(cmdString, '\0', sizeof(cmdString));
	sprintf(cmdString, "%s %s", WFD_SUBELEM_SET_CMD,subElem);
	wfd_logd("cmd: %s", cmdString);

	wpa_cli_cmd_handler(cmdString);
    return 0;
}

//mDisp = 0 or 1
int AwP2PSetDisplay(int mDisp)
{
	char cmdString[4096] = "";
	memset(cmdString, '\0', sizeof(cmdString));
	sprintf(cmdString, "%s %d", SET_WIFI_DISPLAY_CMD, mDisp);
	wfd_logd("cmd: %s", cmdString);

	wpa_cli_cmd_handler(cmdString);
    return 0;
}

//Terminate a P2P group
//<ifname> = remove P2P group interface (terminate group if GO)
int AwP2PGroupRemove(char* ifname)
{
	char cmdString[4096] = "";
    memset(cmdString, '\0', sizeof(cmdString));
    sprintf(cmdString, "%s %s",P2P_GROUP_REMOVE,
		ifname ? ifname : wpa_cli_get_ifname(NULL));
    wfd_logd("cmd: %s", cmdString);

    return wpa_cli_cmd_handler(cmdString);
}

//Flush P2P peer table and state
int AwP2PFlush()
{
	char cmdString[4096] = "";
    memset(cmdString, '\0', sizeof(cmdString));
    sprintf(cmdString, "%s", P2P_FLUSH);
    wfd_logd("cmd: %s", cmdString);

    return wpa_cli_cmd_handler(cmdString);
}

int AwP2PClose()
{
	mWFDCBQuit = 1;

	char cmdString[4096] = "";
	memset(cmdString, '\0', sizeof(cmdString));
	sprintf(cmdString, "%s", "quit");
	wfd_logd("cmd: %s", cmdString);
	wpa_cli_cmd_handler(cmdString);

    if(mP2PComponentThreadCreated != 0)
    {
        pthread_join(mP2PThreadId, NULL);
        mP2PComponentThreadCreated = 0;
    }

	wfd_logd("P2PComponentThread closed");

    return 0;
}

void* P2PComponentThread(void* para)
{
    wfd_logd("P2PComponentThread");

	p2pCtx *p = para ;
    prctl(PR_SET_NAME, (unsigned long)"P2PComponentThread", 0, 0, 0);

    char wpa_cli_cmd[]    = "wpa_cli";
    char wpa_cli_ifname[] = "-ip2p0";
    char *argv[] = {wpa_cli_cmd, wpa_cli_ifname, (char*)p->ctrl_iface};
    int  argc = 3;
    int ret = wpa_cli_main(argc, argv, (wfdP2PCallback)p->callback);

    wfd_logd("P2PComponentThread, ret %d", ret);

    return NULL;
}

int AwP2PCreate(wfdP2PCallback callback)
{
    char ctrl_iface_dir_default[] = "-p/var/run/wpa_supplicant" ;
	p2pCtx para;
	para.ctrl_iface = ctrl_iface_dir_default;
	para.callback = callback;

    if(pthread_create(&mP2PThreadId, NULL, P2PComponentThread, &para) == 0)
        mP2PComponentThreadCreated = 1;
    else
        mP2PComponentThreadCreated = 0;

    wfd_logd("wait connection is ready...");

    while( mWFDCBQuit == 0
           &&(!wpa_cli_get_ctrl_conn() || !wpa_cli_get_mon_conn()));

    wfd_logd("connection is ready");

	return 0;
}



