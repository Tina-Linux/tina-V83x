/**
 * @file msm_ble_adapter.h
 * @brief M-Smart BLE适配层头文件
 * @author 叶楚汉/谢建军
 * @version 1.0.0
 * @par Copyright  (c)
 *		Midea
 */

#ifndef __MSM_BLE_ADAPTER_H__
#define __MSM_BLE_ADAPTER_H__

#include "msm_adapter.h"
//#include "ms_ble_app.h"

/**
 * @brief       M-Smart BLE SDK广播及扫描响应消息结构体
 */
typedef struct MSM_BLE_ADV_CONTENT_T
{
	uint8_t advData[32];	/*!< 广播内容*/
	uint8_t advDataLen;	/*!< 广播长度*/
	uint8_t respData[32];	/*!< 扫描响应内容*/
	uint8_t respDataLen;	/*!< 扫描响应长度*/
}msm_ble_adv_content_t;

/**
 * @brief       M-Smart BLE SDK协议栈事件枚举
 */
typedef enum MS_BLE_STACK_EVENT_T{
	MS_BLE_STACK_EVENT_STACK_READY = 0,		/*!< 协议栈初始化成功*/
	MS_BLE_STACK_EVENT_STACK_FAIL,			/*!< 协议栈初始化失败*/
	MS_BLE_STACK_EVENT_ADV_OK,				/*!< 开启或关闭广播执行成功*/
	MS_BLE_STACK_EVENT_DISCONNECT,			/*!< 对端断开连接*/
	MS_BLE_STACK_EVENT_CONNECTED,			/*!< 对端建立连接*/
}ms_ble_stack_event_t;

/**
 * @brief	void (*ms_ble_stack_event_handler_t)(ms_ble_stack_event_t event)
 * ble底层协议栈事件回调函数
 * @param[in]	event	事件
 * @note	此回调函数非常重要，ble sdk基础逻辑靠此回调函数事件驱动
 */
typedef void (*ms_ble_stack_event_handler_t)(ms_ble_stack_event_t event);

#if !defined(MS_WIFI_ENABLE)
#if 0
/**
 * @brief       适配接口结果枚举值
 */
typedef enum{
        MSM_RESULT_SUCCESS = 0, /*!< 接口调用成功*/
        MSM_RESULT_ERROR, /*!< 接口调用未成功 */
}msm_result_t;
#endif

/**
 * @brief	msm_timer_delay_ms(int ms)
 * 延时函数
 * @param[in]	ms	设置的延时时间，单位为毫秒
 */
void msm_timer_delay_ms(int ms);

/**
 * @brief	msm_timer_tickcount_get(void)
 * 处理器开机经过的tick数
 * @return	返回的tick数
 */
unsigned int msm_timer_tickcount_get(void);

/**
 * @brief	msm_memory_alloc(unsigned int size)
 * 申请动态内存
 * @param[in]	size	申请动态内存大小
 * @return	返回申请内存的首地址
 */
void *msm_memory_alloc(unsigned int size);
/**
 * @brief	msm_memory_calloc(size_t n, size_t size)
 * 申请动态内存，一般为数组形式的
 * @param[in]	n	申请内存元素的个数
 * @param[in]	size	每个元素大小
 * @return	返回申请内存的首地址
 */
void *msm_memory_calloc(size_t n, size_t size);

/**
 * @brief	msm_memory_free(void *pmem)
 * 释放动态内存
 * @param[in]	pmem	内存的首地址
 */
void msm_memory_free (void *pmem);

/**
 * @brief msm_printf(const char* format, ...)
 * 格式化输出函数
 * @param[in]	format	格式控制信息
 * @param[in]	...	可选参数，可以是任何类型的数据
 * @return	小于0	输出失败 \n
 *		大于等于0	输出的长度 \n
 */
int msm_printf(const char* format, ...);

/**
* @brief	Get the module version(unsigned char *p_in, int len)
 * 获取集成M-Smart Middleware的设备版本号，长度固定为六个字节
* @parameter[in]	p_in	input data
* @parameter[in]	len	The lenght of input data
* @return	NULL
**/
void msm_get_module_version(unsigned char *p_in, int len);
#endif

/**
 * @brief	msm_ble_flash_erase_info(void)
 * 从flash读取ble数据
 * @param[out]	buf		读取数据缓存区
 * @param[in]	len		读取数据长度[最大长度不超过512字节]
 * @return	MSM_RESULT_SUCCESS	成功 \n
 *			MSM_RESULT_ERROR	失败
 */
msm_result_t msm_ble_flash_erase_info(void);

/**
 * @brief	msm_ble_flash_read_info(uint8_t *buf, unsigned int len)
 * 从flash读取ble数据
 * @param[out]	buf		读取数据缓存区
 * @param[in]	len		读取数据长度[最大长度不超过512字节]
 * @return	MSM_RESULT_SUCCESS	成功 \n
 *			MSM_RESULT_ERROR	失败
 */
