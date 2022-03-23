/**
 * @file ms_ble_app.h
 * @brief M-Smart BLE头文件
 * @author 叶楚汉/谢建军
 * @version 1.0.0
 * @par Copyright  (c)
 *		Midea
 */

#ifndef __MS_BLE_APP_H__
#define __MS_BLE_APP_H__

#if defined(MS_WIFI_ENABLE)
#include "msm_adapter.h"
#else
#include "msm_ble_adapter.h"
#endif

/**
 * @brief       M-Smart BLE 读服务规范
 */
#define MS_BLE_CHAR_UUID			(0xff81)//(0x81ff)

/**
 * @brief       M-Smart BLE 写服务规范
 */
#define MS_BLE_NOTIFY_UUID		(0xff82)//(0x81ff)

/**
 * @brief       M-Smart BLE 服务UUID
 */
#define MS_BLE_SERVICE_UUID			(0xff80)//(0x80ff) // Data Transfer Service

/**
 * @brief       M-Smart BLE 读服务规范
 */
#define MS_BLE_PT_CHAR_UUID			(0xff91)//(0x81ff)

/**
 * @brief       M-Smart BLE 写服务规范
 */
#define MS_BLE_PT_NOTIFY_UUID		(0xff92)//(0x81ff)

/**
 * @brief       M-Smart BLE 服务UUID
 */
#define MS_BLE_PT_SERVICE_UUID			(0xff90)//(0x80ff) // Data Transfer Service

/**
 * @brief       Minimum advertising interval.The range is from 0x0020 to 0x4000.
 */
#define BLE_MCRC_MIN_INTERVAL		(0x01E0)    /*0.3s*/

/**
 * @brief       Maximum advertising interval.The range is from 0x0020 to 0x4000 and should be greater than the minimum advertising interval.
 */
#define BLE_MCRC_MAX_INTERVAL		(0x01E0)    /*0.3s*/

/**
 * @brief       Advertising channel map
 */
#define MS_BLE_CHANNEL_NUM		(7)

/**
 * @brief       Advertising filter policy
 */
#define MS_BLE_FILTER_POLICY		(0)

/**
 * @brief       M-Smart BLE service 使能情况
 */
typedef enum ENUM_MS_BLE_SERVICE_ENABLE_T
{
	ENUM_MS_BLE_SERVICE_DISABLE = 0x00,		/*!< 配置及透传服务不使能*/
	ENUM_MS_BLE_SERVICE_CONFIG_ENABLE,			/*!< 配置服务使能*/
	ENUM_MS_BLE_SERVICE_PT_ENABLE,				/*!< 透传服务使能*/
}enum_ms_ble_service_enable_t;

/**
 * @brief       M-Smart云端登陆状态
 */
typedef enum ENUM_MS_CLOUD_STATE_T
{
	ENUM_MS_CLOUD_STATE_LOGOUT = 0x00,	/*!< 未登陆*/
	ENUM_MS_CLOUD_STATE_LOGIN,			/*!< 已登陆*/
}enum_ms_cloud_state_t;

/**
 * @brief       登陆云过程
 */
typedef enum ENUM_LOGIN_CLOUD_STATE_T
{
	ENUM_LOGIN_CLOUD_STATE_NOT_STATE = 0x00,	/*!< 未有配网信息*/
	ENUM_LOGIN_CLOUD_STATE_CONNECTING_ROUTER,	/*!< 连接路由中*/
	ENUM_LOGIN_CLOUD_STATE_IN_DNS,				/*!< DNS解析中*/
	ENUM_LOGIN_CLOUD_STATE_LOGINING_CLOUD,		/*!< 登陆服务器中*/
	ENUM_LOGIN_CLOUD_STATE_LOGIN,				/*!< 登陆成功*/
}enum_login_cloud_state_t;

/**
 * @brief       登陆云错误码
 */
typedef enum ENUM_LOGIN_CLOUD_ERR_T
{
	ENUM_LOGIN_CLOUD_ERR_SUCCESS = 0X00,		/*!< 无*/
	ENUM_LOGIN_CLOUD_ERR_NOT_FIND_SSID,			/*!< 找不到SSID*/
	ENUM_LOGIN_CLOUD_ERR_CONNECT_AP,			/*!< 连接路由失败*/
	ENUM_LOGIN_CLOUD_ERR_DNS,					/*!< DNS解析失败*/
	ENUM_LOGIN_CLOUD_ERR_CONNECT_SERVER,		/*!< 与服务器建立TCP连接失败*/
	ENUM_LOGIN_CLOUD_ERR_HB_TIMEOUT,			/*!< 心跳超时*/
	ENUM_LOGIN_CLOUD_ERR_SST,					/*!< 登陆过程SST错误*/
	ENUM_LOGIN_CLOUD_ERR_POSITIVE_REBOOT,		/*!< 模组主动重启*/
	ENUM_LOGIN_CLOUD_ERR_PASSIVE_REBOOT,		/*!< 模组被动重启*/
	ENUM_LOGIN_CLOUD_ERR_SDK_CHECK,				/*!< SDK认证失败*/
	ENUM_LOGIN_CLOUD_ERR_SERVER_CLOSE_SOCKET,	/*!< 登陆过程被服务器主动关闭*/
	ENUM_LOGIN_CLOUD_ERR_SEND_DATA,				/*!< 登陆过程发送数据失败*/
}enum_login_cloud_err_t;

/**
 * @brief       BLE SDK与应用层通讯事件
 */
