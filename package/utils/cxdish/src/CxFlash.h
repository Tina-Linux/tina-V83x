/****************************************************************************************
*****************************************************************************************
***                                                                                   ***
***                                 Copyright (c) 2015                                ***
***                                                                                   ***
***                                Conexant Systems, Inc.                             ***
***                                                                                   ***
***                                 All Rights Reserved                               ***
***                                                                                   ***
***                                    CONFIDENTIAL                                   ***
***                                                                                   ***
***               NO DISSEMINATION OR USE WITHOUT PRIOR WRITTEN PERMISSION            ***
***                                                                                   ***
*****************************************************************************************
**
**  File Name:
**      cxflash.c
**
**  Abstract:
**      This code is to download the firmware to CX2070X device via I2C bus.
**
**
**  Product Name:
**      Conexant Hudson
**
**  Remark:
**
**  Version: 3.14.0.0.
********************************************************************************
*****************************************************************************************/
#ifdef __cplusplus
extern "C"{
#endif


typedef int (*fn_I2cReadMem)(  void * context,
                                      unsigned char slave_addr,
                                      uint32_t sub_addr,
                                      uint32_t rd_len,
                                      unsigned char*rd_buf);


typedef int (*fn_I2cWriteMem)(  void * context,
                              unsigned char slave_addr,
                              uint32_t sub_addr,
                              uint32_t wr_len,
                              unsigned char*wr_buf);

typedef int (*fn_SetResetPin) ( void* pCallbackContext,
                                int         bSet);
/*
 * The SetupI2cWriteMemCallback sets the I2cWriteMem callback function.
 *
 * PARAMETERS
 *
 *    pCallbackContext [in] - A pointer to a caller-defined structure of data items
 *                            to be passed as the context parameter of the callback
 *                            routine each time it is called.
 *
 *    I2cWritePtr      [in] - A pointer to a i2cwirte callback routine, which is to
 *                            write I2C data. The callback routine must conform to
 *                            the following prototype:
 *
 *                        int (*fn_I2cWriteMem)(
 *                                void * context,
 *                                unsigned char slave_addr,
 *                                unsigned long sub_addr,
 *                                unsigned long write_len,
 *                                unsigned char* write_buf,
 *                             );
 *
 *                        The callback routine parameters are as follows:
 *
 *                        context          [in] - A pointer to a caller-supplied
 *                                                context area as specified in the
 *                                                CallbackContext parameter of
 *                                                SetupI2cWriteMemCallback.
 *                        slave_addr       [in] - The i2c chip address.
 *                        sub_addr         [in] - The i2c data address.
 *                        write_len        [in] - The size of the output buffer, in bytes.
 *                        write_buf        [in] - A pointer to the putput buffer that contains
 *                                                the data required to perform the operation.
 *
 *    cbMaxWriteBufSize  [in] - Specify the maximux transfer size for a I2c continue
 *                              writing with 'STOP'. This is limited in I2C bus Master
 *                              device. The size can not less then 4.
 *
 * RETURN
 *
 *    None
 *
 */
void SetupI2cWriteMemCallback( void * pCallbackContext,
                              fn_I2cWriteMem       I2cWritePtr,
                              uint32_t        cbMaxWriteBufSize);

/*
 * The SetupI2cReadMemCallback sets i2cReadMem callback function.
 *
 * PARAMETERS
 *
 *    pCallbackContext    [in] - A pointer to a caller-defined structure of data items
 *                               to be passed as the context parameter of the callback
 *                               routine each time it is called.
 *
 *    pI2cReadMemPtr      [in] - A pointer to a i2cwirte callback routine, which is to
 *                               write I2C data. The callback routine must conform to
 *                               the following prototype:
 *
 *                        int (*fn_I2cReadMem)(
 *                                void * context,
 *                                unsigned char slave_addr,
 *                                unsigned long sub_addr,
 *                                unsigned long rd_len
 *                                void*         rd_buf,
 *                             );
 *
 *                        The callback routine parameters are as follows:
 *
 *                         context            [in]  - A pointer to a caller-supplied
 *                                                    context area as specified in the
 *                                                    CallbackContext parameter of
 *                                                    SetupI2cWriteMemCallback.
 *                         slave_addr         [in]  - The i2c chip address.
 *                         sub_addr           [in]  - slave addr.
 *                         rd_buf             [out] - A pointer to the input buffer
 *                         rd_len             [in]  - Specify the read data size.
 *
 *   cbMaxI2CRead         [in] - Specify the maximux transfer size for a I2c continue
 *                               read. The size can not less then 4.
 * RETURN
 *      None
 *
 */
void SetupI2cReadMemCallback( void * pCallbackContext,
    fn_I2cReadMem I2cWriteThenReadPtr,
    uint32_t cbMaxI2CRead);



/*
 * Set the SetResetPin callback function.
 *
 * PARAMETERS
 *
 *    pCallbackContext    [in] - A pointer to a caller-defined structure of data items
 *                               to be passed as the context parameter of the callback
 *                               routine each time it is called.
 *
 *    SetResetPinPtr      [in] - A pointer to a i2cwirte callback routine, which is to
 *                               write I2C data. The callback routine must conform to
 *                               the following prototype:
 *
 *                        int (*fn_SetResetPin)(
 *                                  void * pCallbackContext,
 *                                  int    bSet );
 *
 *                        The callback routine parameters are as follows:
 *
 *                         pCallbackContext [in] - A pointer to a caller-supplied
 *                                                 context area as specified in the
 *                                                 CallbackContext parameter of
 *                                                 SetupI2cWriteMemCallback.
 *                         bSet             [in] - Indicates whether to high or low the GPIO pin.
 *
 * RETURN
 *
 *    If the operation completes successfully, the return value is ERRNO_NOERR.
 *    Otherwise, return ERRON_* error code.
 *
 */
void SetupSetResetPin(  void * pCallbackContext,
                        fn_SetResetPin  SetResetPinPtr);

/*
 * cx_get_buffer_size.
 *
 *  Calculates the buffer size required for firmware update processing..
 *
 * PARAMETERS
 *    None
 *
 * RETURN
 *
 *    return buffer size required for firmware update processing..
 *
 */
uint32_t GetSizeOfBuffer(void);



typedef enum _SFS_UPDATE_PARTITION
{
  SFS_UPDATE_AUTO          = 0,   /* update partition according to partition status.*/
  SFS_UPDATE_PARTITION_0   = 1,   /* force to update partition 0.*/
  SFS_UPDATE_PARTITION_1   = 2,   /* force to update partition 1.*/
  SFS_UPDATE_BOTH          = 3,   /* force to update both partitions.*/
  SFS_UPDATE_NONE          = -1,  /* do nothing*/
} SFS_UPDATE_PARTITION;


/*
 * Download Firmware to Hudson.
 *
 * PARAMETERS
 *
 *    pBuffer             [in] - A potinter to a buffer for firmware update processing.
 *    pBootLoader         [in] - A pointer to the input buffer that contains I2C bootloader data.
 *    cbBootLoader        [in] - The size of Bootloader data in bytes.
 *    pImageData          [in] - A pointer to the input buffer that contains Image data.
 *    cbImageData         [in] - The size of Image data in bytes.
 *    slave_address       [in] - Specifies I2C chip address.
 *    DownLoadMode        [in] - Specifies the download mode, Could be the follwoing one of values.(Reno only)
 *                               0 :  auto
 *                               1 :  flash partition 0
 *                               2 :  flash partition 1
 *                               3 :  flash both partitions

 * RETURN
 *
 *    If the operation completes successfully, the return value is ERRNO_NOERR.
 *    Otherwise, return ERRON_* error code.
 *
 * REMARKS
 *
 *    You need to set up both I2cWrite and I2cWriteThenRead callback function by calling
 *    SetupI2cWriteMemCallback and SetupI2cReadMemCallback before you call this function.
 */
int DownloadFW(        void *                      pBuffer,
                       const unsigned char *       loader_data,
                       uint32_t               loader_size,
                       const unsigned char *       image_data,
                       uint32_t               image_size,
                       unsigned char               slave_address,
                       SFS_UPDATE_PARTITION        eDownLoadMode,
                       int boot_time_upgrade,
                       int legacy_device);

/*
 * time delay function.
 *
 * PARAMETERS
 *
 *    ms_delay         [in] - Specifies millisecond delay
 *
 *
 * RETURN
 *
 *    None.
 *
 */
void sys_mdelay(unsigned int ms_delay);



#ifdef __cplusplus
}
#endif


