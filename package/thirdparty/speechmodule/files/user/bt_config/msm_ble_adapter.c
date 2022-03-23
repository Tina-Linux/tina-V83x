/**
 * @file msm_ble_adapter.c
 * @brief M-Smart BLE����??2??����y
 * @author D??��?��/��?3too
 * @version 1.0.0
 * @par Copyright  (c)
 *		Midea
 */

#include "unistd.h"

#include "msm_ble_adapter.h"
#include "ms_common.h"
#include "ms_ble_app.h"
#include "bluetooth.h"
#include <pthread.h>
#include <fcntl.h>

#define MS_BLE_CHAR_VALUE_HANDLE		(42)
#define MS_BLE_CHAR_VALUE_HANDLE2		(45)

#define MS_BLE_PT_CHAR_VALUE_RECV_HANDLE (52)
#define MS_BLE_PT_CHAR_VALUE_NOTIFY_HANDLE (55)


#define MS_UUID_INIT_WITH_UUID16(x)						\
	   {{0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80,					\
		0x00, 0x10, 0x00, 0x00,												\
							   (uint8_t)x,									\
									  (uint8_t)(x >> 8),					\
											0x00, 0x00}}		 /**< Bluetooth SIG base UUID. */

#define MS_BLE_MAX_RING_BUF_SIZE		(256*6)
static uint16_t ringbuf_total_len = 0;//for BLE recv buffer
static uint8_t BLE_RECV_RING_BUF[MS_BLE_MAX_RING_BUF_SIZE];
pthread_rwlock_t g_ring_lock;
static uint16_t s_connect_interval = 50;//default value

#if defined(CHIP_PLATFORM_MTK_7697)

#define MS_BLE_PACKED(...) __VA_ARGS__ __attribute__((__packed__))

/**
 * @brief bt_uuid union.
 */
typedef union ms_bt_uuid ms_bt_uuid_t;

MS_BLE_PACKED(union ms_bt_uuid{
	uint8_t uuid[16];			/**< An array to store 128-bit UUID. */
	MS_BLE_PACKED(struct {
		uint32_t reserved1[3];	/**< Placeholder. */
		uint32_t uuid32;		/**< 32-bit UUID. */
	});
	MS_BLE_PACKED(struct {
		uint16_t reserved2[6]; /**< Placeholder. */
		uint16_t uuid16;	   /**< 16-bit UUID. */
		uint16_t __zero16;	   /**< Placeholder. */
	});					   /**< Union of bt_uuid. */
});

//test implement
static uint16_t s_connectHandle = 0x0200;//default value
static uint16_t s_connect_interval = 50;//default value

const ms_bt_uuid_t MS_BLE_CHAR_UUID128 = MS_UUID_INIT_WITH_UUID16(MS_BLE_CHAR_UUID);
const ms_bt_uuid_t MS_BLE_CHAR_UUID128_2 = MS_UUID_INIT_WITH_UUID16(MS_BLE_NOTIFY_UUID);

//PT
const ms_bt_uuid_t MS_BLE_PT_CHAR_UUID128 = MS_UUID_INIT_WITH_UUID16(MS_BLE_PT_CHAR_UUID);
const ms_bt_uuid_t MS_BLE_PT_CHAR_UUID128_2 = MS_UUID_INIT_WITH_UUID16(MS_BLE_PT_NOTIFY_UUID);

static uint16_t ringbuf_total_len = 0;//for BLE recv buffer
static uint8_t BLE_RECV_RING_BUF[MS_BLE_MAX_RING_BUF_SIZE];

extern enum_ms_ble_service_enable_t ms_ble_service_status(void);


msm_result_t ble_init_ringbuf_op_mutex(void);
void ms_ble_load_to_ring_buf(uint8_t *buf,uint16_t len);

static xSemaphoreHandle ms_ble_buff_mtx = NULL;

void ms_set_connect_handle(uint16_t handle)
{
	s_connectHandle = handle;
	printf("handle: 0x%08x", handle);
}

void ms_set_connection_interval(uint16_t con_interval)
{
	s_connect_interval = con_interval;
	printf("set connect interval: 0x%04x\r\n", con_interval);
}
//////////////////////////////    BASICAL SERVICE CONF    ////////////////////////////////////////////////
/**
	SERVICES  CONFIG
	Do not change anything of these sentence below,
	unless you know exactly what will hapened correspondinglly.
*/

/**
	Key configuration of Midea Ble Service. It's the basical fund of tramsmition.
	Do not change anything of these sentence below,
	unless you know exactly what will hapened correspondinglly.
*/

/**
*	@@bref: when the mtu is not big enough for the buf sending, the service will sepreate the total packge into serval peieces,
*		   each will be sent in order.  But the final complete signal is send by this call back function. So it's the endpoint of a reciving
*		   event. This logic is not fitful for all modules, so this may lead to varial kinds of compatibility problems.
*
*	@para:	handle	->	the connection handle of link
*			flag -> the flag of current's callback
*/
bt_status_t bt_gatt_service_execute_write(uint16_t handle, uint8_t flag)
{
	printf("execute write.\n");
	return BT_STATUS_SUCCESS;
}

