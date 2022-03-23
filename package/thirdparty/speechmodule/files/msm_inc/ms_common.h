/**
 * @file ms_common.h
 * @brief
 * @author 閸欒埖顨熷Ч锟�
 * @version 1.0.0
 * @par Copyright  (c)
 *		Midea
 */
#ifndef MS_COMMON_H

#define MS_COMMON_H

/* Includes ,We must include C standard headers*/
//#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "errno.h"
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netpacket/packet.h>
#include <netinet/if_ether.h>
#include <signal.h>

#include "ms_version.h"

#include "msm_adapter.h"

//#define MS_TEST
/* //delete by 926 @20190618
#define SIG_UART_EVENT_WDG                           __SIGRTMIN+10
#define SIG_SYS_EVENT_DEV_READY                      __SIGRTMIN+11
#define SIG_SYS_EVENT_WIFI_READY                     __SIGRTMIN+12
#define SIG_SYS_EVENT_SYSTEM_STATUS_CHANGE           __SIGRTMIN+13
#define SIG_SYS_EVENT_STOP_MSPEECH			         __SIGRTMIN+14
#define SIG_SYS_EVENT_START_MSPEECH			         __SIGRTMIN+15
*/
#define SIG_UART_EVENT_WDG                           SIGRTMIN+10
#define SIG_SYS_EVENT_DEV_READY                      SIGRTMIN+11
#define SIG_SYS_EVENT_WIFI_READY                     SIGRTMIN+12
#define SIG_SYS_EVENT_SYSTEM_STATUS_CHANGE           SIGRTMIN+13
#define SIG_SYS_EVENT_STOP_MSPEECH			         SIGRTMIN+14
#define SIG_SYS_EVENT_START_MSPEECH			         SIGRTMIN+15

/*Config file*/
#define MS_STORED_INFO          "/mnt/app/cfg/msmart_store_info.txt"
#define MS_DEVICE_ID		"/mnt/app/cfg/device_id"
#define MS_DNSMASQ_CONFIG_PATH  "/mnt/app/cfg/dnsmasq.conf"
#define MS_HOSTAPD_CONFIG_PATH  "/mnt/app/cfg/hostapd.conf"
#define MS_SYS_STATUS_PATH      "/mnt/app/cfg/sys_status.conf"
#define MS_STA_POINT            "wlan0"
#define MS_SOFTAP_POINT         "wlan1"
/*FIFO file*/
#define MSMART_NET_OUT          "/tmp/msmart_net_out"
#define	MSMART_LOCAL_NET_OUT    "/tmp/msmart_local_net_out"
#define MAISPEECH_OUT           "/tmp/maipseech_out"
#define MSMART_NET_IN           "/tmp/msmart_net_in"
#define MSMART_LOCAL_NET_IN     "/tmp/msmart_local_net_in"
#define MAISPEECH_IN            "/tmp/maipseech_in"

//#define MSM_DEBUG_LICENSE
//#define MSMART_MACBIN_FILE "/dev/by-name/private/license"
#define MSMART_MACBIN_FILE      "/mnt/app/license"
#define MSMART_OP_LICENSE_FILE  "/mnt/app/cfg/op_license"
#define MSMART_WAKE_SOCKET_FILE "/mnt/app/cfg/wake_socket"

#if defined(MS_DOMAIN_OVERSEA)
#define	MSM_SOFTWARE_VERSION	"1.0.1_oversea"
#else
#define	MSM_SOFTWARE_VERSION	"1.0.1"
#endif

#ifndef uint8_t
typedef unsigned char uint8_t;
#endif
#ifndef uint8_t
typedef unsigned short int uint16_t;
#endif
#define MSM_SOFTWARE_VERSION	"3.1.1"
#ifndef uint32_t
typedef unsigned  int uint32_t;
#endif

#define MS_UDP_WAKEUP_PORT      7890
#define MS_UDP_MONITOR_PORT		6445	///< 濡�虫健閻╂垵鎯夐獮鎸庢尡缁旑垰褰涢崣锟�
#define MS_UDP_BROADCAST_PORT	7083	///<	娑擄拷閺堣桨绔撮惍浣哥畭閹绢厾顏崣锝呭娇
#define MS_UDP_NEW_PROFILE_BROADCAST_PORT 15000	///< 娑擄拷閸ㄥ绔撮惍浣哥畭閹绢厾顏崣锝呭娇
#define MS_TCP_SERVER_PORT		6444	///< 濡�虫健TCP鏉╃偞甯寸粩顖氬經
#define TCP_CLIENT_TIMEOUT_MS 5000		///< 鐏烇拷閸╃喓缍塗CP socket閹恒儲鏁归崣鎴︼拷浣规闂勶拷

