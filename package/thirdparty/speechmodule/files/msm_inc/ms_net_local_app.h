#ifndef __MS_NET_LOCAL_APP_H__
#define __MS_NET_LOCAL_APP_H__

#include <ms_common.h>

/**
 * \brief enum E_MsNetLocalAppEventMask \n
 *
 */

/**
 * \defgroup  internal_module 锟斤拷锟斤拷模锟斤拷
 * 锟斤拷锟斤拷锟斤拷锟斤拷模锟斤拷锟斤拷锟斤拷锟斤拷锟叫拷锟斤拷锟斤拷锟斤拷锟斤拷锟筋处锟斤拷.锟斤拷锟秸碉拷锟斤拷锟捷伙拷锟斤拷锟酵拷锟絤s_net_local_app_init锟斤拷锟斤拷锟絤s_net_local_event_cb锟截碉拷锟斤拷锟斤拷锟斤拷锟斤拷要锟斤拷锟斤拷锟斤拷锟斤拷时锟斤拷锟皆碉拷锟斤拷ms_net_local_app_send锟斤拷锟斤拷锟斤拷.
 * \see
 * 锟斤拷要锟斤拷锟斤拷锟捷结构: \n
 * E_MsNetLocalAppEvent \n
 * ms_dev_info_t \n
 * T_MsNetLocalAppFrame \n
 * 锟斤拷要锟斤拷锟斤拷: \n
 * extern E_Err ms_net_local_app_init(ST_HANDLE *handle, ms_net_local_event_cb event_cb,ST_HANDLE ops_handle, ms_dev_info_t *dev_info);\n
 * extern E_Err ms_net_local_app_process(ST_HANDLE *handle, uint32_t tm_ms);\n
 * extern E_Err ms_net_local_app_send(ST_HANDLE *handle, T_MsNetLocalAppFrame *p_frame, void *args);\n
 * extern E_Err ms_net_local_app_exit(ST_HANDLE *handle);\n
 * extern E_Err ms_sys_api_get_ap_ssid(uint8_t *ap_ssid,const ms_dev_info_t *p_dev_info);\n
 * \note
 *
 * E_MsNetAppEvent锟叫撅拷锟斤拷使锟矫碉拷锟铰硷拷锟斤拷
 * ENUM_MS_NETLOCALAPP_EVENT_TRANS,ENUM_MS_NETLOCALAPP_EVENT_REPORT_ACK,ENUM_MS_NETLOCALAPP_EVENT_REPORT_NOACK \n
 * ENUM_MS_NETLOCALAPP_EVENT_TRANS锟斤拷锟斤拷APP透锟斤拷锟斤拷模锟斤拷锟斤拷录锟斤拷锟侥ｏ拷锟酵拷锟絋_MsNetAppFrame锟斤拷锟斤拷息锟矫筹拷锟斤拷锟斤拷锟斤拷卮锟斤拷锟絓n
 * ENUM_MS_NETLOCALAPP_EVENT_REPORT_ACK锟斤拷锟斤拷模锟斤拷透锟斤拷锟斤拷APP锟斤拷锟铰硷拷锟斤拷模锟斤拷锟矫碉拷锟斤拷锟斤拷锟斤拷莺螅锟斤拷锟斤拷锟斤拷锟捷猴拷ENUM_MS_NETAPP_EVENT_REPORT_ACK锟斤拷锟斤拷T_MsNetAppFrame锟斤拷\n
 *                                     然锟斤拷锟斤拷锟絤s_net_local_app_send直锟接凤拷锟酵革拷APP锟斤拷锟斤拷锟铰硷拷锟斤拷要APP应锟斤拷\n
 * ENUM_MS_NETLOCALAPP_EVENT_REPORT_NOACK锟斤拷ENUM_MS_NETAPP_EVENT_REPORT_ACK一锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷遣锟斤拷锟揭狝PP应锟斤拷\n
 *
 * 锟斤拷使锟矫癸拷锟斤拷锟斤拷,锟斤拷些锟铰硷拷锟截碉拷锟斤拷锟斤拷锟矫碉拷时锟斤拷冉暇锟�,锟斤拷锟斤拷透锟斤拷锟侥讹拷写锟斤拷锟节碉拷IO锟斤拷锟斤拷锟斤拷,锟斤拷锟斤拷锟铰匡拷一锟斤拷锟斤拷锟教ｏ拷锟竭程ｏ拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷录锟� \n
 * 同时,为锟剿憋拷锟斤拷锟斤拷锟斤拷锟斤拷锟酵憋拷盏锟斤拷锟斤拷锟絀P,port锟侥帮拷锟斤拷锟斤拷锟�,锟铰的斤拷锟教ｏ拷锟竭程ｏ拷锟诫不要锟斤拷锟斤拷锟斤拷锟斤拷锟侥硷拷锟斤拷锟斤拷锟斤拷.\n
 * 锟斤拷突锟斤拷锟斤拷锟皆蓖拷锟斤拷锟较拷锟斤拷锟斤拷锟斤拷锟斤拷锟酵ㄑ�,锟斤拷锟斤拷时一锟斤拷要注锟斤拷锟絨ueue锟斤拷锟斤拷锟斤拷锟斤拷.\n
 * 锟斤拷然,锟酵伙拷锟斤拷锟斤拷员锟斤拷锟斤拷员也锟斤拷锟斤拷锟斤拷锟斤拷系统锟皆达拷锟斤拷queue锟斤拷锟斤拷锟斤拷锟竭筹拷(锟斤拷锟斤拷)锟斤拷通锟斤拷, 锟斤拷freertos锟矫伙拷锟缴诧拷锟斤拷xQUEUE,linux锟矫伙拷锟缴诧拷锟斤拷linux锟斤拷锟斤拷息锟斤拷锟斤拷(Message queue).
 *
 *
 * \par 锟轿匡拷锟斤拷锟斤拷:
 ms_net_local_test.c
 *
 */