uint32_t ms_characteristic_recive_value_callback (const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
	uint8_t *pData = (uint8_t*)data;

	s_connectHandle = handle;

	//if(rw == BT_GATTS_CALLBACK_WRITE)
	//{
		printf("cb recived, rw:0x%02x\n", rw);
		if(size==0){
			printf("recv len = 0, ignore.\n");
			return (uint32_t)size;
		}

		if(ENUM_MS_BLE_SERVICE_CONFIG_ENABLE == ms_ble_service_status())
			ms_ble_load_to_ring_buf(pData, size);

		return (uint32_t)size;
	//}
	//printf("rw is not CALLBACK-WRITE\n\r");
   // return 0;
}




/**
*	@@bref: to configure the ble gatt service of recive's description callback interface,
*			Be careful, the name of these interface may have a special format, user should insist firmly to the rule.
*
*	@para:	rw	->	the recvied type of read or write
*			handle -> the connection handle of link
*			data ->	the recived raw data
*			size -> size of recived raw data
*			offset -> offset of the current part in each complete package
*/
static uint32_t ms_characteristic_recive_user_description_callback (const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    printf("ms_characteristic_recive_user_description_callback,rw = %d, handle = 0x%x, size = %d\r\n", rw, handle, size);
	MS_BLE_DBG_TRACE("recive_user_descrip_callback");
	PRINT_BUF((uint8_t*)data, size);

	if (rw == BT_GATTS_CALLBACK_WRITE)
	{
		printf("recv desciptor.");
	}else{
	return 0;
	}
    return (uint32_t)size;
}

/**
*	@@bref: to configure the ble gatt service of user send callback interface,
*			Be careful, the name of these interface may have a special format, user should insist firmly to the rule.
*
*	@para:	rw	->	the recvied type of read or write
*			handle -> the connection handle of link
*			data ->	the recived raw data
*			size -> size of recived raw data
*			offset -> offset of the current part in each complete package
*/
static uint32_t ms_characteristic_send_value_callback (const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    printf("ms_characteristic_send_value_callback,rw = %d, handle = 0x%x, size = %d\r\n", rw, handle, size);

	MS_BLE_DBG_TRACE("char-send-callback");
	PRINT_BUF((uint8_t*)data, size);

    if (rw == BT_GATTS_CALLBACK_WRITE) {
		printf("send value.");
	}else {
	return 0;
	}

    return (uint32_t)size;
}

/**
*	@@bref: to configure the ble gatt service of user send's decription callback interface,
*			Be careful, the name of these interface may have a special format, user should insist firmly to the rule.
*
*	@para:	rw	->	the recvied type of read or write
*			handle -> the connection handle of link
*			data ->	the recived raw data
*			size -> size of recived raw data
*			offset -> offset of the current part in each complete package
*/
static uint32_t ms_characteristic_send_user_description_callback (const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    printf("ms_characteristic_send_user_description_callback,rw = %d, handle = 0x%x, size = %d\r\n", rw, handle, size);

	MS_BLE_DBG_TRACE("char-send-descrip-callback");
	PRINT_BUF((uint8_t*)data, size);

    if (rw == BT_GATTS_CALLBACK_WRITE) {
		//to set characteristic's genearic description.
    }else {
	return 0;
	}
    return (uint32_t)size;
}

/**
*	@@bref: to configure the ble gatt service of client config's callback timeout interface,
*			Be careful, the name of these interface may have a special format, user should insist firmly to the rule.
*
*	@para:	xTimer	->	handle of timer
*/
void ble_ms_config_timeout_callback(TimerHandle_t xTimer){
	printf("ms_client_config_cb: timer cb\n\r");
}

/**
*	@@bref: to configure the ble gatt service of client config's callback interface,
*			Be careful, the name of these interface may have a special format, user should insist firmly to the rule.
*
*	@para:	rw	->	the recvied type of read or write
*			handle -> the connection handle of link
*			data ->	the recived raw data
*			size -> size of recived raw data
*			offset -> offset of the current part in each complete package
*/
static uint32_t ms_client_config_callback (const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    printf("ms_client_config_callback,rw = %d, handle = 0x%x, size = %d\r\n", rw, handle, size);

	MS_BLE_DBG_TRACE("client_config_callback");
	PRINT_BUF((uint8_t*)data, size);

	static TimerHandle_t s_bleTimer;
	//local
	static uint16_t s_indicateEnable;
	//add a timer
	if (s_bleTimer == NULL) {
		s_bleTimer = xTimerCreate("BLE_MS_XXX_TIMER",
								  1000 / portTICK_PERIOD_MS, pdFALSE,
								  ( void *)0,
								  ble_ms_config_timeout_callback);

		if (!s_bleTimer) {
			printf("[BLE_SMTCN]CCC, create timer fail, timer = 0x%x\n", (unsigned int)s_bleTimer);
		}
	}

    if (rw == BT_GATTS_CALLBACK_WRITE) {
	    if (size != sizeof(s_indicateEnable)){ //Size check
	        return 0;
	    }

		s_indicateEnable = *(uint16_t*)data;

	    if (s_indicateEnable == 0x0002)
		{
	        //send indication
	        if (xTimerStart(s_bleTimer, 0 ) != pdPASS )
			{
	            printf("[ ms ble ] timer start fail\n");
	        } else {
	            printf("[ ms ble ] timer start\n");
	        }
	    }
    }
	else
    {
        if (size!=0){
            memcpy(data, &(s_indicateEnable), sizeof(s_indicateEnable));
        }
    }

    return sizeof(s_indicateEnable);
}
//////////////////////////////////////// END OF BASICAL BLE CONF //////////////////////////////////////////////