#define MS_UDP_BROADCAST_SOCKET_INDEX	0	///< udp楠炴寧鎸眘ocket缁便垹绱�
#define MS_UDP_MONITOR_SOCKET_INDEX		1	///< udp閻╂垵鎯塻ocket缁便垹绱�
#define MS_TCP_SERVER_SOCKET_INDEX		2	///< 濡�虫健TCP server缁便垹绱�
#define MS_TCP_CLIENT_SOCKET_START_INDEX		3	///< 婢舵牠鍎碩CP鏉╃偞甯磗ocket缁便垹绱�
#define MAX_SOCKET_TCP_CLIENT		3	///< 婢舵牠鍎碩CP鏉╃偞甯撮張锟芥径褎鏆�
#define MS_TOTAL_SOCKET_NUM		(MS_TCP_CLIENT_SOCKET_START_INDEX + MAX_SOCKET_TCP_CLIENT)	///< 閹鍙￠惃鍓唎cket閺侊拷

#define MS_SSID_MAX_LENGTH	32	///< 闂勬劕鐣炬径鏍劥鐠侯垳鏁遍幋鏍侀崸姊恜濡�崇础ssid閻ㄥ嫭娓舵径褔鏆辨惔锔芥Ц32鐎涙顑�
#define MS_PWD_MAX_LENGTH	64	///< 鐠佹儳鐣炬径鏍劥鐠侯垳鏁遍幋鏍侀崸姊恜濡�崇础閻ㄥ埦wd閺堬拷闂�澶歌礋64鐎涙顑�
#define MS_SN_MAX_LENGTH		32	///< sn鐠佹儳鐣炬稉锟�32娑擃亜鐡х粭锕傛毐鎼达拷
typedef enum MS_RESULT_T{
        MS_RESULT_SUCCESS = 0X00,
        MS_RESULT_FAIL,
}ms_result_t;

/**
 * \defgroup ms_common_data msmart闁氨鏁ら柈銊ュ瀻
 * \brief msmart sdk闁氨鏁ら幙宥勭稊,閺佺増宓佺紒鎾寸��,鐎瑰繒鐡戠�规矮绠�
 * \par
 * 鐎广垺鍩涚粙瀣碍閸涙ê婀担璺ㄦ暏閻ㄥ嫭妞傞崐娆忔晼闁插繋濞囬悽銊︽拱SDK閻ㄥ嫰锟芥氨鏁ょ�规矮绠�,婵″倷缍呴幙宥勭稊缁涳拷. \n
 * 閸忚渹缍嬬拠宄板棘閼帮拷: ms_common.h \n
 * 閸欙箑顦�,閺堢悞DK娑旂喎鐣炬稊澶夌啊闁劌鍨庨柅姘辨暏閻ㄥ嫭鏆熼幑顔剧波閺嬶拷,婵″倿鎽肩悰锟�,闂冪喎鍨粵锟�,娴犮儲鏌熸笟鍨吂閹撮鈻兼惔蹇撴喅閹垮秳缍�.瑜版挾鍔�,鐠囧嘲顓归幋鐤箷閺勵垯绱崗鍫滃▏閻€劌閽╅崣鏉跨暰娑斿娈戞潻娆庣昂缂佹挻鐎�. \n
 * 閸忚渹缍嬬拠宄板棘閼帮拷:ms_queue.h
 */

/* Macro Functions */
/**
 * \brief BIT_ENABLE(found, mask) \n
 * 鐎瑰繐鍤遍弫甯礉娴ｈ儻鍏橀弻鎰嚋濮ｆ梻澹掓担宥冿拷锟�
 * \param found  閸樼喎顫愰弫鐗堝祦.
 * \param mask  閹衡晝鐖�.
 * \return 閼差垰鐣鹃幍褑顢戦幋鎰.
 */
#define BIT_ENABLE(found, mask)			((found)|=(mask))
/**
 * \brief BIT_DISABLE(found, mask) \n
 * 鐎瑰繐鍤遍弫甯礉缁備胶鏁ら弻鎰嚋濮ｆ梻澹掓担宥冿拷锟�
 * \param found  閸樼喎顫愰弫鐗堝祦.
 * \param mask  閹衡晝鐖�.
 * \return 閼差垰鐣鹃幍褑顢戦幋鎰.
 */
#define BIT_DISABLE(found, mask)		((found)&=~(mask))

