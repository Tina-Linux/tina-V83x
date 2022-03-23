/*
 * ms_net_cloud.h
 *
 */

#ifndef _MS_NET_APP_H_
#define _MS_NET_APP_H_

#include <ms_common.h>

/**
 * \defgroup  external_module 澶栫綉鎺ュ彛
 * 澶栫綉澶勭悊妯″潡鍖呮嫭澶栫綉鍗忚瑙ｆ瀽鍜屽懡浠ゅ鐞�.鎺ユ敹鍒版暟鎹細璋冨叆MS_NetApp_Init浼犲叆鐨刦n_ms_event_cb鍥炶皟鍑芥暟锛岄渶瑕佸彂閫佹暟鎹椂鍙互璋冪敤MS_NetApp_Send鏉ュ彂閫�.
 * \see
 * 閲嶈鐨勬暟鎹粨鏋�: \n
 * E_MsNetAppEvent \n
 * T_MsNetAppFrame \n
 * 閲嶈鍑芥暟: \n
 * extern E_Err MS_NetApp_Init(ST_HANDLE *handle,fn_ms_event_cb cb,char *addr, int port,int timeout_ms,const char *sn,int heartbeat_ms); \n
 * extern E_Err MS_NetApp_Exit(ST_HANDLE *handle); \n
 * extern E_Err MS_NetApp_Process(ST_HANDLE *handle, uint32_t tm_ms); \n
 * extern E_Err MS_NetApp_Send(ST_HANDLE *handle, T_MsNetAppFrame *p_frame, void *args); \n
 * \note
 * E_MsNetAppEvent涓粡甯镐娇鐢ㄧ殑浜嬩欢鏄�
 * ENUM_MS_NETAPP_EVENT_TRANS,ENUM_MS_NETAPP_EVENT_REPORT_ACK,ENUM_MS_NETAPP_EVENT_REPORT_NOACK \n
 * ENUM_MS_NETAPP_EVENT_TRANS鏄敱APP缁忎簯绔�忎紶缁欐ā鍧楃殑浜嬩欢锛屾ā鍧楅�氳繃T_MsNetAppFrame鎶婃秷鎭嬁鍑烘潵缁欑數鎺у鐞哱n
 * ENUM_MS_NETAPP_EVENT_REPORT_ACK鏄敱妯″潡缁忎簯绔�忎紶缁橝PP鐨勪簨浠讹紝妯″潡鎷垮埌鐢垫帶鏁版嵁鍚庯紝灏嗙數鎺ф暟鎹拰ENUM_MS_NETAPP_EVENT_REPORT_ACK濉叆T_MsNetAppFrame锛孿n
 *                                 鐒跺悗璋冪敤MS_NetApp_Send鍙戦�佺粰浜戠锛屾浜嬩欢闇�瑕佷簯绔簲绛斻�俓n
 * ENUM_MS_NETAPP_EVENT_REPORT_NOACK鍜孍NUM_MS_NETAPP_EVENT_REPORT_ACK涓�鏍凤紝鍖哄埆灏辨槸涓嶉渶瑕佷簯绔簲绛斻��
 *\note
 * 鍦ㄤ娇鐢ㄨ繃绋嬩腑,鏈変簺浜嬩欢鍥炶皟鍑芥暟鐢ㄧ殑鏃堕棿姣旇緝涔�,姣斿閫忎紶鐨勮鍐欎覆鍙ｇ瓑IO澶勭悊鍑芥暟,璇烽噸鏂板紑涓�涓繘绋嬶紙绾跨▼锛夋潵澶勭悊姝ょ浜嬩欢 \n
 * 鍚屾椂,涓轰簡閬垮厤鏈嶅姟鍣ㄧ鍚屾椂鏀跺埌涓や釜IP,port鐨勫寘鑰屾墦鏋�,鏂扮殑杩涚▼锛堢嚎绋嬶級璇蜂笉瑕佽皟鐢ㄤ笂杩扮殑鍑犱釜鍑芥暟.\n
 * 璇峰鎴风▼搴忓憳閫氳繃娑堟伅闃熷垪鏉ヨ繘琛岄�氳,鎿嶄綔鏃朵竴瀹氳娉ㄦ剰瀵筿ueue鍔犻攣淇濇姢.\n
 * 褰撶劧,瀹㈡埛绋嬪簭鍛樼▼搴忓憳涔熷彲浠ュ埄鐢ㄧ郴缁熻嚜甯︾殑queue鏉ヨ繘琛岀嚎绋�(杩涚▼)闂撮�氫俊, 濡俧reertos鐢ㄦ埛鍙噰鐢▁QUEUE,linux鐢ㄦ埛鍙噰鐢╨inux涓嬫秷鎭槦鍒�(Message queue).
 * \par 鍙傝�冧唬鐮�:
  ms_net_test.c
 */