BT_GATTS_NEW_PRIMARY_SERVICE_16(ms_primary_service, MS_BLE_SERVICE_UUID);

BT_GATTS_NEW_CHARC_16(ms_characteristic_recive,
                      BT_GATT_CHARC_PROP_WRITE | BT_GATT_CHARC_PROP_READ, MS_BLE_CHAR_VALUE_HANDLE, MS_BLE_CHAR_UUID);
BT_GATTS_NEW_CHARC_VALUE_CALLBACK(ms_characteristic_recive_value, MS_BLE_CHAR_UUID128,
				  BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE, ms_characteristic_recive_value_callback);
BT_GATTS_NEW_CHARC_USER_DESCRIPTION(ms_characteristic_recive_user_description,
                BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE, ms_characteristic_recive_user_description_callback);


BT_GATTS_NEW_CHARC_16(ms_characteristic_send,
                      BT_GATT_CHARC_PROP_INDICATE, MS_BLE_CHAR_VALUE_HANDLE2, MS_BLE_NOTIFY_UUID);
BT_GATTS_NEW_CHARC_VALUE_CALLBACK(ms_characteristic_send_value, MS_BLE_CHAR_UUID128_2,
				  BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE, ms_characteristic_send_value_callback);
BT_GATTS_NEW_CHARC_USER_DESCRIPTION(ms_characteristic_send_user_description,
                BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE, ms_characteristic_send_user_description_callback);
BT_GATTS_NEW_CLIENT_CHARC_CONFIG(ms_client_config,
							   BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE,
							   ms_client_config_callback);

static const bt_gatts_service_rec_t *ms_ble_service_rec[] = {
  (const bt_gatts_service_rec_t *) &ms_primary_service,
  (const bt_gatts_service_rec_t *) &ms_characteristic_recive,
  (const bt_gatts_service_rec_t *) &ms_characteristic_recive_value,
  (const bt_gatts_service_rec_t *) &ms_characteristic_recive_user_description,
  (const bt_gatts_service_rec_t *) &ms_characteristic_send,
  (const bt_gatts_service_rec_t *) &ms_characteristic_send_value,
  (const bt_gatts_service_rec_t *) &ms_characteristic_send_user_description,
  (const bt_gatts_service_rec_t *) &ms_client_config,
};

//[note] Must add ms_ble_service to the ble service array.
const bt_gatts_service_t ms_ble_config_service = {
  .starting_handle = 40,
  .ending_handle = 47,
  .required_encryption_key_size = 0,
  .records = ms_ble_service_rec
};

///todo
uint32_t ms_pt_recive_value_callback (const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
	uint8_t *pData = (uint8_t*)data;

	s_connectHandle = handle;

	//if(rw == BT_GATTS_CALLBACK_WRITE)
	//{
		printf("pt cb recived, rw:0x%02x\n", rw);
		if(size==0){
			printf("recv len = 0, ignore.\n");
			return (uint32_t)size;
		}

		printf("-------------------------[");
		PRINT_BUF(pData, size);
		printf("]\r\n");

		if(ENUM_MS_BLE_SERVICE_PT_ENABLE == ms_ble_service_status())
			ms_ble_load_to_ring_buf(pData, size);

		return (uint32_t)size;
	//}
	//printf("rw is not CALLBACK-WRITE\n\r");
   // return 0;
}




/**
*	@@bref: to configure the ble gatt service of recive's description callback interface,
*			Be careful, the name of these interface may have a special format, user should insist firmly to the rule.
*
*	@para:	rw	->	the recvied type of read or write
*			handle -> the connection handle of link
*			data ->	the recived raw data
*			size -> size of recived raw data
*			offset -> offset of the current part in each complete package
*/
static uint32_t ms_pt_recive_user_description_callback (const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    printf("// pt ms_characteristic_recive_user_description_callback,rw = %d, handle = 0x%x, size = %d\r\n", rw, handle, size);
	MS_BLE_DBG_TRACE("recive_user_descrip_callback");
	PRINT_BUF((uint8_t*)data, size);

	if (rw == BT_GATTS_CALLBACK_WRITE)
	{
		printf("recv desciptor.");
	}else{
	return 0;
	}
    return (uint32_t)size;
}

/**
*	@@bref: to configure the ble gatt service of user send callback interface,
*			Be careful, the name of these interface may have a special format, user should insist firmly to the rule.
*
*	@para:	rw	->	the recvied type of read or write
*			handle -> the connection handle of link
*			data ->	the recived raw data
*			size -> size of recived raw data
*			offset -> offset of the current part in each complete package
*/
static uint32_t ms_pt_send_value_callback (const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    printf("// pt ms_characteristic_send_value_callback,rw = %d, handle = 0x%x, size = %d\r\n", rw, handle, size);

	MS_BLE_DBG_TRACE("char-send-callback");
	PRINT_BUF((uint8_t*)data, size);

    if (rw == BT_GATTS_CALLBACK_WRITE) {
		printf("send value.");
	}else {
	return 0;
	}

    return (uint32_t)size;
}