/**
 * \brief enum E_MsNetLocalAppEvent \n
 *
 */

typedef enum{
	ENUM_MS_NETLOCALAPP_EVENT_CLIENT_NUM_CHANGE	= 0, //!< [APP->Module][SoftAP/STA],锟斤拷锟斤拷锟斤拷锟斤拷模锟斤拷锟斤拷server锟斤拷app锟斤拷client.锟斤拷锟铰硷拷锟斤拷示client锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟戒化锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷为ms_client_num_change_t锟结构锟斤拷锟絘live_num锟斤拷员
	ENUM_MS_NETLOCALAPP_EVENT_TRANS,                 //!< [APP<->Module][SoftAP/STA],透锟斤拷锟铰硷拷锟斤拷APP通锟斤拷锟斤拷event锟斤拷锟斤拷锟斤拷透锟斤拷锟斤拷模锟介，模锟斤拷通锟斤拷锟斤拷event锟斤拷锟斤拷锟斤拷透锟斤拷锟斤拷app锟斤拷锟斤拷锟截碉拷锟阶_MsNetLocalAppFrame锟结构锟斤拷锟� msg_id锟斤拷fd锟斤拷员锟斤拷锟截革拷锟斤拷锟斤拷锟斤拷锟襟保筹拷一锟斤拷
	ENUM_MS_NETLOCALAPP_EVENT_CONFIG,                //!< [APP->Module][SoftAP],锟斤拷锟斤拷锟缴癸拷锟铰硷拷锟斤拷锟酵伙拷锟斤拷锟斤拷员锟秸碉拷锟斤拷锟铰硷拷锟斤拷锟斤拷要锟斤拷锟斤拷SSID锟斤拷锟斤拷锟诫、random_num锟斤拷device_id锟斤拷锟揭匡拷锟矫达拷SSID锟斤拷锟斤拷锟斤拷去锟斤拷路锟斤拷锟斤拷
	ENUM_MS_NETLOCALAPP_EVENT_QUERY_VERSION,         //!< [Module->app][SoftAP/STA],锟芥本锟脚诧拷询
	ENUM_MS_NETLOCALAPP_EVENT_REPORT_NOACK,            //!< [Module->app][SoftAP/STA], 模锟介将锟斤拷锟斤拷锟斤拷息透锟斤拷锟斤拷APP,APP锟斤拷锟斤拷应锟斤拷
	ENUM_MS_NETLOCALAPP_EVENT_REPORT_ACK,            //!< [Module->APP->Module][SoftAP/STA],同ENUM_MS_NETLOCALAPP_MSG_REPORT_NOACK锟斤拷锟斤拷APP锟斤拷要应锟斤拷
	ENUM_MS_NETLOCALAPP_EVENT_EVOKE_HTTP_OTA,        //!< [APP->Module][Unused], 锟斤拷锟斤拷指锟斤拷
	ENUM_MS_NETLOCALAPP_EVENT_REBOOT,                //!< [APP->Module][SoftAP/STA],锟斤拷锟斤拷指锟斤拷
	ENUM_MS_NETLOCALAPP_EVENT_RESET,                 //!< [APP->Module][SoftAP/STA],锟斤拷位指锟斤拷
	ENUM_MS_NETLOCALAPP_EVENT_SCAN_AP_LIST,									//!< [APP<->Module][SoftAP]模锟斤拷扫锟斤拷AP锟叫憋拷锟斤拷锟斤拷锟斤拷锟斤拷锟解场锟斤拷.
}E_MsNetLocalAppEvent;