/*Error codes*/
#define ERRNO_NOERR                 0
#define ERROR_MIS_ENDIANNESS        101
#define ERRNO_WRITE_FAILED          102
#define ERRNO_INVALID_DATA          103
#define ERRNO_CHECKSUM_FAILED       104
#define ERRNO_FAILED                105
#define ERRNO_INVALID_PARAMETER     106
#define ERRNO_NOMEM                 107
#define ERRNO_I2CFUN_NOT_SET        108
#define ERRNO_UPDATE_MEMORY_FAILED  109
#define ERRNO_DEVICE_NOT_RESET      110
#define ERRNO_DEVICE_OUT_OF_CONTROL 111
#define ERRNO_UPDATE_EEPROM_FAILED  112
#define ERRNO_INVALID_BOOTLOADER    113

#define ERROR_INVALID_IMAGE         114
#define ERROR_WAITING_RESET_TIMEOUT 115
#define ERROR_LOAD_IMG_TIMEOUT      116
#define ERROR_STATE_FATAL           117
#define ERROR_CRC_CHECK_ERROR       118
#define ERROR_I2C_ERROR             119
#define ERRNO_BOOT_LOADER_NOT_FOND  120
#define ERRNO_IMAGE_NOT_FOND        121
#define ERROR_WAITING_LOADER_TIMEOUT  123
#define ERROR_NULL_POINTER          124
#define ERROR_I2C_BLOCK_NR_ERROR    125
#define ERROR_SEND_I2C_ERROR        126
#define ERRNO_NO_MORE_DATA          127
#define ERROR_UNSUPPORTED_FLASH_MEMORY 128
#define ERROR_I2CWRITE_DATA_MISALIGNMENT  201
#define ERROR_I2CWRITE_ADDR_MISALIGNEMNT  202
#define ERROR_I2CREAD_DATA_MISALIGNMENT   203
#define ERROR_I2CREAD_ADDR_MISALIGNEMNT   204




#define ERROR_FW_NO_RESPONSE        301
#define ERROR_FW_VER_INCORRECT      302
#define ERROR_FW_CANNOT_BOOT        303

/*Boot loader error code*/
#define BLERR_BLOCK_NR		-1
#define BLERR_CHECKSUM		-2
#define BLERR_LENGTH		-3
#define BLERR_TIMEOUT		-4
#define BLERR_NSUPP		-5
#define BLERR_NPROT		-6
#define BLERR_ERASE		-7
#define BLERR_WRITE		-8
#define BLERR_VERIFY		-9
#define BLERR_INVALID_CMD	-10
#define BLERR_NOTHING_TO_DO	-11
#define BLERR_INVALID_SFS_HDR	-12
#define BLERR_I2C_INTERNAL	-20

#define RESET_INTERVAL_MS    200 // 200 ms

int i2c_sub_burst_write(uint32_t startAddr, uint32_t num_byte, const unsigned char *pData);
int i2c_sub_burst_read(uint32_t startAddr, uint32_t num_byte, uint8_t *pData);