/**
*	@@bref: to configure the ble gatt service of user send's decription callback interface,
*			Be careful, the name of these interface may have a special format, user should insist firmly to the rule.
*
*	@para:	rw	->	the recvied type of read or write
*			handle -> the connection handle of link
*			data ->	the recived raw data
*			size -> size of recived raw data
*			offset -> offset of the current part in each complete package
*/
static uint32_t ms_pt_send_user_description_callback (const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    printf("// pt ms_characteristic_send_user_description_callback,rw = %d, handle = 0x%x, size = %d\r\n", rw, handle, size);

	MS_BLE_DBG_TRACE("char-send-descrip-callback");
	PRINT_BUF((uint8_t*)data, size);

    if (rw == BT_GATTS_CALLBACK_WRITE) {
		//to set characteristic's genearic description.
    }else {
	return 0;
	}
    return (uint32_t)size;
}

/**
*	@@bref: to configure the ble gatt service of client config's callback timeout interface,
*			Be careful, the name of these interface may have a special format, user should insist firmly to the rule.
*
*	@para:	xTimer	->	handle of timer
*/
void ble_ms_pt_config_timeout_callback(TimerHandle_t xTimer){
	printf("// pt ms_client_config_cb: timer cb\n\r");
}

/**
*	@@bref: to configure the ble gatt service of client config's callback interface,
*			Be careful, the name of these interface may have a special format, user should insist firmly to the rule.
*
*	@para:	rw	->	the recvied type of read or write
*			handle -> the connection handle of link
*			data ->	the recived raw data
*			size -> size of recived raw data
*			offset -> offset of the current part in each complete package
*/
static uint32_t ms_pt_client_config_callback (const uint8_t rw, uint16_t handle, void *data, uint16_t size, uint16_t offset)
{
    printf("// pt ms_client_config_callback,rw = %d, handle = 0x%x, size = %d\r\n", rw, handle, size);

	MS_BLE_DBG_TRACE("client_config_callback");
	PRINT_BUF((uint8_t*)data, size);

	static TimerHandle_t s_bleTimer;
	//local
	static uint16_t s_indicateEnable;
	//add a timer
	if (s_bleTimer == NULL) {
		s_bleTimer = xTimerCreate("BLE_MS_XXX_TIMER",
								  1000 / portTICK_PERIOD_MS, pdFALSE,
								  ( void *)0,
								  ble_ms_pt_config_timeout_callback);

		if (!s_bleTimer) {
			printf("[BLE_SMTCN]CCC, create timer fail, timer = 0x%x\n", (unsigned int)s_bleTimer);
		}
	}

    if (rw == BT_GATTS_CALLBACK_WRITE) {
	    if (size != sizeof(s_indicateEnable)){ //Size check
	        return 0;
	    }

		s_indicateEnable = *(uint16_t*)data;

	    if (s_indicateEnable == 0x0002)
		{
	        //send indication
	        if (xTimerStart(s_bleTimer, 0 ) != pdPASS )
			{
	            printf("[ ms ble ] timer start fail\n");
	        } else {
	            printf("[ ms ble ] timer start\n");
	        }
	    }
    }
	else
    {
        if (size!=0){
            memcpy(data, &(s_indicateEnable), sizeof(s_indicateEnable));
        }
    }

    return sizeof(s_indicateEnable);
}

BT_GATTS_NEW_PRIMARY_SERVICE_16(ms_primary_pt_service, MS_BLE_PT_SERVICE_UUID);

BT_GATTS_NEW_CHARC_16(ms_pt_recive,
                      BT_GATT_CHARC_PROP_WRITE | BT_GATT_CHARC_PROP_READ, MS_BLE_PT_CHAR_VALUE_RECV_HANDLE, MS_BLE_PT_CHAR_UUID);
BT_GATTS_NEW_CHARC_VALUE_CALLBACK(ms_pt_recive_value, MS_BLE_PT_CHAR_UUID128,
				  BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE, ms_pt_recive_value_callback);
BT_GATTS_NEW_CHARC_USER_DESCRIPTION(ms_pt_recive_user_description,
                BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE, ms_pt_recive_user_description_callback);


BT_GATTS_NEW_CHARC_16(ms_pt_send,
                      BT_GATT_CHARC_PROP_INDICATE, MS_BLE_PT_CHAR_VALUE_NOTIFY_HANDLE, MS_BLE_PT_NOTIFY_UUID);
BT_GATTS_NEW_CHARC_VALUE_CALLBACK(ms_pt_send_value, MS_BLE_PT_CHAR_UUID128_2,
				  BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE, ms_pt_send_value_callback);
BT_GATTS_NEW_CHARC_USER_DESCRIPTION(ms_pt_send_user_description,
                BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE, ms_pt_send_user_description_callback);
BT_GATTS_NEW_CLIENT_CHARC_CONFIG(ms_pt_config,
							   BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE,
							   ms_pt_client_config_callback);