/**
 * \brief struct ms_client_num_change_t \n
 * 锟斤拷锟斤拷锟斤拷TCP锟斤拷锟斤拷锟斤拷
 */
typedef struct{
	uint8_t alive_num;														//!< 锟斤拷录锟斤拷锟斤拷锟斤拷TCP锟斤拷锟斤拷锟斤拷.
}ms_client_num_change_t;


/**
 * \brief struct ms_dev_info_t \n
 * 锟斤拷锟斤拷锟斤拷锟芥储锟斤拷息锟侥结构锟斤拷
 *
 **/
typedef struct{
	uint8_t		ip[4];                // !< 锟斤拷锟侥Ｊ� ex. 192.168.1.2 -> ip[3] = 192
	char	sn[33];                   // 锟斤拷< the last byte is '0'
	uint8_t		config_type;          // !<  used in msdk
	uint8_t	device_type[4];           // !<  锟借备锟斤拷锟斤拷
	uint8_t	mac_addr[6];              // !<  MAC锟斤拷址锟斤拷锟斤拷要锟矫伙拷锟斤拷
	uint8_t		device_id[6];         // !< device_id,锟斤拷app锟斤拷锟斤拷
	char		random[17];              // !<the last byte is '0'
	char	ssid[MS_SSID_MAX_LENGTH+1];  //!< the last byte is '0'
	char		pwd[MS_PWD_MAX_LENGTH+1];//!< the last byte is '0'
	uint8_t		mcloud_login_status;     // !< 0 : logout , 1: login
	e_network_status_t network_status;   // !< 模锟斤拷墓锟斤拷锟侥Ｊ斤拷锟紼_NETWORK_STATUS_AP : SoftAP模式 , E_NETWORK_STATUS_STA : STA模式
	char	*p_ms_model_config;          //!< 一锟斤拷一锟斤拷锟斤拷锟斤拷锟斤拷sta锟斤拷锟斤拷锟斤拷息锟矫ｏ拷锟矫伙拷锟斤拷锟斤拷员锟斤拷锟斤拷锟斤拷锟接此筹拷员

}ms_dev_info_t;

/**
 * \brief 帧锟斤拷式锟斤拷锟斤拷 \n
 * 锟斤拷锟斤拷协锟斤拷栈锟酵碉拷锟斤拷锟竭达拷锟斤拷锟斤拷锟捷碉拷帧锟斤拷式锟斤拷
 */