/**
 * \brief 娴嬭瘯鐢ㄧ殑鏈嶅姟鍣ㄧ鍙ｅ彿 \n
 * 瀹氫箟娴嬭瘯浣跨敤鐨勬湇鍔″櫒鍦板潃锛岄粯璁や娇鐢�28870.
 */
#if defined(MS_SST)
#define MS_NET_PORT		(28443)
#else
#define MS_NET_PORT		(28870)
#endif
/**
 * \brief 闃诲瓒呮椂 \n
 * 瀹氫箟娴嬭瘯鏃剁綉缁滄搷浣滅殑闃诲鏃堕棿锛岄粯璁�9绉掕秴鏃�.
 */
#define NET_OP_TIMEOUT_MS   (1000)
/**
 * \brief 瀹氫箟SN鐨勬渶澶ч暱搴� \n
 * 瀹氫箟妯″潡涓茬爜(SN)鐨勬渶澶ч暱搴︿负64锛屽寘鎷粨鏉熺'\0'.
 */
#define MS_SN_MAX	(64)

/**
 * \brief 甯х被鍨嬪畾涔� \n
 * 瀹氫箟浜嬩欢鍥炶皟鐨勫抚绫诲瀷锛屾瀹氫箟涔熺敤浜嶮S_NetApp_Send锛岃〃绀鸿鍙戦�佺殑甯х被鍨嬨��
 */
typedef enum{
	ENUM_MS_NETAPP_EVENT_LOGIN,					//!< [鍗忚浜嬩欢]鐧婚檰鎴愬姛锛屼粎浣滀负浜嬩欢閫氱煡锛岃閫氱煡鏃犳暟鎹�.
	ENUM_MS_NETAPP_EVENT_LOGOUT,				//!< 璁惧绂荤嚎锛屼粎浣滀负浜嬩欢閫氱煡锛岃閫氱煡鏃犳暟鎹�.

	ENUM_MS_NETAPP_EVENT_TRANS,					//!< [server<->client			]鏀跺埌閫忎紶鏁版嵁锛屾敹鍒扮殑鏁版嵁璇峰弬鑰僒_MsNetAppFrame锛岄渶閲嶇偣鍏虫敞鍏舵垚鍛榤sg_id锛屽洖澶嶉渶涓庤姹備繚鎸佷竴鑷�

	ENUM_MS_NETAPP_EVENT_REBOOT,				//!< [server ->client ->server	]閲嶅惎鎸囦护甯э紝璇ユ暟鎹抚鏃犳暟鎹紝璇疯皟鐢� MS_NetApp_Send 鍙戦�佹墽琛岀粨鏋滃悗閲嶅惎绯荤粺.
	ENUM_MS_NETAPP_EVENT_RESTORE,				//!< [server ->client ->server	]閲嶇疆鎸囦护甯э紝璇ユ暟鎹抚鏃犳暟鎹紝璇疯皟鐢� MS_NetApp_Send 鍙戦�佹墽琛岀粨鏋滃悗鎭㈠鍑哄巶閰嶇疆锛岀劧鍚庨噸鍚�.

	ENUM_MS_NETAPP_EVENT_GET_TIME,				//!< [client ->server ->client	]鏃堕棿鍚屾甯э紝鍙戦�佺殑鏁版嵁鏄┖锛屾敹鍒扮殑鏁版嵁鏍煎紡鎸夌収 T_MsNetAppGetTimeAck杩涜瑙ｆ瀽.
	ENUM_MS_NETAPP_EVENT_REPORT_NOACK,			//!< [client ->server			]涓婃姤浜嬩欢甯�(鏃犲洖搴�)锛屼粎琚玀S_NetApp_Send浣跨敤锛屽彂閫佺粰鏈嶅姟鍣ㄧ殑鏄�忎紶鏁版嵁.
	ENUM_MS_NETAPP_EVENT_REPORT_ACK,			//!< [client ->server ->client	]涓婃姤浜嬩欢甯�(鏈夊洖搴�)锛屾ā鍧楀悜鏈嶅姟鍣ㄤ笂鎶ヤ簨浠舵秷鎭紝灏嗕細鏀跺埌鏈嶅姟鍣ㄥ彂鏉ョ殑閫忎紶鏁版嵁.

	ENUM_MS_NETAPP_EVENT_REFRESH_VID,			//!< [client ->server ->client	]浜戠鍒嗛厤VID锛屾洿鏂板埌妯″潡绔紝闇�淇濆瓨鍒癋lash锛屽悗缁眬鍩熺綉鍜屽箍鍩熺綉鍒濆鍖栧潎闇�浣跨敤褰撳墠鏈�鏂扮殑device_id
	ENUM_MS_NETAPP_EVENT_REQUEST_NEW_VERSION,	//!< [client ->server ->client	][Unused]
	ENUM_MS_NETAPP_EVENT_GET_WECHAT_PUBLIC_NUM,	//!< [client ->server ->client	]鍙傛暟涓婃姤鑾峰彇寰俊鍏紬鍙凤紝鏀跺埌鐨勬暟鎹鍙傝�僒_MsNetAppFrame.
	ENUM_MS_NETAPP_EVENT_GET_SST_R3,			//!< [client ->server ->client	]鍚戞湇鍔″櫒鐢宠R3锛屾敹鍒扮殑鏁版嵁璇峰弬鑰僒_MsNetAppFrame.濡傛灉鏀寔灞�鍩熺綉锛屽垯闇�澶勭悊姝や簨浠�
}E_MsNetAppEvent;