msm_result_t msm_ble_flash_read_info(uint8_t *buf, unsigned int len);

/**
 * @brief	msm_ble_flash_save_info(uint8_t *buf, unsigned int len)
 * 存储ble数据至flash
 * @param[in]	buf		待存数据缓存区
 * @param[in]	len		待存数据长度[最大长度不超过512字节]
 * @return	MSM_RESULT_SUCCESS	成功 \n
 *			MSM_RESULT_ERROR	失败
 */
msm_result_t msm_ble_flash_save_info(uint8_t *buf, unsigned int len);

/**
 * @brief	msm_ble_init_msble_attribute(ms_ble_stack_event_handler_t event_handler)
 * 初始化ble stack/gap/gatt/stack_event_handler
 * @param[in]	event_handler		ble底层事件回调函数，事件枚举名为ms_ble_stack_event_t
 * @return	MSM_RESULT_SUCCESS	成功 \n
 *			MSM_RESULT_ERROR	失败
 */
msm_result_t msm_ble_init_msble_attribute(ms_ble_stack_event_handler_t event_handler);

/**
 * @brief	msm_ble_set_mtu(uint16_t mtu)
 * 设置ble mtu(Maximum Transmission Unit)
 * @param[in]	mtu		mtu值
 * @return	MSM_RESULT_SUCCESS	成功 \n
 *			MSM_RESULT_ERROR	失败
 */
msm_result_t msm_ble_set_mtu(uint16_t mtu);

/**
 * @brief	msm_ble_get_mtu(void)
 * 获取ble mtu(Maximum Transmission Unit)
 * @return	mtu值
 */
uint16_t msm_ble_get_mtu(void);

/**
 * @brief	msm_ble_set_on_adv(msm_ble_adv_content_t *adv)
 * 开启ble广播
 * @param[in]	adv		广播及扫描响应消息结构体指针
 * @return	MSM_RESULT_SUCCESS	成功 \n
 *			MSM_RESULT_ERROR	失败
 */
msm_result_t msm_ble_set_on_adv(msm_ble_adv_content_t *adv);

/**
 * @brief	msm_ble_update_adv(msm_ble_adv_content_t *adv)
 * 刷新ble广播内容
 * @param[in]	adv		广播及扫描响应消息结构体指针
 * @return	MSM_RESULT_SUCCESS	成功 \n
 *			MSM_RESULT_ERROR	失败
 */
msm_result_t msm_ble_update_adv(msm_ble_adv_content_t *adv);

/**
 * @brief	msm_ble_set_off_adv(void)
 * 关闭ble广播
 * @return	MSM_RESULT_SUCCESS	成功 \n
 *			MSM_RESULT_ERROR	失败
 */
msm_result_t msm_ble_set_off_adv(void);

/**
 * @brief	msm_ble_data_send(uint8_t *data, int totalLen)
 * 发送ble数据，若mtu小于totalLen，则需要在此函数内实现分包发送
 * @param[in]	data	发送数据缓存区
 * @param[in]	totalLen	发送数据长度
 * @return	MSM_RESULT_SUCCESS	成功 \n
 *			MSM_RESULT_ERROR	失败
 * @note	M-Smart BLE协议共支持两个服务，具体往哪个服务发送数据根据以下方式进行判断：
            调用ms_ble_service_status()函数获取当前可用的服务，若返回为ENUM_MS_BLE_SERVICE_CONFIG_ENABLE，则往
            配置服务发送数据，若返回为ENUM_MS_BLE_SERVICE_PT_ENABLE，则往透传服务发送数据，具体可参考msm_ble_adapter.c
            的ms_ble_7697_send_data接口
 */
msm_result_t msm_ble_data_send(uint8_t *data, int totalLen);

/**
 * @brief	msm_ble_data_recv(uint8_t *in_buf, uint16_t in_len)
 * 读取ble数据，只有满足要求长度的数据才需要填充到接收数据缓存区
 * @param[out]	in_buf	接收数据缓存区
 * @param[in]	in_len	接收数据长度
 * @return	0	接收到in_len长度的字节 \n
 *			-1	无法接收到in_len长度的字节
 */
int8_t msm_ble_data_recv(uint8_t *in_buf, uint16_t in_len);


/**
 * @brief	msm_ble_get_system_tick_in_msecond(void)
 * 处理器开机经过的时间，毫秒级
 * @return	返回时间值
 */
uint32_t msm_ble_get_system_tick_in_msecond(void);

/**
 * @brief	msm_ble_disconnect(void)
 * 断开BLE连接
 * @return	NULL
 */
void msm_ble_disconnect(void);

/**
 * @brief	msm_ble_clear_bonded_info(void)
 * 清除BLE绑定信息
 * @return	NULL
 */
void msm_ble_clear_bonded_info(void);

#endif