/**
 * \brief BIT_ISSET(found, mask) \n
 * 鐎瑰繐鍤遍弫甯礉閸掋倖鏌囬弻鎰嚋濮ｆ梻澹掓担宥嗘Ц閸氾缚濞囬懗濮愶拷锟�
 * \param found  閸樼喎顫愰弫鐗堝祦.
 * \param mask  閹衡晝鐖�.
 * \return true閳ユ柡锟芥柧濞囬懗锟�.\n
 *        false閳ユ柡锟芥梻顩﹂悽锟�.
 */
#define BIT_ISSET(found, mask)			(((found)&(mask))!=0)

/**
 * \brief CLR_STRUCT(p) \n
 * 鐎瑰繐鍤遍弫甯礉濞撳懐鈹栫紒鎾寸�担鎾憋拷锟�
 * \param p  缂佹挻鐎担鎾村瘹闁斤拷.
 * \return 閼差垰鐣鹃幍褑顢戦幋鎰.
 */
#define CLR_STRUCT(p)					memset(p, 0x0, sizeof(*p))

/**
 * \brief NUM_MAX(a,b) \n
 * 鐎瑰繐鍤遍弫甯礉濮ｆ棁绶漚閸滃異閻ㄥ嫬銇囩亸蹇ョ礉鏉╂柨娲栭崗鏈佃厬婢堆呮畱闁絼閲滈妴锟�
 * \param a  闂囷拷鐟曚焦鐦潏鍐畱閺佺増宓�.
 * \param b  闂囷拷鐟曚焦鐦潏鍐畱閺佺増宓�.
 * \return a閸滃異娑擃叀绶濇径褍锟斤拷.
 */
#define NUM_MAX(a,b)						((a>b)?(a):(b))
/**
 * \brief NUM_MIN(a,b) \n
 * 鐎瑰繐鍤遍弫甯礉濮ｆ棁绶漚閸滃異閻ㄥ嫬銇囩亸蹇ョ礉鏉╂柨娲栭崗鏈佃厬鐏忓繒娈戦柇锝勯嚋閵嗭拷
 * \param a  闂囷拷鐟曚焦鐦潏鍐畱閺佺増宓�.
 * \param b  闂囷拷鐟曚焦鐦潏鍐畱閺佺増宓�.
 * \return a閸滃異娑擃叀绶濈亸蹇擄拷锟�.
 */
#define NUM_MIN(a,b)						((a<b)?(a):(b))

/* Debug Functions */
#define MS_DEBUG 1;
#if defined(MS_DEBUG)
#define MS_TRACE(format, arg...)			msm_printf("[%s:%d]:\t"format"\n", __FILE__, __LINE__, ##arg)
#define MS_ERR_TRACE(format, arg...)		msm_printf("[%s:%d]ERR:\t"format"\n", __FILE__, __LINE__, ##arg)
#define MS_DBG_TRACE(format, arg...)			msm_printf("[%s:%d]DBG:\t"format"\n", __FILE__, __LINE__, ##arg)
#define TRACE(format, arg...)				msm_printf("[%s:%d]:\t"format"\n", __FILE__, __LINE__, ##arg)
#define MS_BLE_TRACE(format, arg...)			msm_printf("[MS_BLE]"format"\n", ##arg)
#define MS_BLE_ERR_TRACE(format, arg...)		msm_printf("[MS_BLE_ERR]"format"\n", ##arg)
#define MS_BLE_DBG_TRACE(format, arg...)		msm_printf("[MS_BLE_DBG]"format"\n", ##arg)
#else
/**
 * \brief MS_TRACE(format, arg...) \n
 * 鐎瑰繐鍤遍弫甯礉閺嶇厧绱￠崠鏍ㄥⅵ閸楁媽绶崙鎭掞拷锟�
 * \param format  閹垫挸宓冮惃鍕壐瀵拷.
 * \param arg  闂囷拷鐟曚焦鐗稿蹇撳閹垫挸宓冮惃鍕殶閹癸拷.
 * \return 閼差垰鐣鹃幍褑顢戦幋鎰.
 * \note 娴兼俺鍤滈崝銊﹀潑閸旂姴澧犵紓锟�(濠ф劖鏋冩禒鏈电秴缂冿拷)閸滃苯鎮楃紓锟�(閹广垼顢�).
 * \par 缁�杞扮伐娴狅絿鐖�:\n
 * @code
 * MS_TRACE("hello world!");
 * @endcode
 * 鐎圭偤妾潏鎾冲毉:\n
 * @code
 * [test_ms.c:12]:[MS]hello world!
 *
 * @endcode
 */