/**
 * \brief 甯ф牸寮忓畾涔� \n
 * 瀹氫箟鍗忚鏍堝拰璋冪敤鑰呬紶閫掓暟鎹殑甯ф牸寮忋��
 */
typedef struct ST_MsNetAppFrame{
	E_MsNetAppEvent event;						//!< 甯х被鍨嬶紝@see E_MsNetAppEvent.
	uint8_t data[1500];							//!< 甯ф暟鎹紝甯ф暟鎹殑鏍煎紡鏍规嵁甯х被鍨嬭繘琛岃В鏋�.
	size_t size;								//!< 甯ф暟鎹暱搴︼紝缂撳啿鍖洪噷鏈夋晥鏁版嵁鐨勯暱搴�.
	uint32_t msg_id;							//!< 缃戠粶娑堟伅ID
}T_MsNetAppFrame;

/**
 * \brief 閲嶅惎甯х殑鏁版嵁鏍煎紡瀹氫箟 \n
 * 瀹氫箟閲嶅惎甯х殑鏁版嵁鏍煎紡:
 * 鏁板�紎瀹氫箟
 * --|--
 * true|鎴愬姛
 * false|澶辫触
 */
typedef bool T_MsNetAppReboot;

/**
 * \brief 閲嶇疆甯х殑鏁版嵁鏍煎紡瀹氫箟 \n
 * 瀹氫箟閲嶇疆甯х殑鏁版嵁鏍煎紡:
 * 鏁板�紎瀹氫箟
 * --|--
 * true|鎴愬姛
 * false|澶辫触
 */
typedef bool T_MsNetAppRestore;

/**
 * \brief 涓婃姤浜嬩欢甯�(鏈夊洖搴�)鐨勬暟鎹牸寮忓畾涔� \n
 * 涓婃姤浜嬩欢甯�(鏈夊洖搴�)鐨勬暟鎹牸寮�:
 * 鏁板�紎瀹氫箟
 * --|--
 * true|鎴愬姛
 * false|澶辫触
 */
typedef bool T_MsNetAppReportACK;

/**
 * \brief 鏃堕棿鍚屾甯х殑鏁版嵁鏍煎紡瀹氫箟 \n
 * 瀹氫箟鏃堕棿鍚屾甯х殑鏁版嵁鏍煎紡.
 */
typedef struct {
	uint8_t tm_sec;				//!< 绉�.
	uint8_t tm_min;				//!< 鍒�.
	uint8_t tm_hour;			//!< 鏃�.
	uint8_t tm_week;			//!< 涓�鍛ㄤ腑鐨勭鍑犲ぉ锛�0-6渚濇琛ㄧず鏄熸湡鏃ュ埌鏄熸湡鍏�.
	uint8_t tm_day;			//!< 鏃ユ湡.
	uint8_t tm_mon;				//!< 鏈堜唤.
	uint8_t tm_year;			//!< 骞翠唤(00~99).
	uint8_t tm_zone;			//!< ZONE.
}T_MsNetAppGetTimeAck;

