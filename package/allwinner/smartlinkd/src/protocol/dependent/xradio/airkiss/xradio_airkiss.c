//平台相关头文件



#define LOG_TAG "Airkiss.c"


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>           // close()
#include <string.h>           // strcpy, memset()
#include <getopt.h>

#include <netinet/ip.h>       // IP_MAXPACKET (65535)
#include <sys/time.h>
#include <net/ethernet.h>     // ETH_P_ALL
#include <net/if.h>	      // struct ifreq
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <linux/if_packet.h>
#include <sys/ioctl.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>
#include "radiotap.h"
#include "radiotap_iter.h"
#include <errno.h>

#include <pthread.h>

//包含AirKiss头文件
#include "airkiss.h"
#include <Airkiss_app.h>
#include <sm_link_manager.h>
//#define LOG_MSG printf
static char g_log_enable = 0;



static char *frame_buf;
static int  frame_len;

static pthread_t thread_monitor;
static char g_thread_monitor_running = 1;
static char server_state[5]="yes";


#if AIRKISS_ENABLE_CRYPT
static char g_aes_key[512] = "1234567890123456";
#endif

const unsigned char bc_mac[6]= {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

static char ifName[IFNAMSIZ];


//当前监听的无线信道
static char g_cur_channel = 1;
static char g_config_state;

static char g_config_timeout;
static int f_send_finished;

//AirKiss过程中需要的RAM资源，完成AirKiss后可以供其他代码使用
airkiss_context_t akcontex;
static airkiss_result_t airkiss_result;


//另一种更节省资源的使用方法，通过malloc动态申请RAM资源，完成后利用free释放，需要平台支持
//示例：
//airkiss_context_t *akcontexprt;
//akcontexprt =
//(airkiss_context_t *)os_malloc(sizeof(airkiss_context_t));

//定义AirKiss库需要用到的一些标准函数，由对应的硬件平台提供，前三个为必要函数
const airkiss_config_t akconf = {
    ( airkiss_memset_fn )&memset,
    ( airkiss_memcpy_fn )&memcpy,
    ( airkiss_memcmp_fn )&memcmp,
    ( airkiss_printf_fn )&printf

};

 void XR_SYSTEM( const char *cmd )
{
    LOG_DEBUG( "shell:  %s", cmd );
    system( cmd );
}

void wifi_do_scan( void )
{
    char cmdstr[250];

    sprintf( cmdstr, "rm -rf %s", SCAN_FILE_PATH );
    XR_SYSTEM( cmdstr );
    sprintf( cmdstr, "iw dev wlan0 scan >%s", SCAN_FILE_PATH );
    XR_SYSTEM( cmdstr );
}


void airkiss_wifi_set_channel( uint8_t channel )
{
    char cmdstr[256];

    sprintf( cmdstr, "iw dev %s set channel %d", ifName, channel );
    XR_SYSTEM( cmdstr );

}

void airkiss_wifi_disable( const char* ifName )
{
    char cmdstr[250];

//    XR_SYSTEM( "killall wpa_supplicant" );
    sprintf( cmdstr, "ifconfig %s down", ifName );
    XR_SYSTEM( cmdstr );

}

void airkiss_wifi_enable( const char* ifName )
{
    char cmdstr[250];

    sprintf( cmdstr, "ifconfig %s up", ifName );
    XR_SYSTEM( cmdstr );
    usleep( 500000 );
}

static int doPopenCmd( const char *cmd, char *out, int len )
{
    FILE *pstream;
    char dobuf[1024] = {0};

	XR_SYSTEM("touch /tmp/cmdlog.txt");
	XR_SYSTEM( "chmod 777 /tmp/cmdlog.txt");

	//printf("doPopenCmd 1: %s\n",cmd);
    sprintf( dobuf,"%s > /tmp/cmdlog.txt",cmd );
    XR_SYSTEM( dobuf );

	//XR_SYSTEM( "cat /tmp/cmdlog.txt");

    pstream =fopen( "/tmp/cmdlog.txt","r" );
    if( !pstream ) {
        perror( "open pstream fail!" );
        return ERROR;
    }
    fgets( out,len - 1,pstream );
    fclose( pstream );

	//printf("out:%s\n",out);

    XR_SYSTEM( "rm /tmp/cmdlog.txt" );
    return SUCCESS;

}

void wifi_monitor_exit( void )
{
    char cmdstr[200];

    memset( cmdstr, 0, sizeof( char )*200 );

    sprintf( cmdstr,"ifconfig %s down\n",ifName );
    XR_SYSTEM( cmdstr );

    sprintf( cmdstr, "iw dev %s set type station", ifName );
    XR_SYSTEM( cmdstr );

    sprintf( cmdstr,"ifconfig %s up\n",ifName );
    XR_SYSTEM( cmdstr );

}

void wifi_monitor_setup( void )
{
    char cmdstr[200];

    memset( cmdstr, 0, sizeof( char )*200 );

    sprintf( cmdstr,"ifconfig %s down\n",ifName );
    XR_SYSTEM( cmdstr );

    sprintf( cmdstr, "iw dev %s set type monitor", ifName );
    XR_SYSTEM( cmdstr );

    sprintf( cmdstr,"ifconfig %s up\n",ifName );
    XR_SYSTEM( cmdstr );

}


void airkiss_clear_wpas()
{
//    system( "killall wpa_supplicant 2>/dev/null" ); //TODO and also clear the conf file

}

static void print_usage( void )
{
    printf( "\n\nUsage: \nairkiss \t-i<ifName> -c<wpa_cfgfile> -a <ak_key> -t <timeout>\n\t\t\t\t[-dm] [-v]" );
    printf( "\n\nOPTIONS:\n\t" );
    printf( "-i = interface name\n\t" );
    printf( "-a = airkiss key\n\t" );
    printf( "-m = enable debug message, -d = more message.\n\t" );
    printf( "-t = timeout (seconds)\n\t" );
	printf( "-s,Whether to connect smartlinkd server(yes or no),default yes\n\t");
    printf( "-v = version\t\n\n" );
    printf( "example:\n\t" );
    printf( "airkiss -i wlan0 -c ./wpa_conf -a 1234567890123456 -d\n" );
    return;
}

uint8_t parse_opt( int argc, char **argv )
{
    g_log_enable = LOGDEF_NONE;
    strcpy( ifName, IF_NAME );
    g_config_timeout = T_MONITOR_MODE;


    char* const short_options = "mdvha:i:t:s:";


    int c;
    while( ( c = getopt ( argc, argv, short_options ) ) != -1 ) {
        switch ( c ) {
            case 'm':
                g_log_enable = LOGDEF_MSG;;
                break;
            case 'd':
                g_log_enable = LOGDEF_DEBUG;
                break;
#if	AIRKISS_ENABLE_CRYPT
            case 'a':
                strcpy( g_aes_key, optarg );
                LOG_MSG( "g_aes_key %s", g_aes_key );
                break;
#endif
            case 'i':
                strcpy( ifName, optarg );
                LOG_MSG( "ifName %s", ifName );
                break;
			case 's':
				strncpy(server_state,optarg,sizeof(server_state));
				break;
            case 't':	// timeout
                g_config_timeout = atoi( optarg );
                break;
            case 'v':
                printf( "%s -- %s\n", argv[0], PROGRAM_VERSION );
                exit( EXIT_SUCCESS );
            case 'h':
            default: /* '?' */
                print_usage();
                return -EINVAL;

        }
    }

    if( g_log_enable ) {

        LOG_MSG( "========option parse========\n" );
#if AIRKISS_ENABLE_CRYPT
        LOG_MSG( "g_aes_key = %s\n", g_aes_key );
#endif
        LOG_MSG( "timeout = %d\n", g_config_timeout );
        LOG_MSG( "Whether to connect smartlinkd server = %s\n",  server_state );
		LOG_MSG( "ifName = %s\n",  ifName );
        LOG_MSG( "========================\n" );
    }
    return 0;
}


/*
 * 平台相关定时器中断处理函数，100ms中断后切换信道
 */
static void change_channel( void )
{
    //切换信道
    if ( g_cur_channel > 13 )
        g_cur_channel = 1;
    else
        g_cur_channel++;
    airkiss_wifi_set_channel( g_cur_channel );
    airkiss_change_channel( &akcontex ); //清缓存
    LOG_MSG( "switch to channel %d\n", g_cur_channel );
}

/*
 * airkiss成功后读取配置信息，平台无关，修改打印函数即可
 */
static void airkiss_finish( void )
{
    int8_t err;
    char buffer[256];

    err = airkiss_get_result( &akcontex, &airkiss_result );
    if ( err == 0 ) {
        LOG_MSG( "airkiss_get_result() ok!" );
        sprintf( buffer,
                 "ssid = \"%s\", pwd = \"%s\", ssid_length = %d, pwd_length = %d, random = 0x%02x\r\n",
                 airkiss_result.ssid,
                 airkiss_result.pwd,
                 airkiss_result.ssid_length,
                 airkiss_result.pwd_length,
                 airkiss_result.random );
    } else {
        printf( "airkiss_get_result() failed !\n" );
    }
}



int open_socket( void )
{
    int sock_fd;
    int return_value = 0;
    int ret;

    sock_fd = socket( PF_PACKET, SOCK_RAW, htons( ETH_P_ALL ) );
    if( sock_fd < 0 ) {
        perror("create socket failed");
        return_value = -1;
        return( return_value );
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //++  entry 混杂模式

    struct ifreq ifr;
    int sockopt = 1;


    memset( &ifr, 0, sizeof( ifr ) );

    strncpy( ifr.ifr_name, ifName, strlen( ifName ) );

    ret = ioctl( sock_fd, SIOCGIFFLAGS, &ifr );
    if( ret < 0 ) {
        perror("Get nic mode failed");
        return_value = -1;
        goto socket_exit;
    }

    ifr.ifr_flags |= IFF_PROMISC;
    ret = ioctl( sock_fd, SIOCSIFFLAGS, &ifr );
    if( ret < 0 ) {
        perror("Set nic promisc mode failed:");
        return_value = -1;
        goto socket_exit;
    }

    /* allow the socket to be reused - incase connection is closed prematurely */
    if ( setsockopt( sock_fd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof( sockopt ) ) < 0 ) {
        perror( "setsockopt(SO_REUSEADDR)" );
        goto socket_exit;
    }

    LOG_MSG( "Enable promisc success" );
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    struct sockaddr_ll ll;
    memset( &ll, 0, sizeof( ll ) );
    ll.sll_family = PF_PACKET;
    ll.sll_protocol = htons( ETH_P_ALL );
    ll.sll_ifindex = if_nametoindex( ifName );
    ret = bind( sock_fd, ( struct sockaddr * )&ll, sizeof( ll ) );
    if( ret < 0 ) {
        fprintf( stderr, "Bind socket raw to %s failed: %s\n", ifName, strerror( errno ) );
        return_value = -1;
        goto socket_exit;
    }

    return sock_fd;

socket_exit:
    close( sock_fd );
    return( return_value );
}

int wifi_recv_raw_package( int fd )
{
    int len, ret;
    fd_set rfds;
    struct timeval tv;

    FD_ZERO( &rfds );
    FD_SET( fd, &rfds );

    tv.tv_sec = 0;
    tv.tv_usec = T_SELECT_TIMEOUT;

    ret = select( fd + 1, &rfds, NULL, NULL, &tv );
    if ( ret <= 0 ) {
        // fprintf( stderr, "select timeout or failed:" );
        return -1;
    }

    memset( frame_buf, 0, IP_MAXPACKET );
    len = recv( fd, frame_buf, IP_MAXPACKET, 0 );
    if ( len <= 0 ) {
        fprintf( stderr, "recv() failed:" );
        //Something weird happened
        return -2;
    }
    frame_len = len;


    return 0;
}


int time_calc_usec( struct timeval *start )
{
    struct timeval now;
    int time_us;

    gettimeofday( &now, NULL );

    time_us = ( now.tv_sec - start->tv_sec ) * 1000000
              + ( now.tv_usec - start->tv_usec );

    return time_us;
}

static void ProcessPacket( u8 *buffer, int size )
{
    unsigned char	*da;
    unsigned char	*sa;
    unsigned char	*bssid;
    u8	to_fr_ds;
    u8	type;
    airkiss_status_t state;

    /*	80211 header format
	ver:	2bit
	type:	2bit
	subtype:	4bit
	tods:	1bit
	frds:	1bit
	other:	6bit		*/

    type = *buffer & TYPE_MASK;


//	subtype = (*buffer & SUBTYPE_MASK) >> 4;
    if ( ( type != TYPE_DATA ) || ( size < 21 ) )
        return ;

    to_fr_ds = *( buffer + 1 ) & FRTODS_MASK;
    if ( to_fr_ds == 1 ) {
        sa = GetAddr2Ptr( buffer );
        bssid = GetAddr1Ptr( buffer );
        da = GetAddr3Ptr( buffer );
    } else if ( to_fr_ds == 2 ) {
        sa = GetAddr3Ptr( buffer );
        bssid = GetAddr2Ptr( buffer );
        da = GetAddr1Ptr( buffer );
    } else {
        sa = NULL;
        da = NULL;
    }
	if(da != NULL)
    LOG_MSG( "----data len=%d---- da:%x %x %x %x %x %x!\n", size, da[0], da[1],da[2],da[3],da[4],da[5]);


    if ( ( da ) && ( memcmp( da, bc_mac, ETH_ALEN ) == 0 ) ) {
        //将网络帧传入airkiss库进行处理
        state = airkiss_recv( &akcontex, buffer, size );
        //判断返回值，确定是否锁定信道或者读取结果
        if ( state == AIRKISS_STATUS_CHANNEL_LOCKED ) {
            g_config_state = AIRKISS_STATUS_CHANNEL_LOCKED;
            LOG_MSG( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" );
            LOG_MSG( "AIRKISS_STATUS_CHANNEL_LOCKED" );
            LOG_MSG( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" );

            LOG_MSG( "bssid=[%02x:%02x:%02x:%02x:%02x:%02x] ch=%d\n",
                     ( u8 )bssid[0], ( u8 )bssid[1], ( u8 )bssid[2],
                     ( u8 )bssid[3], ( u8 )bssid[4], ( u8 )bssid[5],g_cur_channel );
        } else if ( state == AIRKISS_STATUS_COMPLETE ) {
            airkiss_finish();
            g_config_state = AIRKISS_STATUS_COMPLETE;
            LOG_MSG( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" );
            LOG_MSG( "AIRKISS_STATUS_COMPLETE" );
            LOG_MSG( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" );

        } else if( state == AIRKISS_STATUS_CONTINUE )
        {
            LOG_MSG( "AIRKISS_STATUS_CONTINUE \n" );
        } else {
            //g_config_state = state;
            LOG_MSG( "airkiss_recv para error \n" );
        }
    }
    return;
}


void *airkiss_sniffer_thread_func( void *arg )
{
    int listen_sock;
    struct timeval start;
    int state, ret;

    int radio_tap_len, scan_time;

    struct ieee80211_radiotap_iterator iter;

   // arg = arg;

    g_thread_monitor_running = 1;

    gettimeofday( &start, NULL );
    listen_sock = open_socket();
    scan_time = T_SHORT_CHANNEL_SCAN;
    change_channel( );
    g_config_state = AIRKISS_STATUS_CONTINUE;

    /* channel scanning */
    while ( g_thread_monitor_running & ( g_config_state != AIRKISS_STATUS_COMPLETE ) ) {
        int time_used = time_calc_usec( &start );

        if( g_config_state == AIRKISS_STATUS_CONTINUE ) {
            if ( time_used > scan_time ) {
                /* switch to next channel */
                scan_time = T_SHORT_CHANNEL_SCAN;
                //FIXME: close the socket before channel switch,
                //otherwise it'll bug on some platform
                //close(sd);

                gettimeofday( &start, NULL );
                change_channel( );

                /* FIXME: reopen the socket, to drop the kernel buffer */
                //sd = open_socket();
            }
        }

        state = wifi_recv_raw_package( listen_sock );
        if( state == 0 ) {
            // 短扫描能收到数据包说明这通道在使用，加大扫描时间
            scan_time = T_ONE_CHANNEL_SCAN;
            //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            //++ 查看google radiotap
            ret = ieee80211_radiotap_iterator_init( &iter, ( void * )frame_buf, frame_len, NULL );
            if( ret ) {
                perror( "init radiotap failed" );
                continue;
            }


            radio_tap_len = iter._max_length;
            LOG_DEBUG( "radio tap len = %d ", radio_tap_len );

            if( ( frame_len - radio_tap_len ) < 24 )
                continue;

            ProcessPacket( ( unsigned char * )( frame_buf + radio_tap_len ), ( frame_len - radio_tap_len ) );

        }
    }

    wifi_monitor_exit();
    g_thread_monitor_running = 0;
    close( listen_sock );

    pthread_exit( 0 );
}



void start_airkiss( char *key )
{
    int8_t ret;

    //key = key;

    LOG_MSG( "Start airkiss!\r\n" );

    ret = airkiss_init( &akcontex, &akconf );

    if ( ret < 0 ) {
        perror("Airkiss init failed!" );
        return;
    }
#if AIRKISS_ENABLE_CRYPT
    airkiss_set_key( &akcontex, ( const unsigned char* )key, strlen( key ) );
#endif
    printf( "Finish init airkiss!\r\n" );
    printf( "===============\n%s\n===============\n", airkiss_version() );
}


int get_breoadcast_addr( void *breodcastaddr )
{
    int ret = -1;
    int sock = -1;
    struct sockaddr_in *sin;

    struct ifreq ifr;



    //建立数据报套接字
    sock = socket( AF_INET, SOCK_DGRAM, 0 );
    if ( sock < 0 ) {
        LOG_DEBUG( "create socket failed: %s", strerror( errno ) );
        return -1;
    }
    memset( &ifr, 0, sizeof( ifr ) );

    strncpy( ifr.ifr_name, ifName, sizeof( ifr.ifr_name ) );

    if ( ioctl( sock, SIOCGIFBRDADDR, &ifr ) == -1 ) {
        LOG_DEBUG( "ioctl error: %s", strerror( errno ) );
        goto out;
    }
    sin = ( struct sockaddr_in * )&ifr.ifr_addr;
    strcpy( breodcastaddr, inet_ntoa( sin->sin_addr ) );
    LOG_MSG( "Broadcast address is %s\n", ( char * )breodcastaddr );
out:
    close( sock );

    ret = 0;
    return ret;
}

int broadcast_notification( char *msg, int msg_num )
{
    int ret , i, sock;

    sock = socket( AF_INET, SOCK_DGRAM, 0 );
    if( sock < 0 ) {
        perror("socket failed");
        return -1;
    }
    int so_broadcast = 1;
    ret = setsockopt( sock, SOL_SOCKET, SO_BROADCAST, &so_broadcast, sizeof( so_broadcast ) );
    if( ret < 0 ) {
        perror("setsockopt failed");
        goto out;
    }

    struct sockaddr_in local, dest;
    char broadcast[16] = {0};
    get_breoadcast_addr( ( void * )broadcast );


    memset( &local, 0, sizeof( local ) );
    memset( &dest, 0, sizeof( dest ) );
    local.sin_family = AF_INET;
    local.sin_port = htons( UDP_RX_PORT );
    local.sin_addr.s_addr = htonl( INADDR_ANY );
    dest.sin_family = AF_INET;
    dest.sin_port = htons( UDP_RX_PORT );
    dest.sin_addr.s_addr = inet_addr( broadcast );

    ret = bind( sock, ( void * )&local, sizeof( local ) );
    if( ret < 0 ) {
        perror("Bind failed");
        goto out;
    }

    for( i = 0; i < msg_num; i++ ) {
        ret = sendto( sock, msg, strlen( msg ), 0, ( void * )&dest, sizeof( dest ) );
        if( ret < 0 ) {
            perror("sendto failed");
            goto out;
        }
        LOG_MSG( "broadcast: %s data len = %d\n", msg, strlen( msg ) );
    }

out:
    close( sock );

    ret = 0;
    return ret;

}

int xrairkiss_server_thread_handle(char *buf, int length)
{
    int ret;

    ret = strcmp(buf,"OK");
    if(0 == ret)
    {
	f_send_finished = 1;
	printf("thread_handle of xrsc: recieve OK from server!\n");
	return 0;
//	return THREAD_EXIT;
    }
	return -1;
//    return THREAD_CONTINUE;
}
int xradio_airkiss_protocol_resource_free(void *arg)
{
    g_thread_monitor_running = 0;
    pthread_join( thread_monitor, NULL );

	if(frame_buf)
		free(frame_buf);
}
int xradio_airkiss_protocol(void *arg)
{
    int ret, seconds;
    char cmdstr[256];
	char wifi_connect_info[100];
	bool is_receive = false;

	struct pro_feedback info;

	struct pro_worker *_worker = (struct pro_worker *)arg;
	struct net_info netInfo;

	_worker->enable = true;
	info.force_quit_sm = atoi((_worker->params->argv)[0]);
	info.protocol = _worker->type;

	g_config_timeout = atoi((_worker->params->argv)[1]);
	strcpy( g_aes_key,(_worker->params->argv)[2]);
	strcpy(ifName,(_worker->params->argv)[3]);

	printf("force quit:%d\n",info.force_quit_sm);

	printf("if name %s\n",ifName);
	printf("aes  %s\n",g_aes_key);
	printf(" time out %d\n",g_config_timeout);

	_worker->free_cb = xradio_airkiss_protocol_resource_free;

    frame_buf = malloc( IP_MAXPACKET );
    if( frame_buf == NULL ) {
        perror( "malloc err ok!" );
		goto end;
    }

#if AIRKISS_ENABLE_CRYPT
    start_airkiss(g_aes_key);
#else
	start_airkiss(NULL);
#endif

    //init_chplan(chnum);
    airkiss_wifi_disable( ifName );
    airkiss_wifi_enable( ifName );
    wifi_monitor_exit( );
    wifi_do_scan();
    wifi_monitor_setup( );

    g_thread_monitor_running = 1;


    ret = pthread_create( &thread_monitor, NULL, airkiss_sniffer_thread_func, NULL );
    if ( ret < 0 ) {
        perror( "pthread_create failed!" );
		goto end;
    }

    seconds = g_config_timeout;
    while (_worker->enable && --seconds && g_thread_monitor_running ) {
        sleep(1);
	}

	printf("=====================\n");
	printf("SSID:     %s\n",(char *)airkiss_result.ssid);
	printf("PASSWORD: %s\n",(char *)airkiss_result.pwd);
	printf("=====================\n");
    LOG_MSG( "monitor mode: switch to station mode\n" );

	if(seconds && airkiss_result.ssid != NULL && airkiss_result.pwd != NULL) {
		strcpy(netInfo.ssid,(char *)airkiss_result.ssid);
		strcpy(netInfo.password, (char *)airkiss_result.pwd);
		is_receive = true;
	}else
		goto main_exit;

    // TODO:
    // 向端口10000发送至少20个UDP广播包，内容为 airkiss_result.random 的值，一个字节
    memset( cmdstr, 0x00, 256 );
    cmdstr[0] = airkiss_result.random;

    broadcast_notification( cmdstr, 20 );

    LOG_MSG( "---exit airkiss config----\n" );


main_exit:
end:
	_worker->enable = false;
	_worker->cb(&info,is_receive,&netInfo);

    return 0;
}
