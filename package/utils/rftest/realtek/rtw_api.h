/******************************************************************************
 *
 * Copyright(c) 2007 - 2018 Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 *****************************************************************************/

int iw_sockets_open(void);
void iw_sockets_close(int skfd);
int wlan_ioctl_mp(
		int skfd,
		char *ifname,
		void *pBuffer,
		unsigned int BufferSize);
u8 key_char2num(u8 ch);
u8 key_2char2num(u8 hch, u8 lch);
void macstr2num(u8 *dst, u8 *src);
void dump_buf(char *buf, int len);
int randInRange( int min, int max );
UCHAR randomByte();
void split(char **arr, char *str, char *del);
int wifirate2_ratetbl_inx(int rate);
u8 HwRateToMPTRate(u8 rate);
int rtw_mpRateParseFunc(char *targetStr);
u16 rtw_ant_strpars(char *targetStr);

int Read_Parsing_file(PRT_PMAC_TX_INFO pPMacTxInfo2, u16 *Antenna, u8 *UnicastDID);
int psd_analysis(char *ifname, char *psdcmd, int psdlen);

void rtw_help ();
void
PMAC_Notify(
	PRT_PMAC_TX_INFO pPMacTxInfo
	);
void
PMAC_Leave(
	PRT_PMAC_TX_INFO pPMacTxInfo
	);
	void
PMAC_Enter(
	PRT_PMAC_TX_INFO	pPMacTxInfo
	);
void
PMAC_Get_Pkt_Param(
	PRT_PMAC_TX_INFO	pPMacTxInfo,
	PRT_PMAC_PKT_INFO	pPMacPktInfo
	);
void
PMAC_Nsym_generator(
	PRT_PMAC_TX_INFO	pPMacTxInfo,
	PRT_PMAC_PKT_INFO	pPMacPktInfo
	);
UINT
LDPC_parameter_generator(
	UINT N_pld_int,				// number of initial information bits
	UINT N_CBPSS,				// number of coded bits per OFDM symbol per stream
	UINT N_SS,					// number of spatial-stream
	UINT R,						// coding rate
	UINT m_STBC,				// STBC indicator
	UINT N_TCB_int				// number of total coded bits
);
UINT
LDPC_parameter_generator(
	UINT N_pld_int,				// number of initial information bits
	UINT N_CBPSS,				// number of coded bits per OFDM symbol per stream
	UINT N_SS,					// number of spatial-stream
	UINT R,						// coding rate
	UINT m_STBC,				// STBC indicator
	UINT N_TCB_int				// number of total coded bits
);

void
VHT_SIG_B_generator(
	PRT_PMAC_TX_INFO	pPMacTxInfo
	);

void
VHT_SIG_A_generator(
	PRT_PMAC_TX_INFO	pPMacTxInfo,
	PRT_PMAC_PKT_INFO	pPMacPktInfo
);

void
HT_SIG_generator(
	PRT_PMAC_TX_INFO	pPMacTxInfo,
	PRT_PMAC_PKT_INFO	pPMacPktInfo
	);

void
L_SIG_generator(
	UINT	N_SYM,		// Max: 750
	PRT_PMAC_TX_INFO	pPMacTxInfo,
	PRT_PMAC_PKT_INFO	pPMacPktInfo
	);

void
CCK_generator(
	PRT_PMAC_TX_INFO	pPMacTxInfo,
	PRT_PMAC_PKT_INFO	pPMacPktInfo
	);

void
CRC16_generator(
	BOOL *out,				// crc16 output
	BOOL *in,				// binary input
	UCHAR in_size			// size of binary input signal
);

void
CRC8_generator(
	BOOL *out,				// crc8 output
	BOOL *in,				// binary input
	UCHAR in_size			// size of binary input signal
);