/**
 * \brief int (*fn_ms_event_cb)(T_MsNetAppFrame *p_frame, void *args) \n
 * 鍥炶皟鍑芥暟瀹氫箟锛屾鍥炶皟鍑芥暟鐢盡S_NetApp_Init浼犲叆锛屽湪鎺ユ敹鍒版暟鎹抚鏃朵細璋冪敤.
 * \param p_frame  鏀跺埌鐨勬暟鎹抚.
 * \param args 鐢ㄦ埛浼犻�掔殑鍙傛暟锛孧S_NetApp_Init鐢辫皟鐢ㄨ�呬紶鍏�.
 * \return 鍒濆鍖栭敊璇姸鎬�.璇ラ敊璇姸鎬佸繀椤昏杩斿洖,鑰屼笖蹇呴』瑕佸鐞�.
 * \note handle鐨勫瓨鍌ㄧ┖闂寸敱SDK绔垎閰�,涓嶇敤鐨勬椂鍊欑敱SDK绔噴鏀�.
 * \warning 鍥炶皟鍑芥暟涓笉瑕佹墽琛屽彲鑳介樆濉炵殑鎿嶄綔锛屽惁鍒欏彲鑳藉奖鍝嶅崗璁爤鐨勭ǔ瀹氭��.
 */
typedef int (*fn_ms_event_cb)(T_MsNetAppFrame *p_frame, void *args);

/**
 * \brief 鍒濆鍖杕smart澶栫綉瀵硅薄.
 * \param handle  澶栫綉瀵硅薄鐨勬寚閽�.
 * \param cb 鍥炶皟鍑芥暟锛屽綋鏀跺埌鏈嶅姟鍣ㄥ彂鏉ョ殑鏁版嵁甯ф椂浼氳嚜鍔ㄨ皟鐢�.
 * \param ops_handle osal鐨刪andle.
 * \param args 鐢ㄦ埛浼犻�掔殑鍙傛暟锛屽崗璁爤鍐呬笉浣跨敤锛屼粎浠呭湪璋冪敤鍥炶皟鐨勬椂鍊欎紶閫掔粰璋冪敤鑰咃紝寤鸿浼犲叆鍗忚鏍堢殑鎵�鏈夎�呯殑handle.
 * \param sn 璁惧鐨凷N鍙�,浣跨敤璇ユ帴鍙ｇ殑鏃跺��,閫氳繃涓插彛浠庣數鎺х璇诲彇.
 * \param heartbeat_ms 蹇冭烦鍖呯殑鍙戦�侀棿闅旓紝鍗曚綅姣.
 * \param timeout_ms,缃戠粶鎿嶄綔瓒呮椂鏃堕棿
 * \return 鍒濆鍖栭敊璇姸鎬�.璇ラ敊璇姸鎬佸繀椤昏杩斿洖,鑰屼笖蹇呴』瑕佸鐞�.
 * \note handle鐨勫瓨鍌ㄧ┖闂寸敱SDK绔垎閰�,涓嶇敤鐨勬椂鍊欑敱SDK绔噴鏀�.
 */
extern E_Err MS_NetApp_Init(ST_HANDLE *handle,fn_ms_event_cb cb, ST_HANDLE ops_handle, void *args, ms_cloud_init_info_t *p_cloud_init_info);
/**
 *\param handle 澶栫綉瀵硅薄鎸囬拡.
 *\note  閲婃斁handle鎸囬拡鐨勬椂鍊欏繀椤婚噴鏀緃andle鎸囬拡.
 */
extern E_Err MS_NetApp_Exit(ST_HANDLE *handle);