#define MS_TRACE(format, arg...)			msm_printf("[MS]"format"\n", ##arg)
/**
 * @brief MS_ERR_TRACE(format, arg...) \n
 * 鐎瑰繐鍤遍弫甯礉閺嶇厧绱￠崠鏍ㄥⅵ閸楀府绱濋悽銊ょ艾闁挎瑨顕ゆ穱鈩冧紖閵嗭拷
 * @param format  閹垫挸宓冮惃鍕壐瀵拷.
 * @param arg  闂囷拷鐟曚焦鐗稿蹇撳閹垫挸宓冮惃鍕殶閹癸拷.
 * @return 閼差垰鐣鹃幍褑顢戦幋鎰.
 * @note 閹垫挸宓冪敮锕�澧犵紓锟絒MS_ERR]
 */
#define MS_ERR_TRACE(format, arg...)		msm_printf("[MS_ERR]"format"\n", ##arg)
/**
 * @brief MS_DBG_TRACE(format, arg...) \n
 * 鐎瑰繐鍤遍弫甯礉閺嶇厧绱￠崠鏍ㄥⅵ閸楁媽绶崙鐚寸礉閻€劋绨幍鎾冲祪鐠嬪啳鐦穱鈩冧紖
 * @param format  閹垫挸宓冮惃鍕壐瀵拷.
 * @param arg  闂囷拷鐟曚焦鐗稿蹇撳閹垫挸宓冮惃鍕殶閹癸拷.
 * @return 閼差垰鐣鹃幍褑顢戦幋鎰.
 * @note 閼奉亜濮╁ǎ璇插閸撳秶绱慬MS_DBG]
 */
#define MS_DBG_TRACE(format, arg...)			msm_printf("[MS_DBG]"format"\n", ##arg)
/**
 * \brief TRACE(format, arg...) \n
 * 鐎瑰繐鍤遍弫甯礉閺嶇厧绱￠崠鏍ㄥⅵ閸楁媽绶崙鎭掞拷锟�
 * @param format  閹垫挸宓冮惃鍕壐瀵拷.
 * @param arg  闂囷拷鐟曚焦鐗稿蹇撳閹垫挸宓冮惃鍕殶閹癸拷.
 * @return 閼差垰鐣鹃幍褑顢戦幋鎰.
 * @note 娴兼俺鍤滈崝銊﹀潑閸旂姴澧犵紓锟�(濠ф劖鏋冩禒鏈电秴缂冿拷)閸滃苯鎮楃紓锟�(閹广垼顢�).
 * @par 缁�杞扮伐娴狅絿鐖�:\n
 * @code
 * TRACE("hello world!");
 * @endcode
 * 鐎圭偤妾潏鎾冲毉:\n
 * @code
 * [test_ms.c:12]:[MS]hello world!
 *
 * @endcode
 */
#define TRACE(format, arg...)				msm_printf("[MS]"format"\n", ##arg)
#define MS_BLE_TRACE(format, arg...)
#define MS_BLE_ERR_TRACE(format, arg...)
#define MS_BLE_DBG_TRACE(format, arg...)
#endif


/**
 * @brief PRINT_FORMAT(format, arg...) \n
 * 鐎瑰繐鍤遍弫甯礉閺嶇厧绱￠崠鏍ㄥⅵ閸楁澘鐡х粭锔胯閹存牗鏆熺紒鍕繆閹拷
 * @param[in]	buf  閹垫挸宓冮惃鍕繆閹垯缍�
 * @param[in]	len  閹垫挸宓冮惃鍕Х閹垯缍嬮惃鍕毐鎼达拷
 * @param[in]	format	閹垫挸宓冮惃鍕畱閺嶇厧绱�
 *			%c 鐎涙顑佽ぐ銏犵础鏉堟挸鍤�
 *			%d 閸椾浇绻橀崚鎯扮翻閸戯拷
 *			%x閸椾礁鍙氭潻娑樺煑鏉堟挸鍤�
 * \return 閼差垰鐣鹃幍褑顢戦幋鎰.
 */
#define PRINT_FORMAT(buf, len, format)	do{int i;if((int)len<0)break;printf("len:%d\n", len);msm_printf("[");for(i=0;i<len;i++){if(0==(i%16)&&0!=i)msm_printf("\n");char *p=(char*)buf;msm_printf(format, (p[i]&0xff));}msm_printf("]\n");}while(0)
#define PRINT_CHAR(buf, len)				PRINT_FORMAT(buf, len, "%c")
/**
 * \brief PRINT_BUF(buf, len) \n
 * 鐎瑰繐鍤遍弫甯礉娴犮儱宕勯崗顓＄箻閸掕埖瀵滅�涙濡幍鎾冲祪鏉堟挸鍤紓鎾冲暱閸栧搫鍞寸�瑰箍锟斤拷
 * \param buf  闂囷拷鐟曚焦澧﹂崡鎵畱缂傛挸鍟块崠鍝勬勾閸э拷.
 * \param len  闂囷拷鐟曚焦澧﹂崡鎵畱闂�鍨閿涘苯宕熸担宥呯摟閼猴拷.
 * \return 閼差垰鐣鹃幍褑顢戦幋鎰.
 */
