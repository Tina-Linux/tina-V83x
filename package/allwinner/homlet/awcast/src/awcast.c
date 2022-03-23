/* 
** $Id: awcast.c 741 2009-03-31 07:16:18Z weiym $
**
** awcast.c: Sample program for MiniGUI Programming Guide
**         Load and display a bitmap.
**
** Copyright (C) 2003 ~ 2017 FMSoft (http://www.fmsoft.cn).
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include <wifi_intf.h>
#include <aw_smartlinkd_connect.h>

#define CAPTION "AwCast"
#define STR_SSID_PREFIX "SSID  : "
#define STR_SSID_UNKNOWN STR_SSID_PREFIX "Unknown"

#define STR_CONN_STATUS_PREFIX "Status: " 
#define STR_CONN_STATUS_DISCONNECTED STR_CONN_STATUS_PREFIX "Disconnected" 
#define STR_CONN_STATUS_CONNECTED STR_CONN_STATUS_PREFIX "Connected" 
#define STR_CONN_STATUS_CONNECTEING STR_CONN_STATUS_PREFIX "Connecting" 
#define STR_CONN_STATUS_DHCP STR_CONN_STATUS_PREFIX "Getting IP" 

#define SCRENN_REVOLUTION_WIDTH (1920)
#define SCRENN_REVOLUTION_HEIGHT (1080)
#define LOGO_WIDTH (520)
#define LOGO_HEIGHT (200)
#define CONN_WIDTH  (224 * 3)
#define CONN_HEIGHT (45 * 3)
static void wifi_state_handle(struct Manager *w, int event_label);
static void GetNetworkInfo(void);

static char ssid_text [512];
static char wifi_status_text [256];
static RECT ssid_rc = {10, 100, 600, 400};
static RECT wifi_status_rc = {10, 100, 600, 400};




static BITMAP bmp_logo;
static BITMAP bmp_conn;
static PLOGFONT wifi_font;
static HWND hMainWnd;

/* Wifi */
enum wmgState wifi_state = DISCONNECTED;
const aw_wifi_interface_t *p_wifi_interface = NULL;
tKEY_MGMT key_mgmt;
int event_label = 0;
struct wifi_status wifi_mg_status = {
    .state = STATE_UNKNOWN,
    .ssid = {'\0'},
};