/**
 * \brief 鍗忚鏍堢殑鐘舵�佹満.\n
 * 濡傛灉鏀跺埌鏁版嵁甯т細璋冪敤MS_NetApp_Init浼犲叆鐨勪簨浠跺洖璋冿紝娉ㄦ剰浜嬩欢鍥炶皟涓笉鍏佽鏈夐樆濉炴搷浣溿�備笉鍚屽抚绫诲瀷鐨勬暟鎹涓�:
 * 甯х被鍨媩鎺ユ敹鐨勬暟鎹牸寮弢鏄惁闇�瑕佸洖搴攟杩斿洖鐨勬暟鎹牸寮弢璇存槑
 * --|--|--|--|--
 * ENUM_MS_NETAPP_EVENT_LOGIN|鏃爘鍚鏃爘鏃�
 * ENUM_MS_NETAPP_EVENT_TRANS|鏃爘鍚鏃爘娴佸紡鏃犳牸寮忔暟鎹紝
 * ENUM_MS_NETAPP_EVENT_REBOOT|鏃爘鏄瘄T_MsNetAppReboot|鏃�
 * ENUM_MS_NETAPP_EVENT_RESTORE|鏃爘鏄瘄T_MsNetAppRestore|鏃�
 * ENUM_MS_NETAPP_EVENT_GET_TIME|T_MsNetAppGetTimeAck|鍚鏃爘鏃�
 * ENUM_MS_NETAPP_EVENT_REPORT_ACK|鏃爘鍚鏃爘娴佸紡鏃犳牸寮忔暟鎹�
 * \param handle  澶栫綉瀵硅薄鐨勬寚閽�.
 * \param tm_ms 褰撳墠鏃堕棿锛屽崟浣嶆绉掞紝寤鸿浣跨敤msm_timer_get_systime_ms杩斿洖鐨勭郴缁熸椂闂�.
 * \return 閿欒浠ｇ爜.璇ラ敊璇姸鎬佸繀椤昏杩斿洖,鑰屼笖蹇呴』瑕佸鐞�.
 * \note 1.蹇呴』鐢变竴涓嚎绋嬩笉鏂皟鐢�.\n
 *       2.姝ゆ帴鍙ｆ湁鍙兘闃诲.\n
 *       3.濡傛灉缃戠粶鍙戠敓寮傚父锛屾鎺ュ彛浼氳繑鍥為敊璇紝涓嶄細鑷姩閲嶈繛锛屽繀椤昏皟鐢∕S_NetApp_Exit涔嬪悗鍐嶈皟鐢∕S_NetApp_Init.\n
 */
extern E_Err MS_NetApp_Process(ST_HANDLE *handle, uint32_t tm_ms);

/**
 * \brief 鍙戦�丮-Smart鏍煎紡鐨勬暟鎹抚.\n
 * 涓嶅悓甯х被鍨嬬殑鏁版嵁濡備笅:
 * 甯х被鍨媩鍙戦�佺殑鏁版嵁鏍煎紡|鏄惁闇�瑕佸洖搴攟鍥炲簲鐨勬暟鎹牸寮弢璇存槑
 * --|--|--|--|--
 * ENUM_MS_NETAPP_EVENT_TRANS|鏃爘鍚鏃爘娴佸紡鏃犳牸寮忔暟鎹紝
 * ENUM_MS_NETAPP_EVENT_REBOOT|T_MsNetAppReboot|鍚鏃爘鐢辨湇鍔″櫒鍙戣捣
 * ENUM_MS_NETAPP_EVENT_RESTORE|T_MsNetAppRestore|鍚鏃爘鐢辨湇鍔″櫒鍙戣捣
 * ENUM_MS_NETAPP_EVENT_GET_TIME|鏃爘鍚鏃爘鐢卞鎴风鍙戣捣
 * ENUM_MS_NETAPP_EVENT_REPORT_NOACK|鏃爘鍚鏃爘鐢卞鎴风鍙戣捣锛屽彂閫佺殑鏄祦寮忔棤鏍煎紡鏁版嵁
 * ENUM_MS_NETAPP_EVENT_REPORT_ACK|鏃爘鏄瘄鏃爘鐢卞鎴风鍙戣捣锛屾敹鍙戠殑閮芥槸娴佸紡鏃犳牸寮忔暟鎹�
 * \param handle  澶栫綉瀵硅薄鐨勬寚閽�.
 * \param p_frame 瑕佸彂閫佺殑鏁版嵁甯�.
 * \param args 淇濈暀鍙傛暟锛岀幇鍦ㄦ湭浣跨敤锛屽缓璁紶鍏ULL銆�
 * \return 鍒濆鍖栭敊璇姸鎬�.璇ラ敊璇姸鎬佸繀椤昏杩斿洖,鑰屼笖蹇呴』瑕佸鐞�
 * \note 1.姝ゆ帴鍙ｆ湁鍙兘闃诲.\n
 *       2.濡傛灉缃戠粶鍙戠敓寮傚父锛屾鎺ュ彛浼氳繑鍥為敊璇紝涓嶄細鑷姩閲嶈繛锛屽繀椤昏皟鐢∕S_NetApp_Exit涔嬪悗鍐嶈皟鐢∕S_NetApp_Init.\n
 */
extern E_Err MS_NetApp_Send(ST_HANDLE *handle, T_MsNetAppFrame *p_frame, void *args);

#endif /* _MS_NET_APP_H_ */