typedef struct{
	E_MsNetLocalAppEvent event;											//!< 帧锟斤拷锟酵ｏ拷@see E_MsNetLocalAppEvent
	uint8_t data[MAX_UART_LEN];//the max num is the config length (1500)			//!< 帧锟斤拷锟捷ｏ拷帧锟斤拷锟捷的革拷式锟斤拷锟斤拷帧锟斤拷锟酵斤拷锟叫斤拷锟斤拷.
	size_t size;														//!< 帧锟斤拷锟捷筹拷锟饺ｏ拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷效锟斤拷锟捷的筹拷锟斤拷.
	uint32_t msg_id;													//!< 锟斤拷锟斤拷锟斤拷息ID
	int16_t	fd;															//!< 锟斤拷锟斤拷帧锟斤拷TCP client锟斤拷socket锟斤拷锟斤拷锟斤拷
}T_MsNetLocalAppFrame;

/**
 * \brief E_Err (*ms_net_local_event_cb)(T_MsNetLocalAppFrame *data) \n
 * 锟截碉拷锟斤拷锟斤拷锟斤拷锟藉，锟剿回碉拷锟斤拷锟斤拷锟斤拷ms_net_local_app_init锟斤拷锟诫，锟节斤拷锟秸碉拷锟斤拷锟斤拷帧时锟斤拷锟斤拷锟�.
 * \param data  锟秸碉拷锟斤拷锟斤拷锟斤拷帧.
 * \return 锟斤拷始锟斤拷锟斤拷锟斤拷状态.锟矫达拷锟斤拷状态锟斤拷锟斤拷要锟斤拷锟斤拷,锟斤拷锟揭憋拷锟斤拷要锟斤拷锟斤拷.
 * \note handle锟侥存储锟秸硷拷锟斤拷SDK锟剿凤拷锟斤拷,锟斤拷锟矫碉拷时锟斤拷锟斤拷SDK锟斤拷锟酵凤拷.
 * \warning 锟截碉拷锟斤拷锟斤拷锟叫诧拷要执锟叫匡拷锟斤拷锟斤拷锟斤拷锟侥诧拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟接帮拷锟叫拷锟秸伙拷锟斤拷榷锟斤拷锟�.
 */
typedef E_Err (*ms_net_local_event_cb)(T_MsNetLocalAppFrame *data);

/**
 * \brief 锟斤拷始锟斤拷msmart锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷.
 * \param handle  锟斤拷锟斤拷锟斤拷锟斤拷锟街革拷锟�.
 * \param event_cb 锟截碉拷锟斤拷锟斤拷锟斤拷锟斤拷锟秸碉拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷帧时锟斤拷锟皆讹拷锟斤拷锟斤拷.
 * \param ops_handle osal锟斤拷handle.
 * \param dev_info 锟矫伙拷锟斤拷锟捷的诧拷锟斤拷锟斤拷协锟斤拷栈锟节诧拷使锟矫ｏ拷锟斤拷锟斤拷锟节碉拷锟矫回碉拷锟斤拷时锟津传递革拷锟斤拷锟斤拷锟竭ｏ拷锟斤拷锟介传锟斤拷协锟斤拷栈锟斤拷锟斤拷锟斤拷锟竭碉拷handle.
 * \return 锟斤拷始锟斤拷锟斤拷锟斤拷状态.锟矫达拷锟斤拷状态锟斤拷锟斤拷要锟斤拷锟斤拷,锟斤拷锟揭憋拷锟斤拷要锟斤拷锟斤拷.
 * \note handle锟侥存储锟秸硷拷锟斤拷SDK锟剿凤拷锟斤拷,锟斤拷锟矫碉拷时锟斤拷锟斤拷SDK锟斤拷锟酵凤拷.
 */
E_Err ms_net_local_app_init(ST_HANDLE *handle, ms_net_local_event_cb event_cb,ST_HANDLE ops_handle, ms_dev_info_t *dev_info);