int onRead(char* buf,int length)
{
    pid_t pid;
    char ssid[256] = {0};
    char passwd[256] = {0};
    int status;

    if(length == THREAD_INIT){
        printf("if(length == THREAD_INIT)...\n");
    }
    else if(length == -1){  //RST
        printf("else if(length == -1)...\n");
    }else if(length == 0){   //EOF
        printf("server close the connection...\n");
        //exit(0);
        return THREAD_EXIT;

    }else {
        printf("length: %d\n",length);
        //printf_info((struct _cmd *)buf);
        struct _cmd* c = (struct _cmd *)buf;
        if(c->cmd == AW_SMARTLINKD_FAILED){
            printf("smartlinkd_demo:response failed\n");
            return THREAD_CONTINUE;
        }
        if(c->info.protocol == AW_SMARTLINKD_PROTO_FAIL){
            printf("proto scan fail");
            //connect_ap_finished_flag = 1;
            return THREAD_EXIT;
        }
        printf("cmd: %d\n",c->cmd);
        printf("pcol: %d\n",c->info.protocol);
        printf("ssid: %s\n",c->info.base_info.ssid);
        printf("pasd: %s\n",c->info.base_info.password);
        printf("security: %d\n",c->info.base_info.security);

        if(c->info.protocol == AW_SMARTLINKD_PROTO_AKISS)
            printf("radm: %d\n",c->info.airkiss_random);
        if(c->info.protocol == AW_SMARTLINKD_PROTO_COOEE){
            printf("ip: %s\n",c->info.ip_info.ip);
            printf("port: %d\n",c->info.ip_info.port);
        }
        if(c->info.protocol == AW_SMARTLINKD_PROTO_ADT){
            printf("adt get: %s\n",c->info.adt_str);
        }

        printf("\n:*********************************\n");
        printf("***Starting to connect ap!***\n");
        printf("*********************************\n");

/*
    if ((pid = fork()) < 0)
    {
        printf("smartlinkd_demo: fork 2 error\n");
    }
    else if(pid == 0)
*/
    {
        
        //event_label = rand();
        p_wifi_interface = aw_wifi_on(wifi_state_handle, event_label);
        if(p_wifi_interface == NULL){
            printf("wifi on failed \n");
            return -1;
        }
        
        if(wifi_state == NETWORK_CONNECTED){
            wmg_printf(MSG_INFO,"auto connected Successful  !!!!\n");
            wmg_printf(MSG_INFO,"==================================\n");
        }
        printf("c->info.protocol === %d \n",c->info.protocol);
        if(c->info.protocol == AW_SMARTLINKD_PROTO_XRSC ||
            c->info.protocol == AW_SMARTLINKD_PROTO_COOEE ||
            c->info.protocol == AW_SMARTLINKD_PROTO_AKISS)
        {
            printf("c->info.base_info.security == %d \n",c->info.base_info.security);
                //judge the encryption method of wifi
            if(c->info.base_info.security ==0)
                key_mgmt = 0;
            else if(c->info.base_info.security ==1)
                key_mgmt = 3;
            else if(c->info.base_info.security ==2)
                key_mgmt = 1;
            else if(c->info.base_info.security ==3)
                key_mgmt = 2;
            event_label++;
            p_wifi_interface->add_network(c->info.base_info.ssid,key_mgmt,c->info.base_info.password,event_label);
        }
        else
        {
            convert_adt_str_to_info(ssid, passwd, c->info.adt_str);
            event_label++;
            p_wifi_interface->connect_ap(ssid, passwd, event_label);
        }

		if(wifi_state == NETWORK_CONNECTED) {
            GetNetworkInfo();
			system("/bin/touch /tmp/connected_network");
			printf("finished: return exit\n");
        	return THREAD_EXIT;
		}
 
    }


/*
    if(waitpid(pid,&status,0) != pid)
    {
        printf("waitpid 2 error!\n");
    }

    if(WIFEXITED(status))
    {
        if(0 == WEXITSTATUS(status))
        {
            printf("*****************************************\n");
            printf("Smartlink demo:wifi connect ap: Successful!\n");
            printf("*****************************************\n");
            ConnWpaSupplicant(); 
        }
        else
        {
            printf("*****************************************\n");
            printf("Smartlink demo:wifi connect ap: failed!\n");
            printf("*****************************************\n");
        }
    }
    */

    }
    return THREAD_CONTINUE;

}

static void InvokeSmartLink() {
    if(aw_smartlinkd_init(0,onRead) == 0){
        aw_startxrsc();
    }
    
}

static void GetNetworkInfo() {
    if(p_wifi_interface != NULL && p_wifi_interface->get_status(&wifi_mg_status) >= 0) {
        wmg_printf(MSG_INFO,"wifi state:   %s\n",wmg_state_txt(wifi_mg_status.state));

            if(wifi_mg_status.state == NETWORK_CONNECTED) {
                wmg_printf(MSG_INFO,"connected ssid:%s\n",wifi_mg_status.ssid);
                sprintf(ssid_text, STR_SSID_PREFIX "%s", wifi_mg_status.ssid);
                printf("TJDEBUG connected ssid:%s\n", ssid_text);
            }
                
    }
}