#define PRINT_BUF(buf, len)				PRINT_FORMAT(buf, len, "%02x ")
/**
 * \brief PRINT_CHAR(buf, len) \n
 * 鐎瑰繐鍤遍弫甯礉娴狀櫓har閺嶇厧绱￠幐澶婄摟閼哄倹澧﹂崡鎷岀翻閸戣櫣绱﹂崘鎻掑隘閸愬懎顔愰妴锟�
 * \param buf  闂囷拷鐟曚焦澧﹂崡鎵畱缂傛挸鍟块崠鍝勬勾閸э拷.
 * \param len  闂囷拷鐟曚焦澧﹂崡鎵畱闂�鍨閿涘苯宕熸担宥呯摟閼猴拷.
 * \return 閼差垰鐣鹃幍褑顢戦幋鎰.
 */
#define PRINT_CHAR(buf, len)			PRINT_FORMAT(buf, len, "%c")
/**
 * \brief PRINT_STR(buf, len) \n
 * 鐎瑰繐鍤遍弫甯礉娴狀櫓har閺嶇厧绱￠幐澶婄摟閼哄倹澧﹂崡鎷岀翻閸戣櫣绱﹂崘鎻掑隘閸愬懎顔�.
 * \param buf  闂囷拷鐟曚焦澧﹂崡鎵畱缂傛挸鍟块崠鍝勬勾閸э拷.
 * \param len  闂囷拷鐟曚焦澧﹂崡鎵畱闂�鍨閿涘苯宕熸担宥呯摟閼猴拷.
 * \note 娴兼俺鍤滈崝銊﹀潑閸旂姴澧犵紓锟�(濠ф劖鏋冩禒鏈电秴缂冿拷)閸滃苯鎮楃紓锟�(閹广垼顢�).
 * \return 閼差垰鐣鹃幍褑顢戦幋鎰.
 */
#define PRINT_STR(buf, len)	do{msm_printf("[%s:%d]:\t[", __FILE__, __LINE__);PRINT_FORMAT(buf, len, "%c");msm_printf("]\n");}while(0)
/**
 * \brief PRINT_STR_EX(buf, len) \n
 * 鐎瑰繐鍤遍弫甯礉閸掑棗鍩嗘禒顧﹉ar閺嶇厧绱￠崪灞藉磩閸忣叀绻橀崚璺鸿埌瀵繑瀵滅�涙濡幍鎾冲祪鏉堟挸鍤紓鎾冲暱閸栧搫鍞寸�癸拷.
 * \param buf  闂囷拷鐟曚焦澧﹂崡鎵畱缂傛挸鍟块崠鍝勬勾閸э拷.
 * \param len  闂囷拷鐟曚焦澧﹂崡鎵畱闂�鍨閿涘苯宕熸担宥呯摟閼猴拷.
 * \note 娴兼俺鍤滈崝銊﹀潑閸旂姴澧犵紓锟�(濠ф劖鏋冩禒鏈电秴缂冿拷)閸滃苯鎮楃紓锟�(閹广垼顢�).
 * \return 閼差垰鐣鹃幍褑顢戦幋鎰.
 */
#define PRINT_STR_EX(buf, len)	do{\
											msm_printf("[%s:%d]:\t[\n", __FILE__, __LINE__);\
											PRINT_FORMAT(buf, len, "%c\t");\
											msm_printf("]\n");\
											PRINT_FORMAT(buf, len, "%02x\t");\
											msm_printf("]\n");\
										}while(0)
/**
 * \brief ST_HANDLE \n
 * 缂佺喍绔寸�规矮绠焗andle缁鐎锋稉绡燭_HANDLE閵嗭拷
 * \note 濞夈劍鍓伴棃鐐寸《缁鐎锋稉绡扤VALID_STHANDLE.
 */
typedef void *ST_HANDLE;

 /**
  * \brief INVALID_STHANDLE \n
  * 缂佺喍绔寸�规矮绠熼棃鐐寸《閻ㄥ埅andle缁鐎锋稉绡扤VALID_STHANDLE閵嗭拷
  */
#define INVALID_STHANDLE					(NULL)

/**
 * \brief ST_FD \n
 * 缂佺喍绔寸�规矮绠焒d缁鐎锋稉绡燭_FD閵嗭拷
 * \note 濞夈劍鍓伴棃鐐寸《缁鐎锋稉绡扤VALID_FD.
 */
typedef int ST_FD;