static const bt_gatts_service_rec_t *ms_ble_pt_service_rec[] = {
  (const bt_gatts_service_rec_t *) &ms_primary_pt_service,
  (const bt_gatts_service_rec_t *) &ms_pt_recive,
  (const bt_gatts_service_rec_t *) &ms_pt_recive_value,
  (const bt_gatts_service_rec_t *) &ms_pt_recive_user_description,
  (const bt_gatts_service_rec_t *) &ms_pt_send,
  (const bt_gatts_service_rec_t *) &ms_pt_send_value,
  (const bt_gatts_service_rec_t *) &ms_pt_send_user_description,
  (const bt_gatts_service_rec_t *) &ms_pt_config,
};

const bt_gatts_service_t ms_ble_pt_service = {
  .starting_handle = 50,
  .ending_handle = 57,
  .required_encryption_key_size = 0,
  .records = ms_ble_pt_service_rec
};
////////////////////////////////  END OF IMPLEMENT OF BLE BASICAL SERVICE CONF  ////////////////////////////////////

#else
///TODO
#endif

#if !defined(MS_WIFI_ENABLE)
void msm_timer_delay_ms(int ms)
{
#if defined(OSTYPE_LINUX)
	usleep(ms*1000);
#elif defined(OSTYPE_FREERTOS)
	vTaskDelay(ms/portTICK_RATE_MS);
#else
	///TODO
#endif
}

unsigned int msm_timer_tickcount_get(void)
{
#if defined(OSTYPE_LINUX)
	return times( NULL );
#elif defined(OSTYPE_FREERTOS)
	unsigned int  msRet;
	msRet =  ( uint32_t )xTaskGetTickCount();
	return msRet;
#else
	///TODO
	return 0;
#endif
}

void *msm_memory_alloc(unsigned int size)
{
#if defined(OSTYPE_LINUX)
	return malloc((size_t)size);
#elif defined(OSTYPE_FREERTOS)
	return pvPortMalloc( (size_t)size );
#else
	///TODO
#endif
}

void *msm_memory_calloc(size_t n, size_t size)
{
#if defined(OSTYPE_LINUX)
	return calloc(n, size);
#elif defined(OSTYPE_FREERTOS)
	void *p_buf = NULL;

	p_buf = msm_memory_alloc(n * size);
	//assert
	if(p_buf)
	{
		memset(p_buf, 0, n * size);
	}

	return p_buf;
#else
	///TODO
#endif
}

void msm_memory_free (void *pmem)
{
	if(pmem)
	{
	#if defined(OSTYPE_LINUX)
		free(pmem);
	#elif defined(OSTYPE_FREERTOS)
		vPortFree( pmem );
	#else
	///TODO
	#endif
	}
}

int msm_printf(const char* format, ...)
{
	  va_list ap;
         int ret = -1;

         va_start(ap, format);

#if defined(OSTYPE_LINUX)
	ret = vprintf(format, ap);
#elif defined(CHIP_PLATFORM_MTK_7697)
         ret = vprintf(format, ap);
#else
	///TODO...
#endif
         va_end(ap);

         return ret;
}

void msm_get_module_version(unsigned char *p_in, int len)
{
	unsigned char ver[6] = {0};

	///TODO
	ver[0] = 0x51;
	ver[1] = 0x00;
	//...

	memcpy(p_in, ver, sizeof(ver));
}
#endif


//flash
///TODO
uint8_t test_save_info[33] = {0};
msm_result_t msm_ble_flash_erase_info(void)
{
	///TODO
	memset(test_save_info, 0, sizeof(test_save_info));

	return MSM_RESULT_SUCCESS;
}

msm_result_t msm_ble_flash_read_info(uint8_t *buf, unsigned int len)
{
	///TODO
	if(len < sizeof(test_save_info))
	{
		memcpy(buf, test_save_info, len);
		MS_BLE_TRACE("msm ble read info[%d]:", len);
		PRINT_BUF(buf, len);
	}

	return MSM_RESULT_SUCCESS;
}

msm_result_t msm_ble_flash_save_info(uint8_t *buf, unsigned int len)
{
	///TODO
	if(len < sizeof(test_save_info))
	{
		memset(test_save_info, 0, sizeof(test_save_info));
		memcpy(test_save_info, buf, len);
		MS_BLE_TRACE("msm ble save info[%d]:", len);
		PRINT_BUF(test_save_info, len);
	}

	return MSM_RESULT_SUCCESS;
}


msm_result_t msm_ble_init_msble_attribute(ms_ble_stack_event_handler_t event_handler)
{
#if defined(CHIP_PLATFORM_MTK_7697)
	/*************************************************/
	// need to supply the 7697-ble-lowlevel-hal
	//Step1: config M-Smart BLE service(7697 has inited)
	//extern void bt_stack_event_handle_setup(ms_ble_stack_event_handler_t cb);
	//bt_stack_event_handle_setup(event_handler);

	return ble_init_ringbuf_op_mutex();
#else
	MS_BLE_TRACE("msm_ble_init_msble_attribute");
	///TODO
#endif
	return MSM_RESULT_SUCCESS;
}

