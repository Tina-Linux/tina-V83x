/**
 * @file msm_adapter.c
 * @brief
 * @author Humble
 * @version 1.0.0
 * @par Copyright  (c)
 *		Midea
 */
#include "ms_common.h"
#include "ms_sn.h"

#define MS_AC_SN_LENGTH			29
#define MS_DEV_SN_LENGTH	32
#define MS_AC_CODE			0XAC
#define AC_CODE_INDEX	1

static const int8_t Decryption_Table_one[64] = //0911
{
	39,7,47,15,55,23,63,31,
	38,6,46,14,54,22,62,30,
	37,5,45,13,53,21,61,29,
	36,4,44,12,52,20,60,28,
	35,3,43,11,51,19,59,27,
	34,2,42,10,50,18,58,26,
	33,1,41, 9,49,17,57,25,
	32,0,40, 8,48,16,56,24
};

static const int8_t Decryption_Table_two[64] =
{
	49,43,63, 4,47, 0,13,28,
	35,42,30,44,45,48,22,33,
	5 ,10,37,17, 7,46, 8,60,
	50,31,26,61,16,24,62,36,
	55, 2,25,11, 1,41,12, 3,
	19,59,23,58,39,40,34,56,
	18,29,38,15,51, 6,32,57,
	54,53,21, 9,27,52,14,20
};

static const int8_t Taxis_Converse_Array[10][20] =
{
	{
		7 ,18,19,12, 1,
		14, 2, 8, 3, 9,
		4 ,16, 5,11,10,
		17,13, 6, 0,15
    },
    {
		2 ,16,8 ,11,10,
		0 ,7 ,17,19,14,
		4 ,5 ,15,9 ,12,
		13,18,1 ,6 ,3
    },
    {
		4 ,0 ,11,1 ,6 ,
		17,5 ,3 ,9 ,14,
		2 ,15,8 ,19,18,
		10,13,12,7 ,16
    },
    {
		19,2 ,6 ,16,8 ,
		9 ,10,0 ,14,11,
		17,12,18,7 ,13,
		15,3 ,1 ,5 ,4
    },
    {
		6 ,17,7 ,10,14,
		13,12,3 ,2 ,5 ,
		0 ,8 ,18,9 ,16,
		19,1 ,11,4 ,15
    },
    {
		16,13,9 ,5 ,1 ,
		0 ,2 ,14,4 ,8 ,
		17,18,3 ,15,6 ,
		12,19,7 ,11,10
    },
    {
		5 ,3 ,15,2 ,6 ,
		14,11,8 ,10,13,
		0 ,19,18,16,9 ,
		4 ,7 ,17,1 ,12
    },
    {
		16,9 ,4 ,15,14,
		5 ,3 ,17,13,8 ,
		0 ,12,19,11,6 ,
		10,2 ,7 ,18,1
    },
    {
		16,17,10,5 ,8 ,
		12,7 ,1 ,6 ,14,
		18,2 ,0 ,9 ,3 ,
		15,13,11,4 ,19
    },
    {
		7 ,9 ,3 ,4 ,18,
		16,19,5 ,11,15,
		14,10,17,1 ,13,
		2 ,6 ,12,0 ,8
    }
};

void ms_fw_air_sn_decoding(uint8_t *pucBarcode, uint8_t *pucID)
{
	uint8_t	ucTemp,ucTemp1;
	uint8_t	ucTempArray[22];

	ucTemp1 = *(pucID + 21);
	if(ucTemp1>9)
	{
		ucTemp1 = 9;
	}

	for(ucTemp = 0; ucTemp < 22; ucTemp++)
	{
		ucTempArray[ucTemp] = 0x00;
		*(pucBarcode + ucTemp) = 0x00;
	}

	for(ucTemp = 1; ucTemp < 21; ucTemp++)
	{
		ucTempArray[ucTemp-1] = Decryption_Table_one[*(pucID + ucTemp)];
	}

	for(ucTemp = 0; ucTemp < 20; ucTemp++)
	{
		*(pucBarcode + Taxis_Converse_Array[ucTemp1][ucTemp]) = ucTempArray[ucTemp];
	}

	for(ucTemp = 0; ucTemp < 20; ucTemp++)
	{
		ucTempArray[ucTemp] = Decryption_Table_two[*(pucBarcode + ucTemp)];
	}

	for(ucTemp = 0; ucTemp < 20; ucTemp++)
	{
		if(ucTempArray[ucTemp]<10)
		{
			*(pucBarcode + ucTemp + 1) = ucTempArray[ucTemp] + 48;
		}
		else
		{
			*(pucBarcode + ucTemp + 1) = ucTempArray[ucTemp] + 55;
		}
	}

	ucTemp =  *(pucID + 0) - 36;
	if(ucTemp<10)
	{
		*(pucBarcode + 0) = ucTemp + 48;
	}
	else
	{
		*(pucBarcode + 0) = ucTemp + 55;
	}

	ucTemp =  *(pucID + 21);
	if(ucTemp<10)
	{
		*(pucBarcode + 21) = ucTemp + 48;
	}
	else
	{
		*(pucBarcode + 21) = ucTemp + 55;
	}
}

