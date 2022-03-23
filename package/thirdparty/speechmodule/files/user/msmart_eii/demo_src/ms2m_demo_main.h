/*
 * Copyright (c) 2018 - 2019 M-SMART Research Institute of Midea Group.
 * All rights reserved.
 *
 * File Name            : ms2m_main.h
 * Introduction         :
 *
 * Current Version      : v1.0
 * Author               : Zachary Chau <zhouzh6@midea.com.cn>
 * Create Time          : 2018/04/18
 * Change Log           :
 *
 * All software, firmware and related documentation here in ("M-Smart Software") are
 * intellectual property of M-SMART Research Institute of Midea Group and protected
 * by law, including, but not limited to, copyright law and international treaties.
 *
 * Any use, modification, reproduction, retransmission, or republication of all
 * or part of M-Smart Software is expressly prohibited, unless prior written
 * permission has been granted by M-Smart.
 */

#ifndef __MS2M_DEMO_MAIN_H__
#define __MS2M_DEMO_MAIN_H__

#include "dlist.h"
void ms2m_main();

typedef struct {
	uint8_t roomtype;
	uint8_t devtype;
	uint8_t devID[6];
}ms2m_devinfo_t;

typedef struct {
	ms2m_devinfo_t info;
	struct list_head node;
}ms2m_devinfo_node;

typedef struct {
	uint8_t roomtype;
	uint8_t devtype;
	uint8_t data;
	uint8_t devID[6];
}ms2m_dev_state_repo;

typedef struct {
	int asr_cmd;
	struct list_head asr_ctrl_list;
	int list_len;
}ms2m_asr_ctrl_info;

#define RAC_ON_AUDIO	"/oem/AC/bedroom_rac_on.mp3"
#define GWH_ON_AUDIO	"/oem/AC/water_heater_on.mp3"
#define DWM_ON_AUDIO	"/oem/AC/washing_machine_on.mp3"
#define HUM_ON_AUDIO	"/oem/AC/humidifier_on.mp3"
#define RAC_OFF_AUDIO	"/oem/AC/bedroom_rac_off.mp3"
#define GWH_OFF_AUDIO	"/oem/AC/water_heater_off.mp3"
#define DWM_OFF_AUDIO	"/oem/AC/washing_machine_off.mp3"
#define HUM_OFF_AUDIO	"/oem/AC/humidifier_off.mp3"
#define DWM_WORK_DONE_AUDIO	"/oem/AC/washing_machine_work_done.mp3"
#define CONTROL_FAIL_AUDIO	"/oem/AC/wav172.mp3"
#define DEV_NOT_FOUND_AUDIO	"/oem/AC/dev_not_found.mp3"

enum asr_cmd_type {
	ASR_CMD_RAC_ON,
	ASR_CMD_FAN_ON,
	ASR_CMD_AP_ON,	//Air Purifier
	ASR_CMD_HUM_ON,	//Humidifier
	ASR_CMD_SR_ON,	//Sweeping Robot
	ASR_CMD_LAMP_ON,	//Lamp
	ASR_CMD_PWM_ON,	//Pulsator washing machine
	ASR_CMD_DWM_ON,	//Drum washing machine
	ASR_CMD_CWM_ON,	//Compound washing machine
	ASR_CMD_AWH_ON,	//Air energy water heater
	ASR_CMD_EWH_ON,	//Electric water heater
	ASR_CMD_GWH_ON,	//Gas water heater
	ASR_CMD_RAC_OFF,
	ASR_CMD_FAN_OFF,
	ASR_CMD_AP_OFF,	//Air Purifier
	ASR_CMD_HUM_OFF,	//Humidifier
	ASR_CMD_SR_OFF,	//Sweeping Robot
	ASR_CMD_LAMP_OFF,	//Lamp
	ASR_CMD_PWM_OFF,	//Pulsator washing machine
	ASR_CMD_DWM_OFF,	//Drum washing machine
	ASR_CMD_CWM_OFF,	//Compound washing machine
	ASR_CMD_AWH_OFF,	//Air energy water heater
	ASR_CMD_EWH_OFF,	//Electric water heater
	ASR_CMD_GWH_OFF,	//Gas water heater
	ASR_CMD_BACK,
	ASR_CMD_LEAVE,
	ASR_CMD_NULL
};

enum ms2m_sence {
	LEAVE,
	BACK
};

enum ms2m_ctrl_data {
	OFF = 0x0,
	ON
};

enum ms2m_room_type {
	ROOM_TYPE_PARLOUR = 0x0,
	ROOM_TYPE_BEDROOM,
	ROOM_TYPE_BATHROOM,
	ROOM_TYPE_BALCONY,
	ROOM_TYPE_KITCHEN,
	ROOM_TYPE_NULL = 0xFE,
	ROOM_TYPE_ALL = 0xFF
};

enum ms2m_cmd_type {
	CMD_POWER_CTRL = 0x0,
	CMD_FUNC_CTRL,
	CMD_ROBOT_CTRL,
	CMD_WEATHER_QUERY,
	CMD_SCENE_CTRL,
	CMD_DEV_INFO_LIST = 0xA0,
	CMD_DEV_CTRL_SUCCESS = 0xA5,
	CMD_DEV_STATE_REPO = 0xAA
};

enum ms2m_dev_type {
	DEV_TYPE_WM = 0x0,	//washing machine
	DEV_TYPE_WH = 0x1,	//water heater
	DEV_TYPE_RAC = 0xAC,
	DEV_TYPE_FAN = 0xFA,
	DEV_TYPE_AP = 0xFC,	//Air Purifier
	DEV_TYPE_HUM = 0xFD,	//Humidifier
	DEV_TYPE_SR = 0xB8,	//Sweeping Robot
	DEV_TYPE_LAMP = 0x13,	//Lamp
	DEV_TYPE_PWM = 0xDA,	//Pulsator washing machine
	DEV_TYPE_DWM = 0xDB,	//Drum washing machine
	DEV_TYPE_CWM = 0xD9,	//Compound washing machine
	DEV_TYPE_AWH = 0xCD,	//Air energy water heater
	DEV_TYPE_EWH = 0xE2,	//Electric water heater
	DEV_TYPE_GWH = 0xE3,	//Gas water heater

	DEV_TYPE_NULL = 0xFE,
	DEV_TYPE_ALL = 0xFF
};

#endif /* __MS2M_DEMO_MAIN__ */
