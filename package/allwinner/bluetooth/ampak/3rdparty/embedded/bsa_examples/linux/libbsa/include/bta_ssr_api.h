/*****************************************************************************
**
**  Name:           bta_ssr_api.h
**
**  Description:    This is the header file of the API for sensor interface
**					between host application and BRCM2075 chipset.
**
**  Copyright (c) 2008, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/

#ifndef BTA_SSR_API_H
#define BTA_SSR_API_H

#include "bta_api.h"

#ifndef BTA_SSR_INCLUDED
#define BTA_SSR_INCLUDED       TRUE
#endif

/* Extra Debug Code */
#ifndef BTA_SSR_DEBUG
#define BTA_SSR_DEBUG           FALSE
#endif

enum
{
	BTA_SSR_OK,
	BTA_SSR_ERR,
    BTA_SSR_ERR_INVALID_OP,
    BTA_SSR_ERR_BAD_PARAM,
    BTA_SSR_ERR_NO_RESOURCE
};
typedef UINT8 tBTA_SSR_STATUS;

enum
{
	BTA_SSR_OPEN_EVT,
	BTA_SSR_CLOSE_EVT,
	BTA_SSR_RESET_EVT,
	BTA_SSR_GENERIC_MSG_EVT,
	BTA_SSR_DATA_READ_EVT,
	BTA_SSR_MODE_SET_EVT,
	BTA_SSR_WRITE_CFG_EVT,
	BTA_SSR_READ_CFG_EVT,
	BTA_SSR_ENABLE_EVT,
	BTA_SSR_DISABLE_EVT,
	BTA_SSR_DEV_LIST_EVT
};
typedef UINT8 tBTA_SSR_EVT;

/* sensor data operation mode */
#define BTA_SSR_OP_MODE_NORM		0		/* normal mode */
#define BTA_SSR_OP_MODE_AUTO		1		/* automaitc mode */
typedef UINT8 tBTA_SSR_OP_MODE;

/* sensor device operation mode */
#define BTA_SSR_DEV_MODE_NORM		0		/* normal mode */
#define BTA_SSR_DEV_MODE_SAVING		1		/* power saving mode */
#define BTA_SSR_DEV_MODE_SLEEP		2		/* Sleep mode */
#define BTA_SSR_DEV_MODE_STOP		3		/* stop mode */
typedef UINT8 tBTA_SSR_DEV_MODE;

#define BTA_SSR_GEN_MSG_READ        0x01
#define BTA_SSR_GEN_MSG_WRITE       0x03
typedef UINT8 tBTA_SSR_GEN_MSG_TYPE;

#define BTA_SSR_DEV_ID_INVALID      0xff

typedef struct
{
    UINT8       hh;
    UINT8       mm;
    UINT8       ss;
    UINT8       ms;
}tBTA_SSR_TIME;

typedef struct
{
    tBTA_SSR_STATUS     status;
    tBTA_SSR_TIME       time_stamp;
    UINT8               len;
    UINT8               *p_data;
}tBTA_SSR_GEN_DATA;

typedef struct
{
    UINT32              upd_time;
    UINT16              mode_time;
    tBTA_SSR_STATUS     status;
    tBTA_SSR_OP_MODE    op_mode;
    tBTA_SSR_DEV_MODE   dev_mode;
    UINT8               sample;
    UINT8               cfg_data_len;
    UINT8               *p_cfg_data;
}tBTA_SSR_READ_CFG;

typedef union
{
    tBTA_SSR_READ_CFG   cfg_data;
    tBTA_SSR_GEN_DATA   read;
    tBTA_SSR_GEN_DATA   generic;
	tBTA_SSR_STATUS		status;
}tBTA_SSR;

/* CG callback */
typedef void (tBTA_SSR_CBACK)(tBTA_SSR_EVT event, UINT16 sensor_id, tBTA_SSR *p_data);


/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif


/*******************************************************************************
**
** Function         BTA_SsrEnable
**
** Description      Enable Sensor fuctionality.
**
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_SsrEnable(tBTA_SSR_CBACK *p_cback, UINT8 app_id);

/*******************************************************************************
**
** Function         BTA_SsrDisable
**
** Description      Disable SSR functionality.
**
** Parameters       none
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_SsrDisable(void);
/*******************************************************************************
**
** Function         BTA_SsrGetDeviceList
**
** Description      Get the list of the available sensor device.
**
** Parameters       none
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_SsrGetDeviceList(void);

/*******************************************************************************
**
** Function         BTA_SsrOpen
**
** Description      Open a connection to a sensor device.
**
** Parameters       sensor_id: Sensor ID.
**					mode: sensor data operation mode.
**					upd_time: Measurement update time, used only at autonoumous mode.
**							unit in ms, recommended minimum update time to be several
**							hundreds of ms.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_SsrOpen(UINT16 sensor_id, tBTA_SSR_OP_MODE mode, UINT32	upd_time);
/*******************************************************************************
**
** Function         BTA_SsrClose
**
** Description      Close a connection to a sensor device.
**
** Parameters       sensor_id: Sensor ID.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_SsrClose(UINT16 sensor_id);
/*******************************************************************************
**
** Function         BTA_SsrReset
**
** Description      To soft reset a sensor device.
**
** Parameters       sensor_id: Sensor ID.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_SsrReset(UINT16 sensor_id);

/*******************************************************************************
**
** Function         BTA_SsrReadData
**
** Description      Read Sensor data.
**
** Parameters       sensor_id: Sensor ID.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_SsrReadData(UINT16 sensor_id, UINT8 read_size);
/*******************************************************************************
**
** Function         BTA_SsrSetMode
**
** Description      Set a sedvice mode of a sensor to be normal, power saving mode,
**					sleep mode or stop mode.
**
** Parameters       sensor_id: Sensor ID.
**					dev_mode: device mode to set to.
**					mode_time: value in the unit of 10ms.
**								used in power saving mode: it is the sleep period
**								between wakeups to read measurement.
**								in sleep mode: it is the sleep time, 0 means sleep
**								forever. It's ignored in other modes.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_SsrSetDevMode(UINT16 sensor_id, tBTA_SSR_DEV_MODE dev_mode,
                                      UINT8 samples, UINT16 mode_time);
/*******************************************************************************
**
** Function         BTA_SsrWriteConfig
**
** Description      Write control parameters for a sensor.
**
** Parameters       sensor_id: Sensor ID.
**					len: length of the configuration data.
**					p_cfg_data: configuration data to be written.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_SsrWriteConfig(UINT16 sensor_id, UINT8 op_mode, UINT32 upd_time,
                        UINT8 len, UINT8 *p_cfg_data);
/*******************************************************************************
**
** Function         BTA_SsrReadConfig
**
** Description      Read control parameters for a sensor.
**
** Parameters       sensor_id: Sensor ID.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_SsrReadConfig(UINT16 sensor_id, UINT8 read_len);
/*******************************************************************************
**
** Function         BTA_SsrGenericCmd
**
** Description      Send a generic command to a sensor device. It's a virtual pipe
**					host to sensor device.
**
** Parameters       sensor_id: Sensor ID.
**					len: length of the generic data.
**					p_data: generic data bytes.
**
** Returns          void
**
*******************************************************************************/
BTA_API extern void BTA_SsrGenericCmd(UINT16 sensor_id, tBTA_SSR_GEN_MSG_TYPE msg_type,
                       UINT8 read_size, UINT8 write_size, UINT8 *p_data);

#ifdef __cplusplus
}
#endif

#endif/* BTA_SSR_API_H */