typedef enum
{
	MS_BLE_EVENT_BLE_STACK_READY,		/*!< [sdk ->user] BLE协议栈初始化成功*/
	MS_BLE_EVENT_WIFI_STATE_CHANGE,		/*!< [user ->sdk] Wi-Fi模块状态改变，注：ms_ble_wifi_info_t结构体任一成员发生变化，应用层均需通过此事件通知sdk*/
	MS_BLE_EVENT_TRANS,				/*!< [user ->sdk;sdk ->user] 透传指令*/
	MS_BLE_EVENT_CONFIG,				/*!< [sdk ->user;user -> sdk] sdk->user:BLE配网情况通知;user->sdk:外部配网情况通知BLE SDK*/
	MS_BLE_EVENT_CONNECTED,				/*!< [sdk ->user] BLE已连接*/
	MS_BLE_EVENT_DISCONNECT,			/*!< [sdk ->user] BLE链路断开*/
	MS_BLE_EVENT_PT_READY,				/*!< [sdk ->user] 控制模式:手机鉴权成功*/
	MS_BLE_EVENT_USER_AUTH_OK,			/*!< [user ->sdk] 用户鉴权*/
}ms_ble_event_t;

/**
 * @brief       BLE SDK应用事件消息结构体
 */
typedef struct MS_BLE_EVENT_FRAME_T
{
	ms_ble_event_t event;	/*!< BLE SDK与应用层通讯事件*/
	uint8_t data[300];		/*!< 消息内容*/
	uint16_t dataSize;		/*!< 消息长度*/
	uint8_t msgId;			/*!< 消息ID，预留*/
}ms_ble_event_frame_t;

/**
 * @brief	int(*cb_fun)(ms_ble_event_frame_t *pframe)
 * ble sdk事件回调函数,由应用层在调用ms_ble_app_init时传入,sdk通过此回调函数通知应用层做事件处理
 * @param[in]	pframe	BLE SDK应用事件消息结构体指针
 * @return		0		成功\n
 *			    其它	失败
 * @note	此回调函数非常重要，ble 应用层基础逻辑靠此回调函数事件驱动
 */
typedef int(*cb_fun)(ms_ble_event_frame_t *pframe);

/**
 * @brief	BLE逻辑所关注的Wi-Fi信息结构体
 */
typedef struct MS_BLE_WIFI_INFO_T
{
	enum_login_cloud_state_t login_cloud_state;	/*!< 连云状态*/
	enum_login_cloud_err_t login_cloud_errcode;	/*!< 连云错误码*/
}ms_ble_wifi_info_t;

/**
 * @brief	BLE初始化状态
 */
typedef enum ENUM_MS_BLE_INIT_STATE_T
{
	ENUM_MS_BLE_INIT_STATE_UNCONFIG_USER_NOT_AUTH = 0,	/*!< 未配置，未用户鉴权*/
	ENUM_MS_BLE_INIT_STATE_UNCONFIG_USER_AUTH,				/*!< 未配置，已用户鉴权*/
	ENUM_MS_BLE_INIT_STATE_CONFIGURED,							/*!< 已配置*/
}enum_ms_ble_init_state_t;

/**
 * @brief	M-Smart BLE初始化参数结构体
 */
typedef struct MS_BLE_INIT_PARA_T
{
	enum_login_cloud_state_t cloud_logined_state;			/*!< M-Smart云登陆状态*/
	enum_ms_ble_init_state_t ble_init_state;			/*!< BLE初始化状态*/
	uint8_t mac[6];										/*!< MAC地址，小端模式*/
	uint8_t device_ip[4];								/*!< IP地址，小端模式*/
	uint8_t device_type;								/*!< 品类码*/
	uint8_t sub_device_type_low;						/*!< 设备型号码低字节，从0XA0串口消息体获取*/
	uint8_t sub_device_type_high;						/*!< 设备型号码高字节，从0XA0串口消息体获取*/
	uint8_t sn[33];										/*!< sn，最后一个字节为'\0'*/
	cb_fun cb;											/*!< 应用层注册至sdk的事件回调函数*/
}ms_ble_init_para_t;

/**
 * @brief	操作码
 */
typedef enum MS_OP_CODE_T
{
	MS_OP_CONFIG_SUCCESS								= 0x00,		/*!< BLE配网成功*/
	MS_OP_CONFIG_TIMEOUT								= 0x01,		/*!< BLE配网超时*/
	MS_OP_EXCEPTION										= 0x02,		/*!< 操作异常*/
	MS_OP_RESERVED										= 0xff,		/*!< SDK内部预留*/
}ms_op_code_t;

/**
 * @brief	MS_BLE_EVENT_CONFIG事件所对应的消息体
 */
typedef struct MS_BLE_CONFIG_RET_CB_T
{
	ms_op_code_t result_code;		/*!< 操作码*/
	char ssid[33];					/*!< ssid内容*/
	char pwd[65];					/*!< pwd内容*/
	uint8_t ssid_len;				/*!< ssid长度*/
	uint8_t pwd_len;				/*!< pwd长度*/
	uint8_t random[16];				/*!< 配网随机数*/
}ms_ble_config_ret_cb_t;

/**
 * @brief	ms_ble_app_init(ms_ble_init_para_t *para)
 * 初始化ble sdk
 * @param[in]	para	初始化参数
 * @return	MSM_RESULT_SUCCESS	初始化成功 \n
 *			MSM_RESULT_ERROR	初始化失败
 */
msm_result_t ms_ble_app_init(ms_ble_init_para_t *para);

/**
 * @brief	ms_ble_app_usr_msg_handle(ms_ble_event_frame_t *frame)
 * ble sdk处理应用层事件
 * @param[in]	frame	事件帧参数
 * @return	NULL
 */
void ms_ble_app_usr_msg_handle(ms_ble_event_frame_t *frame);

/**
* @brief   ms_ble_app_process(void)
* ble sdk逻辑运行函数
* @return  NULL
*/
void ms_ble_app_process(void);

#endif