msm_result_t msm_ble_set_mtu(uint16_t mtu)
{
#if defined(CHIP_PLATFORM_MTK_7697)
	bt_status_t tmpRet = BT_STATUS_FAIL;
	tmpRet = bt_gatts_set_max_mtu(mtu);
	if(BT_STATUS_SUCCESS != tmpRet){
		MS_BLE_ERR_TRACE("set mtu failed!\n\r");
		return MSM_RESULT_ERROR;
	}
#else
	///TODO
#endif
	return MSM_RESULT_SUCCESS;
}

/**
*@@bref: get mtu of current set
*
*@return: mtu <uint16_t>
*/
uint16_t msm_ble_get_mtu(void)
{
#if defined(CHIP_PLATFORM_MTK_7697)
	return bt_gattc_get_mtu(s_connectHandle);//mtu here Asserted to be 250
#else
	///TODO
	return 0;
#endif
}
uint16_t msm_ble_get_connection_interval(void)
{
	printf("get connect interval: 0x%04x\r\n", s_connect_interval);
	return s_connect_interval;
}

/*
*@function: turn on advertise
*@params  input: none
*
*
*@return MSM_RESULT_SUCCESS :turn on success
*			 MSM_RESULT_ERROR  :turn on fail(not power on ready or other reasons)
*@@attention:
*	This function is used to turn on the advertising exactlly.
*	The stack layer's behaviour is depended on this swtich's affect.
*/
msm_result_t msm_ble_set_on_adv(msm_ble_adv_content_t *adv)
{
#if defined(CHIP_PLATFORM_MTK_7697)
	bt_status_t ret;

	bt_hci_cmd_le_set_advertising_parameters_t ms_adv_param =
	{
			.advertising_interval_min = BLE_MCRC_MIN_INTERVAL,
			.advertising_interval_max = BLE_MCRC_MAX_INTERVAL,
			.advertising_type = BT_HCI_ADV_TYPE_CONNECTABLE_UNDIRECTED,
			.own_address_type = BT_ADDR_PUBLIC,
			.advertising_channel_map = MS_BLE_CHANNEL_NUM,
			.advertising_filter_policy = MS_BLE_FILTER_POLICY
	};

	bt_hci_cmd_le_set_advertising_enable_t ms_adv_enable;
	ms_adv_enable.advertising_enable = BT_HCI_ENABLE;

	bt_hci_cmd_le_set_advertising_data_t data;
	memcpy(data.advertising_data, adv->advData, adv->advDataLen);
	data.advertising_data_length = adv->advDataLen;

	bt_hci_cmd_le_set_scan_response_data_t scan_data = {0};
	scan_data.scan_response_data_length = adv->respDataLen,
	memcpy(scan_data.scan_response_data, adv->respData, adv->respDataLen);

	MS_BLE_DBG_TRACE("adv-data");
	PRINT_BUF(data.advertising_data, data.advertising_data_length);
	MS_BLE_DBG_TRACE("resp-data");
	PRINT_BUF(scan_data.scan_response_data, scan_data.scan_response_data_length);

	ret = bt_gap_le_set_advertising(&ms_adv_enable, &ms_adv_param, &data, &scan_data);

	if(BT_STATUS_SUCCESS!=ret){
		printf("adv action sent fail\n\r");
	}else{
		printf("adv action sent ok\n\r");
	}

	return (ret!=BT_STATUS_SUCCESS) ? MSM_RESULT_ERROR : MSM_RESULT_SUCCESS;
#else
	///TODO
	MS_TRACE("%s %d,adv data length:%d:", __FUNCTION__, __LINE__,adv->advDataLen);
	PRINT_BUF(adv->advData, adv->advDataLen);

	MS_TRACE("%s %d,resp data length:%d:", __FUNCTION__, __LINE__,adv->respDataLen);
	PRINT_BUF(adv->respData, adv->respDataLen);

	msm_write_config("/data/cfg/adv_content",adv,sizeof(msm_ble_adv_content_t));

    if (-1 == system("ble_wifi_introducer.sh start")) {
        printf("Start bsa ble failed, errno: %d\n", errno);
        return -1;
    }
    msm_ble_stack_callback(MS_BLE_STACK_EVENT_ADV_OK);
    sleep(1);
    msm_ble_stack_callback(MS_BLE_STACK_EVENT_CONNECTED);

	return MSM_RESULT_SUCCESS;
#endif
}

msm_result_t msm_ble_update_adv(msm_ble_adv_content_t *adv)
{
	//msm_ble_set_off_adv();
	return msm_ble_set_on_adv(adv);
}

/*
*@function: turn off advertise
*@params  input: none
*
*
*@return MSM_RESULT_SUCCESS :turn off success
*			 MSM_RESULT_ERROR  :turn off fail(not power on ready or other reasons)
*/
msm_result_t msm_ble_set_off_adv(void)
{
#if defined(CHIP_PLATFORM_MTK_7697)
	bt_status_t ret = -1;

	bt_hci_cmd_le_set_advertising_enable_t enable;
	enable.advertising_enable = BT_HCI_DISABLE;
	ret = bt_gap_le_set_advertising(&enable, NULL, NULL, NULL);
	if(BT_STATUS_SUCCESS!=ret){
		return MSM_RESULT_ERROR;
	}else{
		return MSM_RESULT_SUCCESS;
	}
#else
	bluetooth_control(BLE_CLOSE_SERVER,NULL,0);
	///TODO
	return MSM_RESULT_SUCCESS;
#endif
}