/**
 * \brief 协锟斤拷栈锟斤拷状态锟斤拷.\n
 * 锟斤拷锟斤拷盏锟斤拷锟斤拷锟街★拷锟斤拷锟斤拷ms_net_local_app_init锟斤拷锟斤拷锟斤拷录锟斤拷氐锟斤拷锟阶拷锟斤拷录锟斤拷氐锟斤拷胁锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟酵★拷锟斤拷偷锟斤拷锟斤拷锟斤拷锟斤拷锟�:
 * \param handle  锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟街革拷锟�.
 * \param tm_ms 锟斤拷前时锟戒，锟斤拷位锟斤拷锟诫，锟斤拷锟斤拷使锟斤拷gettimeofday锟斤拷锟截碉拷系统时锟斤拷.
 * \return 锟斤拷锟斤拷锟斤拷锟�.锟矫达拷锟斤拷状态锟斤拷锟斤拷要锟斤拷锟斤拷,锟斤拷锟揭憋拷锟斤拷要锟斤拷锟斤拷.
 * \note 1.锟斤拷锟斤拷锟斤拷一锟斤拷锟竭程诧拷锟较碉拷锟斤拷.\n
 *       2.锟剿接匡拷锟叫匡拷锟斤拷锟斤拷锟斤拷.\n
 *       3.锟斤拷锟斤拷锟斤拷绶拷锟斤拷斐ｏ拷锟斤拷私涌诨岱碉拷卮锟斤拷螅锟斤拷锟斤拷远锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷ms_net_local_app_exit之锟斤拷锟劫碉拷锟斤拷ms_net_local_app_init.\n
 */
E_Err ms_net_local_app_process(ST_HANDLE *handle, uint32_t tm_ms);

/**
 * \brief 锟斤拷锟斤拷M-Smart锟斤拷式锟斤拷锟斤拷锟斤拷帧.\n
 * 锟斤拷同帧锟斤拷锟酵碉拷锟斤拷锟斤拷锟斤拷锟斤拷:
 * \param handle  锟斤拷锟斤拷锟斤拷锟斤拷锟街革拷锟�.
 * \param p_frame 要锟斤拷锟酵碉拷锟斤拷锟斤拷帧.
 * \param args 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷未使锟矫ｏ拷锟斤拷锟介传锟斤拷NULL锟斤拷
 * \return 锟斤拷始锟斤拷锟斤拷锟斤拷状态.锟矫达拷锟斤拷状态锟斤拷锟斤拷要锟斤拷锟斤拷,锟斤拷锟揭憋拷锟斤拷要锟斤拷锟斤拷
 * \note 1.锟剿接匡拷锟叫匡拷锟斤拷锟斤拷锟斤拷.\n
 *       2.锟斤拷锟斤拷锟斤拷绶拷锟斤拷斐ｏ拷锟斤拷私涌诨岱碉拷卮锟斤拷螅锟斤拷锟斤拷远锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷ms_net_local_app_exit之锟斤拷锟劫碉拷锟斤拷ms_net_local_app_init.\n
 */
E_Err ms_net_local_app_send(ST_HANDLE *handle, T_MsNetLocalAppFrame *p_frame, void *args);

/**
 *\param handle 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷指锟斤拷.
 *\note  锟酵凤拷handle指锟斤拷锟绞憋拷锟斤拷锟斤拷锟酵凤拷handle指锟斤拷.
 */
E_Err ms_net_local_app_exit(ST_HANDLE *handle);

/**
 * \brief 锟斤拷取模锟斤拷ap模式锟铰碉拷ssid
 *\param ap_ssid 锟斤拷锟絘p模式ssid锟斤拷锟斤拷锟斤拷.
 *\param p_dev_info 锟斤拷锟斤拷锟斤拷锟结构锟藉，锟斤拷录锟斤拷锟借备锟斤拷锟斤拷息
 */
E_Err ms_sys_api_get_ap_ssid(uint8_t *ap_ssid,const ms_dev_info_t *p_dev_info);

#if defined(MS_SST)
/**
 * \brief 锟斤拷锟铰撅拷锟斤拷锟斤拷通锟脚的硷拷锟杰凤拷式
 *\param udp_key_id 锟斤拷锟斤拷锟斤拷息
 *\param p_dev_info锟借备锟斤拷息
 */
E_Err ms_net_local_app_update_udp_key_id(char *udp_key_id, ms_dev_info_t *p_dev_info);
#endif

#endif
