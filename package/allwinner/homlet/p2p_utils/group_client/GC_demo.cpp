#include <string.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <string>
#include <map>

#include "p2pManager.h"
#include "WFDMessageQueue.h"
#include "wfd_log.h"

static const int WFD_CB_EVENT_QUIT_FROM_P2P  = 0x001;
static const int WFD_CB_EVENT_SETUP_FROM_P2P = 0x002;
static const int WFD_CB_EVENT_RESET_FROM_P2P = 0x003;
static const int WFD_CB_EVENT_RESET_FROM_MANAGER = 0x004;

static WFDMessageQueue* mMessageQueue = NULL;
static pthread_t mWFDCBOnPorcessId = 0;
static int mWFDCBOnProcessRuning = 0;
static int mWFDCBRuning = 0;
static char peerIP[24] = "";// = NULL;

using namespace std;
std::map<string, string> activeDevices; // <device mac, device name>


struct WFDMessage // asynchronous
{
    WFDMessage_COMMON_MEMBERS
    uintptr_t params[3]; // TODO
};

void* onProcessThread(void* arg)
{
    wfd_logd("onProcessThread start");

    WFDMessage msg;

    while(mWFDCBRuning)
    {
        if(WFDMessageQueueGetMessage(mMessageQueue, &msg) < 0)
        {
            wfd_loge("get message fail.");
            continue;
        }

        if(msg.messageId == WFD_CB_EVENT_RESET_FROM_MANAGER
             || msg.messageId == WFD_CB_EVENT_RESET_FROM_P2P)
        {
			AwP2PFind();
            continue;
        }
        else if(msg.messageId == WFD_CB_EVENT_SETUP_FROM_P2P)
        {
            WFDP2PCBEVENT wfdP2PCBEvent = (WFDP2PCBEVENT)msg.params[0];
            char* peerMac = (char*)msg.params[1];
            int peerCtrlPort = (int)msg.params[2];

	        char defaultGOIP[] = "192.168.49.1";
            if(wfdP2PCBEvent == WFD_CB_CONNECTED_CLIENT)
            {
                strcpy(peerIP,defaultGOIP);
            }
            else
            {
                int i = 0;
                for(; i < 10; i++) // wait 10s max
                {
                    AwP2PGetPeerIP(peerMac,peerIP);
                    if(!strcmp(peerIP, ""))
                    {
                        usleep(500*1000); // 500ms
                        continue;
                    }
                    break;
                }
                if (10 == i)
                {
                      wfd_loge("GetPeerIP() failed");
                      continue;
                }
            }
            wfd_logd("peerIP: %s, mac: %s, port: %d ",peerIP,peerMac,peerCtrlPort);

		    wfd_logd("AwP2PGroupRemove()");
		    AwP2PGroupRemove(NULL);

            continue;
        }
        else if(msg.messageId == WFD_CB_EVENT_QUIT_FROM_P2P)
        {
            mWFDCBRuning = 0;
            break;
        }
        else
        {
            wfd_logw("unknow message with id %d, ignore", msg.messageId);
            continue;
        }
    }

    wfd_logd("WFDCBThread end, mWFDCBRuning %d", mWFDCBRuning);

    return NULL;
}
void* getMacAndName(const char *elem1,char *mac,char *name)
{
	 char *pLeft, *pRight;

	 pLeft= strstr((char*)elem1,"p2p_dev_addr=");
	 strncpy(mac,pLeft+13,18);

     if(name)
     {
	    pLeft = strstr((char*)elem1,"name='");
		pRight = strstr(pLeft+6,"'");
		memset(name,0,50);
		strncpy(name,pLeft+6,pRight-pLeft-6);
	 }

	 wfd_logd("p2p mac= %s name= %s",mac,name);

     return NULL;
}
int WFDP2PCB(WFDP2PCBEVENT wfdP2PCBEvent,const char *elem1,const int elem2)
{
    wfd_logd("wfdP2PCB: %d, %s, %d", wfdP2PCBEvent, elem1, elem2);

    if(wfdP2PCBEvent == WFD_CB_QUITTED)
    {
        WFDMessage msg;
        memset(&msg, 0, sizeof(WFDMessage));
        msg.messageId = WFD_CB_EVENT_QUIT_FROM_P2P;
        WFDMessageQueuePostMessage(mMessageQueue, &msg);
    }
    else if (wfdP2PCBEvent == WFD_CB_DISCONNECTED)
    {
        WFDMessage msg;
        memset(&msg, 0, sizeof(WFDMessage));
        msg.messageId = WFD_CB_EVENT_RESET_FROM_P2P;
        WFDMessageQueuePostMessage(mMessageQueue, &msg);
    }
	else if (wfdP2PCBEvent == WFD_CB_DEVICE_FOUND)
    {
       	char p2p_mac[18];
        char p2p_name[50];
		getMacAndName(elem1,p2p_mac,p2p_name);
		wfd_logd("p2p device: %s active, info: %s ",p2p_name, elem1);
        activeDevices[p2p_mac] = p2p_name ;
    }
	else if (wfdP2PCBEvent == WFD_CB_DEVICE_LOST)
    {
       // updata activeDevices and stop commuicate when the GC lost.
	   char p2p_mac[18];
	   getMacAndName(elem1,p2p_mac,NULL);
	   wfd_logd("p2p device: %s lost, info: %s",activeDevices[p2p_mac].c_str(),elem1);
	   activeDevices.erase(p2p_mac);

    }
    else
    {
        WFDMessage msg;
        memset(&msg, 0, sizeof(WFDMessage));
        msg.messageId = WFD_CB_EVENT_SETUP_FROM_P2P;
        msg.params[0] = (uintptr_t)wfdP2PCBEvent;
        msg.params[1] = (uintptr_t)elem1; // mac
        msg.params[2] = (uintptr_t)elem2; // peerCtrlPort
        WFDMessageQueuePostMessage(mMessageQueue, &msg);
    }

    return 0;
}


int main(int argc, char *argv[])
{
	wfd_logd("H6 tina p2p demo start");

    int ret = -1;
    char gcElem[] = "0006001100000032";
	mMessageQueue = WFDMessageQueueCreate(64, "p2pManager");

	ret = AwP2PCreate((wfdP2PCallback) WFDP2PCB);
	if(0 == ret)
	{
	   mWFDCBRuning = 1;
       if(pthread_create(&mWFDCBOnPorcessId, NULL, onProcessThread, NULL) == 0)
         mWFDCBOnProcessRuning = 1;
       else
         mWFDCBOnProcessRuning = 0;
	}
    AwP2PSetElem(gcElem);
	AwP2PSetDisplay(1);
	AwP2PFind();

	wfd_logd("wait to be connectted then do communication");
	usleep(30*1000*1000); // do communication

    if(mWFDCBOnProcessRuning != 0)
    {
        pthread_join(mWFDCBOnPorcessId, NULL);
        mWFDCBOnProcessRuning = 0;
    }

    if(mMessageQueue)
    {
        WFDMessageQueueDestroy(mMessageQueue);
        mMessageQueue = NULL;
    }

	wfd_logd("H6 tina p2p group client demo over");

    return 0;
}