//BLE SEND
#if defined(CHIP_PLATFORM_MTK_7697)
static int32_t ms_ble_7697_send_data(uint8_t *data, int totalLen)
{
	printf("A-> ble start send ...\n\r");
	uint8_t sendData[256] = {0};
	if(totalLen>256)
	{
		printf("Data for sending is bigger than 256 !");
		return -1;
	}
	uint8_t sendLen = totalLen;
	memcpy(sendData, data, sendLen);
	bt_gattc_charc_value_notification_indication_t *req;

	uint8_t buf[256] = {0};

	memset(buf, 0, 256);
	req = (bt_gattc_charc_value_notification_indication_t*)buf;
	req->attribute_value_length = 3 + sendLen;
	req->att_req.opcode = BT_ATT_OPCODE_HANDLE_VALUE_INDICATION; //BT_ATT_OPCODE_HANDLE_VALUE_NOTIFICATION;
	if(ENUM_MS_BLE_SERVICE_CONFIG_ENABLE == ms_ble_service_status())
	{
		req->att_req.handle = MS_BLE_CHAR_VALUE_HANDLE2;//debugbyyip
	}
	else if(ENUM_MS_BLE_SERVICE_PT_ENABLE == ms_ble_service_status())
	{
		req->att_req.handle = MS_BLE_PT_CHAR_VALUE_NOTIFY_HANDLE;
	}
	else
	{
		TRACE("service not available, can't send data");
		return -1;
	}

	memcpy(req->att_req.attribute_value, sendData, sendLen);

	int32_t send_status = bt_gatts_send_charc_value_notification_indication(s_connectHandle, req);//0x0200
	printf("B-> ble finish send\n\r");
	return send_status;
}

msm_result_t ms_ble_send_mtu_data(uint8_t *data, int totalLen)
{
#if defined(CHIP_PLATFORM_MTK_7697)
	int32_t ret = ms_ble_7697_send_data(data, totalLen);
	if(BT_STATUS_SUCCESS==ret){
		return MSM_RESULT_SUCCESS;
	}
	else if(BT_STATUS_OUT_OF_MEMORY==ret)
	{
		MS_BLE_ERR_TRACE("out of memory!@%s:%d\n\r", __FUNCTION__, __LINE__);
	}
	else if(BT_STATUS_CONNECTION_IN_USE==ret)
	{
		MS_BLE_ERR_TRACE("connection in use!@%s:%d\n\r", __FUNCTION__, __LINE__);
	}
	else if(BT_STATUS_FAIL==ret)
	{
		MS_BLE_ERR_TRACE("op faild!@%s:%d\n\r", __FUNCTION__, __LINE__);
	}
	else{
		MS_BLE_ERR_TRACE("unknown error@%s:%d\n\r", __FUNCTION__, __LINE__);
	}
#else
	///TODO
#endif

	return MSM_RESULT_ERROR;
}

/**
*	@@bref: mutex to support the opreation of ringbuffer.
*		   Each process of the ring-buf-mutex, should be sychronized/protected by the mutex.
*	@para: none
*	@output: none
*/
msm_result_t ble_init_ringbuf_op_mutex(void)
{
	ms_ble_buff_mtx = xSemaphoreCreateMutex();
    if(ms_ble_buff_mtx == NULL)
    {
        MS_BLE_ERR_TRACE("creat mutex error");
		return MSM_RESULT_ERROR;
    }
	else
	{
		return MSM_RESULT_SUCCESS;
	}
}

/**
*	@@bref: BLE application layer fill the ringbuffer as recving a package( complete or not ), the ring buf
*		   should be locked when the opreation is conducting, and the mutex should be release as soon as
*		   the filling handle is finished. The ringbuffer is a cache for the progress to recive and save the raw data from app.
*		   Each process of the ring-buf-mutex, should be sychronized/protected by the mutex.
*	@para: buf, the input (recived) raw data
*	@output: len, the length of the raw data's vector in Bytes
*/
void ms_ble_load_to_ring_buf(uint8_t *buf,uint16_t len)
{
	static uint16_t offset = 0;
    uint16_t len_tp = 0;

	xSemaphoreTake(ms_ble_buff_mtx, portMAX_DELAY );

    ringbuf_total_len += len;
    if((offset+len) > sizeof(BLE_RECV_RING_BUF))
    {
        len_tp = sizeof(BLE_RECV_RING_BUF)-offset;
        memcpy(BLE_RECV_RING_BUF+offset,buf,len_tp);
        offset = 0;
        memcpy(BLE_RECV_RING_BUF, buf+len_tp, len-len_tp);
        offset +=(len-len_tp);
    }
    else
    {
        memcpy(BLE_RECV_RING_BUF+offset, buf, len);
        offset += len;
    }

	xSemaphoreGive(ms_ble_buff_mtx);
}

#endif