/**
 * \brief INVALID_FD \n
 * 缂佺喍绔寸�规矮绠熼棃鐐寸《閻ㄥ垿d缁鐎锋稉绡扤VALID_FD閵嗭拷
 */
#define INVALID_FD						(-1)

/**
 * \brief enum E_Err \n
 * 缂佺喍绔寸�规矮绠熼柨娆掝嚖娴狅絿鐖滈妴锟�
 */
typedef enum E_Err
{
	E_ERR_SUCCESS		= 0,		/*!< 閹笛嗩攽閹存劕濮�. */
	E_ERR_UNKNOWN		= -1,		/*!< 閺堫亞鐓￠柨娆掝嚖. */
	E_ERR_INVALID_PARAM = -2,		/*!< 閸欏倹鏆熼柨娆掝嚖. */
	E_ERR_NO_MEM		= -3,		/*!< 濞屸剝婀侀崣顖滄暏閸愬懎鐡�. */
	E_ERR_BUSY			= -4,		/*!< 缁崵绮虹换浣哥箹. */
	E_ERR_NOAUTH		= -5,		/*!< 鐠併倛鐦夋径杈Е. */
	E_ERR_FD			= -6,		/*!< IO闁挎瑨顕�. */
	E_ERR_EOF			= -7,		/*!< 鐠囪褰囬懛鐭監F. */
	E_ERR_TIMEOUT		= -8,		/*!< 閹笛嗩攽鐡掑懏妞�. */
	E_ERR_NOT_SUPPORT	= -9,		/*!< 娑撳秵鏁幐锟�. */
	E_ERR_EMPTY_PTR     = -10,       /*!< 缁岀儤瀵氶柦锟�. */
	E_ERR_EXIST		  = -11,
	E_ERR_SST			  = -12,
}E_Err;
/**
 * \brief enum e_network_status_t \n
 * 缂冩垹绮堕悩鑸碉拷浣碉拷锟�
 */
typedef enum E_NETWORK_STATUS
{
	E_NETWORK_STATUS_IDLE = 0,	///< 缁屾椽妫介悩鑸碉拷浣碉拷锟�
	E_NETWORK_STATUS_AP,	///< ap閻樿埖锟斤拷
	E_NETWORK_STATUS_STA,	///< sta閻樿埖锟斤拷
}e_network_status_t;
/**
 * \brief enum E_socket_type_t \n
 * socket缁鐎烽妴锟�
 */
typedef enum{
	ENUM_MS_SOCKET_TYPE_UDP = 0,	/*!< UDP缁鐎烽惃鍓唎cket */
	ENUM_MS_SOCKET_TYPE_TCP,	/*!< TCP缁鐎烽惃鍓唎cket */
}E_socket_type_t;

#define WEP_ENABLED        0x0001
#define TKIP_ENABLED       0x0002
#define AES_ENABLED        0x0004
#define WSEC_SWFLAG        0x0008

#define SHARED_ENABLED  0x00008000
#define WPA_SECURITY    0x00200000
#define WPA2_SECURITY   0x00400000
#define WPS_ENABLED     0x10000000

/**
 * \brief enum ms_security_t \n
 * 鐠侯垳鏁遍崝鐘茬槕缁鐎烽妴锟�
 */
typedef enum {
    MS_SECURITY_OPEN           = 0,                                                /**< Open security                           */
    MS_SECURITY_WEP_PSK        = WEP_ENABLED,                                      /**< WEP Security with open authentication   */
    MS_SECURITY_WEP_SHARED     = ( WEP_ENABLED | SHARED_ENABLED ),                 /**< WEP Security with shared authentication */
    MS_SECURITY_WPA_TKIP_PSK   = ( WPA_SECURITY  | TKIP_ENABLED ),                 /**< WPA Security with TKIP                  */
    MS_SECURITY_WPA_AES_PSK    = ( WPA_SECURITY  | AES_ENABLED ),                  /**< WPA Security with AES                   */
    MS_SECURITY_WPA2_AES_PSK   = ( WPA2_SECURITY | AES_ENABLED ),                  /**< WPA2 Security with AES                  */
    MS_SECURITY_WPA2_TKIP_PSK  = ( WPA2_SECURITY | TKIP_ENABLED ),                 /**< WPA2 Security with TKIP                 */
    MS_SECURITY_WPA2_MIXED_PSK = ( WPA2_SECURITY | AES_ENABLED | TKIP_ENABLED ),   /**< WPA2 Security with AES & TKIP           */
    MS_SECURITY_WPA_WPA2_MIXED = ( WPA_SECURITY  | WPA2_SECURITY ),                /**< WPA/WPA2 Security                       */

    MS_SECURITY_WPS_OPEN       = WPS_ENABLED,                                      /**< WPS with open security                  */
    MS_SECURITY_WPS_SECURE     = (WPS_ENABLED | AES_ENABLED),                      /**< WPS with AES security                   */

    MS_SECURITY_UNKNOWN        = -1,                                               /**< May be returned by scan function if security is unknown. Do not pass this to the join function! */

    MS_SECURITY_FORCE_32_BIT   = 0x7fffffff                                        /**< Exists only to force rtw_security_t type to 32 bits */
} ms_security_t;

