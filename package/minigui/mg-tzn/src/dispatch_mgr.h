#ifndef	__DISPATCHER_MGR_H__
#define	__DISPATCHER_MGR_H__

#ifdef __cplusplus
extern "C" {
#endif

#define	 LCD2MCU_HEAD1	0xEE
#define  LCD2MCU_HEAD2	0xDD

#define	 MCU2LCD_HEAD1	0xAA
#define  MCU2LCD_HEAD2	0xBB

#define  LCD2MCU_REPLY	0xEE
#define  MCU2LCD_REPLY	0xAA
#define  REPLY_OK		0x06
#define  REPLY_ERR		0x15

#define CMD_BASE_DATA 				0x1001
#define CMD_POINT_CHECK_1 			0x2101
#define CMD_POINT_CHECK_2 			0x2201
#define CMD_TRY_RUN					0x2301
#define CMD_OPTION_SET				0x2401
#define CMD_ROOM_INFO				0x2501
#define CMD_ADDRESS_CHG				0x2601
#define CMD_CONFIRM_RUN				0x2701
#define CMD_ROOM_ADR_REASIGN		0x2801
#define CMD_ROOM_INIT				0x2901
#define CMD_BOARD_CHECK 			0x2A01
#define CMD_SELF_CHECK				0x2B01
#define CMD_PREHEART_OVER			0x2C01
#define CMD_IO_OUTPUT				0x2D01
#define CMD_STANDARD_TIME			0x2E01
#define CMD_DISPLAY_RESET_CNT		0x2F01


#define CMD_OFFDAY_1				0x5001
#define CMD_OFFDAY_2				0x5002
#define CMD_OFFDAY_3				0x5003

#define CMD_SCHEDULE_01				0x4010
#define CMD_SCHEDULE_02				0x4020
#define CMD_SCHEDULE_03				0x4030
#define CMD_SCHEDULE_04				0x4040
#define CMD_SCHEDULE_05				0x4050
#define CMD_SCHEDULE_06				0x4060
#define CMD_SCHEDULE_07				0x4070

#define CMD_SCHEDULE_11				0x4011
#define CMD_SCHEDULE_12				0x4012
#define CMD_SCHEDULE_13				0x4013

#define CMD_SCHEDULE_21				0x4021
#define CMD_SCHEDULE_22				0x4022
#define CMD_SCHEDULE_23				0x4023

#define CMD_SCHEDULE_31				0x4031
#define CMD_SCHEDULE_32				0x4032
#define CMD_SCHEDULE_33				0x4033

#define CMD_SCHEDULE_41				0x4041
#define CMD_SCHEDULE_42				0x4042
#define CMD_SCHEDULE_43				0x4043

#define CMD_SCHEDULE_51				0x4051
#define CMD_SCHEDULE_52				0x4052
#define CMD_SCHEDULE_53				0x4053

#define CMD_SCHEDULE_61				0x4061
#define CMD_SCHEDULE_62				0x4062
#define CMD_SCHEDULE_63				0x4063

#define CMD_SCHEDULE_71				0x4071
#define CMD_SCHEDULE_72				0x4072
#define CMD_SCHEDULE_73				0x4073

#define CMD_ERR_01					0x3001
#define CMD_ERR_02					0x3002
#define CMD_ERR_03					0x3003
#define CMD_ERR_04					0x3004
#define CMD_ERR_05					0x3005
#define CMD_ERR_06					0x3006
#define CMD_ERR_07					0x3007
#define CMD_ERR_08					0x3008
#define CMD_ERR_09					0x3009
#define CMD_ERR_0A					0x300A

int dispatcher_mgr_init(void);
int dispatcher_mgr_uninit(void);

#ifdef __cplusplus
}
#endif
#endif