static void wifi_state_handle(struct Manager *w, int event_label)
{
    wmg_printf(MSG_DEBUG,"event_label 0x%x\n", event_label);

    switch(w->StaEvt.state)
    {
         case CONNECTING:
         {
             wmg_printf(MSG_INFO,"Connecting to the network......\n");
             wifi_state = CONNECTING;
             strcpy (wifi_status_text, STR_CONN_STATUS_CONNECTEING);
             break;
         }
         case CONNECTED:
         {
             wmg_printf(MSG_INFO,"Connected to the AP\n");
             wifi_state = CONNECTED;
             // printf("TJDEBUG %s:%d, p_wifi_interface = %x\n", __func__, __LINE__, p_wifi_interface);
             GetNetworkInfo();
             strcpy (wifi_status_text, STR_CONN_STATUS_CONNECTED);
             start_udhcpc();
             break;
         }

         case OBTAINING_IP:
         {
             wmg_printf(MSG_INFO,"Getting ip address......\n");
             wifi_state = OBTAINING_IP;
             strcpy (wifi_status_text, STR_CONN_STATUS_DHCP);
             break;
         }

         case NETWORK_CONNECTED:
         {
             wmg_printf(MSG_INFO,"Successful network connection\n");
             wifi_state = NETWORK_CONNECTED;
             strcpy (wifi_status_text, STR_CONN_STATUS_CONNECTED);
			 system("/bin/touch /tmp/connected_network");
             break;
         }
        case DISCONNECTED:
        {
            wmg_printf(MSG_ERROR,"Disconnected,the reason:%s(%d)\n",wmg_event_txt(w->StaEvt.event), w->StaEvt.event);
            wifi_state = DISCONNECTED;
            strcpy (wifi_status_text, STR_CONN_STATUS_DISCONNECTED);
            strcpy (ssid_text, STR_SSID_UNKNOWN);
            if (w->StaEvt.event == 37) //WSE_AUTO_DISCONNECTED
                InvokeSmartLink();
            break;
        }
    }
    InvalidateRect (hMainWnd, &wifi_status_rc, TRUE);
    InvalidateRect (hMainWnd, &ssid_rc, TRUE);
}


static void make_wifi_text (void)
{
    const char* sys_charset = GetSysCharset (TRUE);
    const char* format;



    if (sys_charset == NULL)
        sys_charset = GetSysCharset (FALSE);

    

/*
    if (strcmp (sys_charset, FONT_CHARSET_GB2312_0) == 0 
            || strcmp (sys_charset, FONT_CHARSET_GBK) == 0
            || strcmp (sys_charset, FONT_CHARSET_GB18030_0) == 0) {
        format = "»¶Ó­À´µ½ MiniGUI µÄÊÀ½ç! Èç¹ûÄúÄÜ¿´µ½¸ÃÎÄ±¾, ÔòËµÃ÷ MiniGUI Version %d.%d.%d ¿ÉÔÚ¸ÃÓ²¼þÉÏÔËÐÐ!";
    }
    else if (strcmp (sys_charset, FONT_CHARSET_BIG5) == 0) {
        format = "»¶Ó­À´µ½ MiniGUI µÄÊÀ½ç! Èç¹ûÄúÄÜ¿´µ½¸ÃÎÄ±¾, ÔòËµÃ÷ MiniGUI Version %d.%d.%d ¿ÉÔÚ¸ÃÓ²¼þÉÏÔËÐÐ!";
    }
    else {
        format = "Welcome to the world of MiniGUI. \nIf you can see this text, MiniGUI Version %d.%d.%d can run on this hardware board.";
    }
*/
    //sprintf (welcome_text, format, MINIGUI_MAJOR_VERSION, MINIGUI_MINOR_VERSION, MINIGUI_MICRO_VERSION);


    wifi_font = CreateLogFont (NULL, "System", "ISO8859-1", 
                        FONT_WEIGHT_BOOK, FONT_SLANT_ROMAN, FONT_FLIP_NIL,
                        FONT_OTHER_AUTOSCALE, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE, 
                        32, 0);


    strcpy (ssid_text, STR_SSID_UNKNOWN);
    strcpy (wifi_status_text, STR_CONN_STATUS_DISCONNECTED);

}