#define SERVER_OFFICIAL   1		///< 鐎规矮绠熷锝呯础閺堝秴濮熼崳锟�
#define SERVER_UNOFFICIAL     2	///鐎规矮绠熷ù瀣槸閺堝秴濮熼崳锟�


typedef enum MS_BOOT_MODE_T{
	MS_BOOT_MODE_STA = 0x00,
	MS_BOOT_MODE_IDLE,
	MS_BOOT_MODE_CONFIG_MS_AP,
	MS_BOOT_MODE_CONFIG_MS_BT,
	MS_BOOT_MODE_CONFIG_MS_SNIFFER,
	MS_BOOT_MODE_CONFIG_AIRKISS,
	MS_BOOT_MODE_CONFIG_JD_SNIFFER,
	MS_BOOT_MODE_CONFIG_ALI_AP,
	MS_BOOT_MODE_CONFIG_ALI_SNIFFER,
	MS_BOOT_MODE_CONFIG_MIX_SNIFFER,
	MS_BOOT_MODE_UNKNOWN,
}ms_boot_mode_t;
/**
 * \brief	struct ms_stored_info_t
 * 鐎规矮绠熼柊宥囩秹鏉╁洨鈻奸悽銊ф畱闂囷拷鐟曚礁鐡ㄩ崒銊ф畱缂佹挻鐎担锟�
 */
typedef struct{
	uint8_t random_num[16];														/*!< 闁板秶缍夋潻鍥┾柤閻ㄥ嫰娈㈤張鐑樻殶. */
	uint8_t device_id[6];														/*!< 閻㈠灚甯剁拋鎯ь槵ID */
	uint8_t boot_mode;															/*!< 鐠佹澘缍嶅Ο鈥虫健娑撳顐奸崥顖氬З鏉╂稑鍙嗛惃鍕佸锟� */
	uint8_t e1_default_mode;													/*!< 閼存碍婀伴崶鐐差槻E1閻ㄥ嫭膩閸ф绮拋銈喣佸锟� */
	uint8_t ap_sniffer_keep_alive_tm;//E1										/*!< 姒涙顓诲Ο鈥崇础娣囨繃瀵旈惃鍕闂傦拷 */
	uint8_t protocol_support;//E1												/*!< 濡�虫健閺�顖涘瘮閻ㄥ嫮顑囨稉澶嬫煙閸楀繗顔� */
	uint8_t config_method;//config												/*!< 鐠佹澘缍嶅Ο鈥虫健閻ㄥ嫰鍘ょ純鎴炴煙瀵拷 */
	uint8_t sn[MS_SN_MAX_LENGTH + 1/*'\0'*/];									/*!< 閻㈠灚甯堕惃鍓唍 */
	uint8_t device_type[4];//the fist four bytes of 0xa0 response				/*!< 閻㈠灚甯剁拋鎯ь槵閻ㄥ嫬顔嶉悽鐢佃閸拷 */
	uint8_t ssid[MS_SSID_MAX_LENGTH + 1/*'\0'*/];								/*!< 闁板秶缍夌捄顖滄暠閻ㄥ墕sid */
	uint8_t ssid_len;															/*!< ssid閻ㄥ嫰鏆辨惔锟� */
	uint8_t pwd[MS_PWD_MAX_LENGTH + 1/*'\0'*/];									/*!< 闁板秶缍夌捄顖滄暠閻ㄥ埦wd */
	uint8_t pwd_len;															/*!< pwd閻ㄥ嫰鏆辨惔锟� */
	uint8_t ak_notification_flag;												/*!< 瀵邦喕淇婇獮鎸庢尡閻ㄥ嫭鐖ｈ箛妞剧秴 */
	uint8_t ak_random;															/*!< 瀵邦喕淇婇柊宥囩秹閻ㄥ嫰娈㈤張鐑樻殶 */
	uint32_t ak_security_type;													/*!<  */
	uint8_t server_flag;														/*!<  */
	char wechat_public_number_1[20];											/*!< 瀵邦喕淇婇崗顑跨船閸欙拷 */
	char wechat_public_number_2[20];											/*!< 瀵邦喕淇婇崗顑跨船閸欙拷 */
	uint8_t recv_0x64_flag;
        /*Mac addr*/
    uint8_t mac_addr[6];/*!<  */
}ms_stored_info_t;