void ms_ble_load_to_ring_buf(uint8_t *buf,uint16_t len)
{
	static uint16_t offset = 0;
    uint16_t len_tp = 0;
    pthread_rwlock_wrlock(&g_ring_lock);//请求写锁

    ringbuf_total_len += len;
    if((offset+len) > sizeof(BLE_RECV_RING_BUF))
    {
        len_tp = sizeof(BLE_RECV_RING_BUF)-offset;
        memcpy(BLE_RECV_RING_BUF+offset,buf,len_tp);
        offset = 0;
        memcpy(BLE_RECV_RING_BUF, buf+len_tp, len-len_tp);
        offset +=(len-len_tp);
    }
    else
    {
        memcpy(BLE_RECV_RING_BUF+offset, buf, len);
        offset += len;
    }
    pthread_rwlock_unlock(&g_ring_lock);//解锁
}

msm_result_t msm_ble_data_send(uint8_t *data, int len)
{
#if defined(CHIP_PLATFORM_MTK_7697)
	uint16_t mtu=0;
	uint8_t *buf_tp=NULL;
	uint16_t len_tp = 0;
	msm_result_t send_ret;

	buf_tp = data;
	len_tp = len;
	mtu = msm_ble_get_mtu();//mtu here Asserted to be 250
	MS_BLE_TRACE("got mtu: %d\n", mtu);
	mtu = mtu - 3;
	MS_BLE_TRACE("max msg body: %d", mtu);

	int countsend = 0;
	MS_BLE_TRACE("ble send devise to %d parts!\n", len_tp/mtu+1);
	while(1)// seprate package into piece
	{
		countsend++;
		if(len_tp > mtu)
		{
			len_tp -= mtu;
		}
		else
		{
			mtu=len_tp;
			len_tp = 0;
		}
		MS_BLE_TRACE("left-len:%d, part-len:%d\n", len_tp, mtu);

		send_ret = ms_ble_send_mtu_data(buf_tp, mtu);
		MS_BLE_TRACE("part %dth send result: %d\n", countsend, send_ret);
		if(MSM_RESULT_SUCCESS != send_ret)
		{
			MS_BLE_ERR_TRACE("ble send error\n");
			return MSM_RESULT_ERROR;
		}

		if(len_tp == 0)
		{
			MS_BLE_TRACE("Send: all finish\n");
			return MSM_RESULT_SUCCESS;
		}
		buf_tp += mtu;
	}
#else
	MS_DBG_TRACE("msm_ble_data_send");
	bluetooth_control(BLE_SERVER_SEND,data,len);
	///TODO
#endif
}


/**
*	@@bref: This function parse a complete Ms-BLE-Application-Style's data, basically based on the header of 55 AA and
*		   the length of msg body. It is check by the tail byte with checksum, as decripted in the Software Design Description Document.
*
*	@para:  in_buf -> as a output buffer of the complete protocol data.
*		    in_len -> input as the max length of the goal package, and as the output para to store the current total package size.
*	@output: return the result of data parsing. if return:
*			-1, means there may be some fetal error occured;
*			0, means the ms-ble-conf-wifi application is parsing rule data successfully.
*/
int8_t msm_ble_data_recv(uint8_t *in_buf, uint16_t in_len)
{
   static uint16_t offset=0;
   uint16_t len_tp = 0;

   if(in_len > ringbuf_total_len)
   {
        return -1;
   }
   else
   {
	    pthread_rwlock_rdlock(&g_ring_lock);//请求读锁

        ringbuf_total_len -= in_len;

        if(in_len+offset > sizeof(BLE_RECV_RING_BUF))
        {
            len_tp = sizeof(BLE_RECV_RING_BUF)-offset;
            memcpy(in_buf, BLE_RECV_RING_BUF+offset, len_tp);
            offset = 0;
            memcpy(in_buf+len_tp, BLE_RECV_RING_BUF, in_len-len_tp);
            offset += (in_len-len_tp);
        }
        else
        {
            memcpy(in_buf, BLE_RECV_RING_BUF+offset, in_len);
            offset += in_len;
        }
        pthread_rwlock_unlock(&g_ring_lock);//解锁
   }

   return 0;
}


void msm_ble_disconnect(void)
{
#if defined(CHIP_PLATFORM_MTK_7697)
	bt_hci_cmd_disconnect_t disconnect = {
	    .connection_handle = 0x0200,
	    .reason = BT_HCI_STATUS_REMOTE_USER_TERMINATED_CONNECTION,
	};
	disconnect.connection_handle = s_connectHandle;
    MS_BLE_TRACE("do close ble connect\n");
    bt_gap_le_disconnect(&disconnect);

#else
	///TODO
{

    MS_BLE_TRACE("do close ble connect\n");

    //	char data[4]={0xff,0xff,0xff,0xfe};
    //	bluetooth_control(BLE_SERVER_SEND,data,4);
}

#endif
}

void msm_ble_clear_bonded_info(void)
{
#if defined(CHIP_PLATFORM_MTK_7697)
	extern void clear_bonded_info();
	clear_bonded_info();
#else
	///TODO
#endif
}

uint32_t msm_ble_get_system_tick_in_msecond()
	{
	#if defined(CHIP_PLATFORM_MTK_7697)
	return msm_timer_tickcount_get()/portTICK_PERIOD_MS;
	#else
	struct timeval current_tm;
	gettimeofday(&current_tm, NULL);
	return (current_tm.tv_sec<<10) + (current_tm.tv_usec>>10);
	//return sysconf(_SC_CLK_TCK)>>10;	///TODO
	#endif
	}