static LRESULT LoadBmpWinProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    switch (message) {
        case MSG_CREATE:
            if (LoadBitmap (HDC_SCREEN, &bmp_logo, "/usr/res/aw.png"))
                return -1;
            if (LoadBitmap (HDC_SCREEN, &bmp_conn, "/usr/res/aw_conn.jpg"))
                return -1;
            make_wifi_text();
            return 0;

        case MSG_PAINT:

            hdc = BeginPaint (hWnd);
            SelectFont (hdc, wifi_font);

            int logo_x = g_rcScr.right / 2 - LOGO_WIDTH / 2;
            int logo_y = g_rcScr.bottom / 2 - LOGO_HEIGHT / 2;
            int conn_x = g_rcScr.right / 2 - CONN_WIDTH / 2;
            int conn_y = g_rcScr.bottom / 2 - CONN_HEIGHT / 2 + 100;
            // FillBoxWithBitmap (hdc, logo_x, logo_y, logo_x , logo_y, &bmp_logo);
            FillBoxWithBitmap (hdc, logo_x, logo_y, LOGO_WIDTH, LOGO_HEIGHT, &bmp_logo);
            FillBoxWithBitmap (hdc, conn_x, conn_y + LOGO_HEIGHT / 2 + 150, CONN_WIDTH, CONN_HEIGHT, &bmp_conn);


            int rect_wifi_status_lx = conn_x + CONN_WIDTH + 50;
            int rect_wifi_status_ly = conn_y + LOGO_HEIGHT / 2 + 150;
            int rect_wifi_status_rx = rect_wifi_status_lx + 400;
            int rect_wifi_status_ry = rect_wifi_status_ly + 50;

            int rect_ssid_lx = rect_wifi_status_lx;
            int rect_ssid_ly = rect_wifi_status_ry;
            int rect_ssid_rx = rect_ssid_lx + 400;
            int rect_ssid_ry = rect_ssid_ly + 200;



            SetRect (&wifi_status_rc, rect_wifi_status_lx, rect_wifi_status_ly , rect_wifi_status_rx,  rect_wifi_status_ry);
            SetRect (&ssid_rc, rect_ssid_lx , rect_ssid_ly, rect_ssid_rx, rect_ssid_ry);

            DrawText (hdc, ssid_text, -1, &ssid_rc, DT_LEFT | DT_WORDBREAK);
            DrawText (hdc, wifi_status_text, -1, &wifi_status_rc, DT_LEFT | DT_WORDBREAK);
            EndPaint (hWnd, hdc);
            return 0;

        case MSG_CLOSE:
            UnloadBitmap (&bmp_logo);
            DestroyMainWindow (hWnd);
            PostQuitMessage (hWnd);
            return 0;
    }

    return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

int ConnWpaSupplicant() {
    event_label = rand();
    p_wifi_interface = aw_wifi_on(wifi_state_handle, event_label);

    if(p_wifi_interface == NULL){
        printf("wifi on failed \n");
        return -1;
    }
    //FIXME : If the ap was connected before the awcast start, p_wifi_interface will be null
    // so we need to the trick.
    if (wifi_state == CONNECTED || wifi_state == OBTAINING_IP || wifi_state == NETWORK_CONNECTED) {
        GetNetworkInfo();
        InvalidateRect(hMainWnd, &ssid_rc, TRUE);
    } else if (wifi_state == DISCONNECTED) {
        InvokeSmartLink();
    }

}

int MiniGUIMain (int argc, const char* argv[])
{
    MSG Msg;

    MAINWINCREATE CreateInfo;

#ifdef _MGRM_PROCESSES
    JoinLayer(NAME_DEF_LAYER , "awcast" , 0 , 0);
#endif

    CreateInfo.dwStyle = WS_VISIBLE;
    CreateInfo.dwExStyle = WS_EX_NONE;
    CreateInfo.spCaption = CAPTION;
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(0);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = LoadBmpWinProc;
    /*
    CreateInfo.lx = SCRENN_REVOLUTION_WIDTH / 2 - LOGO_WIDTH / 2;
    CreateInfo.ty = SCRENN_REVOLUTION_HEIGHT / 2 - LOGO_HEIGHT / 2;
    CreateInfo.rx = LOGO_WIDTH + CreateInfo.lx;
    CreateInfo.by = LOGO_HEIGHT + CreateInfo.ty;
    */

    CreateInfo.lx = 0;
    CreateInfo.ty = 0;
    CreateInfo.rx = g_rcScr.right;
    CreateInfo.by = g_rcScr.bottom;

    CreateInfo.iBkColor = PIXEL_lightwhite;
    CreateInfo.dwAddData = 0;
    CreateInfo.hHosting = HWND_DESKTOP;
    
    hMainWnd = CreateMainWindow (&CreateInfo);
    
    if (hMainWnd == HWND_INVALID)
        return -1;

    
    


    ShowWindow (hMainWnd, SW_SHOWNORMAL);
	ShowCursor(0);
    ConnWpaSupplicant();
    

    while (GetMessage(&Msg, hMainWnd)) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    MainWindowThreadCleanup (hMainWnd);
    return 0;
}

#ifdef _MGRM_THREADS
#include <minigui/dti.c>
#endif