typedef struct{
	int	    timeout_ms;
	uint8_t sn[MS_SN_MAX_LENGTH + 1/*'\0'*/];
	uint8_t random_num[16];
	uint8_t device_type[4];
	uint8_t mac_addr[6];
	uint8_t server_flag;
	uint8_t product_key[32];
	uint8_t product_secret[32];
	uint8_t camera_id[16];
}ms_cloud_init_info_t;


typedef struct _T_Flag
{
    unsigned char ucMsgIdentify;    //data[6]
    int iCmdFlag;                   //data[9]
}T_Flag;

typedef struct _T_EventFlag
{
    T_Flag stNet;           //MSMART_NET_OUT
    T_Flag stLocalNet;      //MSMART_LOCAL_NET_OUT
    T_Flag stSpeech;      //MAISPEECH_OUT
}T_EventFlag;

typedef enum MS_UART_DOWN_EVENT_T{
    MS_SYS_EVENT_INVALID    = 0,
    //MS_SYS_EVENT_SNIFFER_OK = 0x20,//save & switch to sta
    //MS_SYS_EVENT_SNIFFER_FAILED,
    //MS_SYS_EVENT_SAVE_INFO_AND_REBOOT,
    //MS_SYS_EVENT_LOGIN_MS_CLOUD,
    //MS_SYS_EVENT_LOGOUT_MS_CLOUD,
    MS_SYS_EVENT_SYS_STATUS_CHANGED,//include login/logout, tcp clients change
    MS_SYS_EVENT_TRANSDATA,
    //MS_SYS_EVENT_REBOOT,
    //MS_SYS_EVENT_RESET,
    //MS_SYS_EVENT_ONLY_SAVE_INFO,
    //MS_SYS_EVENT_REFRESH_R3,
    //MS_UART_EVENT_START_SUCCESS,
    //MS_UART_EVENT_START_FAILED,
    MS_UART_EVENT_REPORT_NOACK,
    MS_UART_EVENT_REPORT_ACK,
    MS_UART_EVENT_SYN_TIME,                 //net to uart down msg
    MS_UART_EVENT_AP_MODE,                  //mspeech to uart down msg
    //MS_UART_EVENT_DEVICE_UNBINDING,
    MS_UART_EVENT_WAKE_LOCK,
	MS_UART_EVENT_WAKE_UNLOCK,
	MS_UART_EVENT_WAKE_ENABLE,
	MS_UART_EVENT_WAKE_DISABLE,
    MS_EVENT_TO_MSPEECH_MIN     = 0x400,
    MS_LOCALNET_EVENT_NETWORK_SETTING, //The event are used for voice broadcast during network connection
    MS_EVENT_TO_MSPEECH_MAX,
}E_MsDownStream;

/**

 * \brief 鐢勭壐瀵繐鐣炬稊锟� \n

 * 鐎规矮绠熸稉锟界粩鍨摍娴肩娀锟芥帗鏆熼幑顔炬畱鐢勭壐瀵繈锟斤拷

 */
#define MAX_UART_LEN 255
typedef struct T_MsTransStream{

	E_MsDownStream event;						//!< 鐢呰閸ㄥ绱滰see E_MsNetAppEvent.

	uint8_t data[MAX_UART_LEN];							//!< 鐢勬殶閹诡噯绱濈敮褎鏆熼幑顔炬畱閺嶇厧绱￠弽瑙勫祦鐢呰閸ㄥ绻樼悰宀冃掗弸锟�.

	size_t size;								//!< 鐢勬殶閹诡噣鏆辨惔锔肩礉缂傛挸鍟块崠娲櫡閺堝鏅ラ弫鐗堝祦閻ㄥ嫰鏆辨惔锟�.

	uint32_t msg_id;							//!< 濞戝牊浼匢D

}T_MsTransStream;

typedef struct MS_SYS_STATUS_T
{
        uint8_t module_type;
        uint8_t wifi_mode;
        uint8_t wifi_signal_level;
        uint8_t ip_v4[4];
        uint8_t rf_signal_level;
        uint8_t lan_status;
        uint8_t cloud_status;
        uint8_t tcp_status;
        uint8_t tcp_connected_number;
        uint8_t multi_cloud_status;
        uint8_t ucRsv1;
        uint8_t config_method;
        uint8_t ucRsv[5];
}ms_sys_status_t;

typedef enum
{
	ERROR	= -1,
	OK		= 0,
	FAILD	= 1,
}status_t;

#endif /* ifndef MS_COMMON_H */
