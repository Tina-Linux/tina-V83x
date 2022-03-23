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

#include "rtwpriv.h"
#include "rtw_api.h"

int main(int argc, char *argv[]) {

	PRT_PMAC_TX_INFO pPMacTxInfo;
	RT_PMAC_TX_INFO	MyPMacTxInfo;
	u8 UnicastDID[6] = {0xff,0xff,0xff,0xff,0xff,0xff};
	u16 CurrentAntenna;
	u8 PktPattern = 0x00;
	char *ifname, *incmd;
	int inRate = 11, inBW = 0 ,inCH = 1 ,txmode = 1, psd_len = 256, pktlen = 1500, pktcount = 0, pktPeriod = 100;
	//const char* rtw_pmact_version[] = {"v2.0"};
	int skfd, err = 0;
	char input[BUF_SIZE];
	char input_arg[BUF_SIZE];
	char copy_input[BUF_SIZE];
	char setmpchar[]={"mp_hxtx "};
	char setmp_rate[32] = {"mp_rate"};
	char setmp_ch[32] = {"mp_channel"};
	char setmp_bw[32] = {"mp_bandwidth 40M="};
	char setmp_ant[32] = {"mp_ant_tx "};
	char cmdbuf[64];

	UCHAR tmpU1B, Idx;
	int i = 0;
	pPMacTxInfo = &MyPMacTxInfo;
	pPMacTxInfo->bEnPMacTx = TRUE;

	//Chack input parameter , wlan0 CH BW  TxMode rateID
	if (argc < 6) {
			if (argc > 2) {
				if (strncmp(argv[2], "stop", 4) == 0) {
					DBG("#####Stop HW TX mode!!!#####\n");
					pPMacTxInfo->bEnPMacTx = FALSE;
				} else if (strncmp(argv[2], "ver", 3) == 0) {
					printf("rtwpriv version:%s\n",RTWPRIVE_VERSION);
					return err;
				} else if ((strncmp(argv[2], "?", 1) == 0) || (strncmp(argv[2], "help", 4) == 0)) {
					printf("rtwpriv version:%s\n",RTWPRIVE_VERSION);
					rtw_help();
					return err;
				} else if (strncmp(argv[2], "mp_psd_analysis", 15) == 0) {
						ifname = argv[1];
						incmd = argv[2];
						if(argv[3]!=NULL)
							psd_len = atoi(argv[3]);
						psd_analysis(ifname, incmd, psd_len);
						return err;
				} else {
					if (argc < 3)	{
						printf("rtwpriv version:%s\n",RTWPRIVE_VERSION);
						printf("ioctl no enough parameters!\n");
						return -EINVAL;
					}
					ifname = argv[1];
					sprintf(input, "%s", argv[2]);

					for (i=3; i<argc; i++) {
						sprintf(input_arg, "%s %s", input, argv[i]);
						strcpy(input, input_arg);
					}

					//DBG("ifname = %s\n", ifname);
					//DBG("input = %s\n", input);
					/* decision for use old iwreq to send*/
					skfd = iw_sockets_open();
					err = wlan_ioctl_mp(skfd, ifname, input, strlen(input)+1);
					iw_sockets_close(skfd);

					if (err < 0) {
						fprintf(stderr, "Interface doesn't accept private ioctl...\n");
						fprintf(stderr, "%s: %s\n", argv[2], strerror(errno));
					} else {
						if (strlen(input) != 0)
							printf("%-8.16s %s:%s\n", ifname, argv[2], input);
					}
					return err;

				}
			} else {
						if (argc > 1) {
							if ((strncmp(argv[1], "-v", 2) == 0) || (strncmp(argv[1], "v", 1) == 0))
									printf("rtwpriv version:%s\n",RTWPRIVE_VERSION);
							else{
									fprintf(stderr, "Interface doesn't accept private ioctl...parameter Error\n");
									fprintf(stderr, "%s: %s\n", argv[2], strerror(errno));
									return -EINVAL;
								}
						} else
							rtw_help();
						return err;
			}
	} else {
		if (argc > 2)
		inCH  = atoi(argv[2]);
		if (inCH > 177 || inCH < 0) {
			//rtw_help();
			fprintf(stderr, "Interface doesn't accept private ioctl...parameter Error, channel \n");
			return -EINVAL;
		}
		if (argc > 3)
			inBW  = atoi(argv[3]);
		if (inBW > 3 || inBW <0) {
			rtw_help();
			fprintf(stderr, "Interface doesn't accept private ioctl...parameter Error, bandwidth\n");
			return -EINVAL;
		}
		if (argc > 4)
			CurrentAntenna = rtw_ant_strpars(argv[4]);
		if (argc > 5)
			inRate = rtw_mpRateParseFunc(argv[5]); //parsing string for get Rate ID index.
		if (argc > 6)
			txmode = atoi(argv[6]);
		else {
			fprintf(stderr, "Interface doesn't accept private ioctl...parameter Error\n");
			return -EINVAL;
		}


		if (txmode >= 4 || txmode <= 0) {
			rtw_help();
			fprintf(stderr, "Interface doesn't accept private ioctl...parameter Error\n");
			return -EINVAL;
		}

		if (argc > 7) {
			pktPeriod = atoi(argv[7]);
			printf("config packet Period =%d\n", pktPeriod);
		}

		if (argc > 8) {
			pktlen = atoi(argv[8]);
			printf("config packet length =%d\n", pktlen);
			if (pktlen <= 100) {
					fprintf(stderr, "Interface doesn't accept private ioctl... pktlen <= 100 parameter Error\n");
					return -EINVAL;
			}

		}
		if (argc > 9) {
			pktcount = atoi(argv[9]);
			printf("config packet count =%d\n", pktcount);
		}

		if (argc > 10) {
			int i = atoi(argv[10]);
			if(i <= 255) {
				PktPattern  = strtol(argv[10], NULL, 16);
			} else {
				rtw_help();
				fprintf(stderr, "Interface doesn't accept private ioctl...parameter Error\n");
				return -EINVAL;
		    }
		}

	}
	ifname = argv[1]; // Wlan interface

	// Parsing Config File for get setting.
	if (Read_Parsing_file(&MyPMacTxInfo, &CurrentAntenna, UnicastDID) != 1) {
		u16 umac[]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

		DBG("No rtw_Config.txt File, Use Default Setting for PMACT \n");
		pPMacTxInfo->TX_SC = 0;
		pPMacTxInfo->bSTBC = FALSE;
		pPMacTxInfo->bLDPC = FALSE;
		pPMacTxInfo->bSPreamble = FALSE;
		pPMacTxInfo->bSGI = FALSE;
		// Packet NDP
		pPMacTxInfo->NDP_sound = FALSE;
		// Packet Period
		pPMacTxInfo->PacketPeriod = pktPeriod;
		// Packet Pattern
		if (PktPattern != 0x00)
			pPMacTxInfo->PacketPattern = PktPattern;
		else
			pPMacTxInfo->PacketPattern = randomByte();
		DBG("PacketPattern = %x\n", pPMacTxInfo->PacketPattern);
		// Packet Count
		pPMacTxInfo->PacketCount = pktcount;
		// Packet Length
		pPMacTxInfo->PacketLength = pktlen;
		// Mac Address
		memcpy(pPMacTxInfo->MacAddress, umac, 6);
	}


	for (i = 0; i < argc; i++) {
        DBG("argv[%d] = %s\n", i, argv[i]);
    }
    if (pPMacTxInfo->bEnPMacTx != FALSE) {

	pPMacTxInfo->TX_RATE = HwRateToMPTRate(inRate);;

	if (inBW > 3 || inBW < 0)
		pPMacTxInfo->BandWidth = 0;
	else
		pPMacTxInfo->BandWidth = inBW;

	if (inCH > 168 || inCH < 0)
			inCH = 36;

	if (txmode == 1)
		pPMacTxInfo->Mode = PACKETS_TX;
	else if  (txmode == 2)
		pPMacTxInfo->Mode = CONTINUOUS_TX;
	else if  (txmode == 3)
		pPMacTxInfo->Mode = OFDM_Single_Tone_TX;
	else
		pPMacTxInfo->Mode = PACKETS_TX;

	/* Ioctrl	Set Rate */
	sprintf( cmdbuf, "%s %d", setmp_rate ,inRate);
	DBG("cmdbuf= %s\n", cmdbuf);
	skfd = iw_sockets_open();
	err = wlan_ioctl_mp(skfd, ifname, cmdbuf, strlen(cmdbuf)+1);  //ioctrl for set mp_rate
	iw_sockets_close(skfd);
	if (err < 0) {
		fprintf(stderr, "Interface doesn't accept private ioctl...\n");
		fprintf(stderr, "%s: %s\n", ifname, strerror(errno));
		return -1;
	}
	/* Ioctrl	Set Antenna */
	sprintf( cmdbuf, "%s%s", setmp_ant, argv[4]);
	DBG("cmdbuf= %s\n", cmdbuf);
	skfd = iw_sockets_open();
	err = wlan_ioctl_mp(skfd, ifname, cmdbuf, strlen(cmdbuf)+1);  //ioctrl for set mp_ant_tx
	iw_sockets_close(skfd);
	if (err < 0) {
		fprintf(stderr, "Interface doesn't accept private ioctl...\n");
		fprintf(stderr, "%s: %s\n", ifname, strerror(errno));
		return -1;
	}
	/* Ioctrl Set Bandwidth */
	memset(cmdbuf, 0, sizeof(cmdbuf));
	sprintf( cmdbuf, "%s%d", setmp_bw ,inBW);
	DBG("cmdbuf= %s %d\n", cmdbuf, strlen(cmdbuf));

	skfd = iw_sockets_open();
	err = wlan_ioctl_mp(skfd, ifname, cmdbuf, strlen(cmdbuf)+1); //ioctrl for set mp_bandwidth
	iw_sockets_close(skfd);

	if (err < 0) {
		fprintf(stderr, "Interface doesn't accept private ioctl...\n");
		fprintf(stderr, "%s: %s\n", ifname, strerror(errno));
		return -1;
	}
	/* Ioctrl Set CHannel */
	memset(cmdbuf, 0, sizeof(cmdbuf));
	sprintf( cmdbuf, "%s %d", setmp_ch ,inCH);
	DBG("cmdbuf= %s %d\n", cmdbuf, strlen(cmdbuf));

	skfd = iw_sockets_open();
	err = wlan_ioctl_mp(skfd, ifname, cmdbuf, strlen(cmdbuf)+1);  //ioctrl for set mp_channel
	iw_sockets_close(skfd);

	if (err < 0) {
		fprintf(stderr, "Interface doesn't accept private ioctl...\n");
		fprintf(stderr, "%s: %s\n", ifname, strerror(errno));
		return -1;
	}


	memcpy(pPMacTxInfo->MacAddress, UnicastDID, 6);

	pPMacTxInfo->Ntx = 0;
	for(Idx = 16; Idx < 20; Idx++)
	{
		tmpU1B = (CurrentAntenna >> Idx) & 1;
		if(tmpU1B)
			pPMacTxInfo->Ntx++;
	}
	DBG("pPMacTxInfo->Ntx =%d\n",pPMacTxInfo->Ntx);
	memcpy(input,  &MyPMacTxInfo, sizeof(RT_PMAC_TX_INFO));
	//dump_buf(input, sizeof(RT_PMAC_TX_INFO));

	PMAC_Notify(pPMacTxInfo);

	DBG("SGI %d bSPreamble %d bSTBC %d bLDPC %d NDP_sound %d\n", pPMacTxInfo->bSGI, pPMacTxInfo->bSPreamble, pPMacTxInfo->bSTBC, pPMacTxInfo->bLDPC, pPMacTxInfo->NDP_sound);
	DBG("TXSC %d BandWidth %d PacketPeriod %d PacketCount %u PacketLength %u PacketPattern %d\n", pPMacTxInfo->TX_SC, pPMacTxInfo->BandWidth, pPMacTxInfo->PacketPeriod, pPMacTxInfo->PacketCount,pPMacTxInfo->PacketLength, pPMacTxInfo->PacketPattern);
#ifdef RTWDEBUG
	PRINT_DATA((char *)"LSIG ", pPMacTxInfo->LSIG, 3);
	PRINT_DATA((char *)"HT_SIG", pPMacTxInfo->HT_SIG, 6);
	PRINT_DATA((char *)"VHT_SIG_A", pPMacTxInfo->VHT_SIG_A, 6);
	PRINT_DATA((char *)"VHT_SIG_B", pPMacTxInfo->VHT_SIG_B, 4);
	DBG("VHT_SIG_B_CRC %x\n", pPMacTxInfo->VHT_SIG_B_CRC);
	PRINT_DATA((char *)"VHT_Delimiter", pPMacTxInfo->VHT_Delimiter, 4);
	//PRINT_DATA("Src Address", Adapter->mac_addr, 6);
	PRINT_DATA((char *)"Dest Address", pPMacTxInfo->MacAddress, 6);
#endif
        }
	memcpy(copy_input,  setmpchar, strlen(setmpchar));
	memcpy(copy_input + strlen(setmpchar), (void *)&MyPMacTxInfo, sizeof(RT_PMAC_TX_INFO));

	/* check the driver for new feature support */
	skfd = iw_sockets_open();
	err = wlan_ioctl_mp(skfd, ifname, copy_input, sizeof(RT_PMAC_TX_INFO) + 9);
	iw_sockets_close(skfd);

	if (err < 0) {
		fprintf(stderr, "Interface doesn't accept private ioctl...\n");
		fprintf(stderr, "%s: %s\n", argv[2], strerror(errno));
	} else {
		if (strlen(copy_input) != 0)
			printf("%-8.16s %s:%s\n", ifname, argv[2], copy_input);
	}
	return err;
}