ms_result_t ms_check_sn_is_valid(uint8_t *str , uint8_t len)
{
	uint8_t i;

	if(len != MS_DEV_SN_LENGTH)
	{
		MS_TRACE("The received SN length is %d",len);
		return MS_RESULT_FAIL;
	}

	for(i = 0; i < MS_DEV_SN_LENGTH; i++)
	{
		if(((str[i] >= '0' && str[i] <= '9') || (str[i] >= 'A' && str[i] <= 'Z'))
			&& (str[i] != 'I') && (str[i] != 'O'))
		{
			continue;
		}

		return MS_RESULT_FAIL;
	}

	return MS_RESULT_SUCCESS;
}

ms_result_t ms_check_is_air_condition_sn(uint8_t *str , uint8_t *airSN)
{
	uint8_t i = 0;
	ms_result_t status = MS_RESULT_FAIL;

	for(i = 0; i < 22; i++)
	{
		if(0x00 != str[i])
			break;
	}

	if(22 == i)
	{
		MS_ERR_TRACE("got ac sn error");
		return MS_RESULT_FAIL;
	}

	memset(airSN, '0', 32);
	ms_fw_air_sn_decoding(&airSN[6], str);
	status = ms_check_sn_is_valid(airSN, 32);
	if(status == MS_RESULT_SUCCESS)
	{
		for(i = 0; i < 6; i++)
		{
			if(airSN[i] != '0')
			{
				return MS_RESULT_FAIL;
			}
		}

		for(i = 28; i < 32;i++)
		{
			if(airSN[i] != '0')
			{
				return MS_RESULT_FAIL;
			}
		}
	}
	else
	{
		return MS_RESULT_FAIL;
	}
	return MS_RESULT_SUCCESS;
}

/**
* @Function	Decrypt sn
* @Parameter	p_sn_in:		The 0x07/0x65 uart msg body
*			inlen:		The lenght of 0x07/0x65 uart msg body
*			p_sn_out:	The decrypted sn
*			poutlen:		The length of decrypted sn, the length is always 32
* @return	MS_RESULT_SUCCESS:	Decrypt successfully
*			MS_RESULT_FAIL  :	Decrypt failed
**/
ms_result_t ms_sn_decrypt(uint8_t *p_sn_in, int inlen, uint8_t *p_sn_out, int *poutlen)
{
	if((MS_AC_SN_LENGTH == inlen && MS_AC_CODE == p_sn_in[AC_CODE_INDEX]) || MS_DEV_SN_LENGTH == inlen)
	{
		if(MS_DEV_SN_LENGTH == inlen)
		{
			if(MS_RESULT_SUCCESS == ms_check_sn_is_valid(p_sn_in, inlen))
			{
				memcpy(p_sn_out, p_sn_in, MS_DEV_SN_LENGTH);
				*poutlen = MS_DEV_SN_LENGTH;
				return MS_RESULT_SUCCESS;
			}
			else
			{
				return MS_RESULT_FAIL;
			}
		}
		else if(MS_AC_SN_LENGTH == inlen && MS_AC_CODE == p_sn_in[AC_CODE_INDEX])
		{
			//ms_check_is_air_condition_sn(p_sn_in + 5, p_sn_out);
			if(MS_RESULT_SUCCESS == ms_check_is_air_condition_sn(p_sn_in + 5, p_sn_out))
			{
				*poutlen = MS_DEV_SN_LENGTH;
				return MS_RESULT_SUCCESS;
			}
			else
			{
				return MS_RESULT_FAIL;
			}
		}
		else
		{
			return MS_RESULT_FAIL;
		}
	}
	else
	{
		MS_ERR_TRACE("sn len/content invalid");

		return MS_RESULT_FAIL;
	}
}
