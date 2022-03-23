#include "TParseConfig.h"

#define KEY_LENGTH	256

static void getValue(char *line, char *value)
{
	char * ptemp = line;

	while(*ptemp && (*ptemp++ != '='));

	char *pval = ptemp;
	char *seps = " \n\r\t";
	int offset = 0;
	pval = strtok(pval, seps);
	while (pval != NULL){
		strncpy(value + offset, pval, strlen(pval));
		offset += strlen(pval);
		pval = strtok(NULL, seps);
	}
	*(value + offset) = 0;
}

static void setValue(char *line, char *value)
{
	char * ptemp = line;

	while(*ptemp && (*ptemp++ != '='));

    while(*ptemp && (*ptemp == ' ')){
        ptemp++;
    }

	char *pval = ptemp;
	int lookupLen = strlen(pval);
    int replaceLen = strlen(value);

    memcpy(pval, value, replaceLen);

    if(replaceLen < lookupLen){
        int moreLen = lookupLen - replaceLen;
        while(moreLen){
            pval[replaceLen++] = ' ';
            moreLen--;
        }
        pval[replaceLen] = '\0';
    }
}

int readKey(int cameraId, char *key, char *value)
{
	int bRet = 0;
	int bFlagBegin = 0;
	char strId[2];
	char str[KEY_LENGTH];
	FILE * mhKeyFile;

	if (key == 0 || value == 0){
		printf("error input para");
		return 0;
	}

	mhKeyFile = fopen(CAMERA_KEY_CONFIG_PATH, "rb");
	if (mhKeyFile == 0){
		printf("error key file handle");
		return 0;
	}

	fseek(mhKeyFile, 0L, SEEK_SET);

	memset(str, 0, KEY_LENGTH);
	while (fgets(str, KEY_LENGTH , mhKeyFile)){
		if (!bFlagBegin){
			if (!strncmp(str, "camera_id", strlen("camera_id"))){
				getValue(str, strId);
				if (atoi(strId) == cameraId)
					bFlagBegin = 1;
			}
			continue;
		}

		if (!strncmp(str, "camera_id", strlen("camera_id")))
			break;

		if (!strncmp(key, str, strlen(key))){
			getValue(str, value);

			bRet = 1;
			break;
		}
		memset(str, 0, KEY_LENGTH);
	}

    fclose(mhKeyFile);

	return bRet;
}

int setKey(int cameraId, char *key, char *value)
{
	int bRet = 0;
	int bFlagBegin = 0;
	char strId[2];
	char str[KEY_LENGTH];
    int replaceLineLen = 0;
	FILE * mhKeyFile = NULL;
    FILE * tempKeyFile = NULL;

	if (key == 0 || value == 0){
		printf("error input para");
		return 0;
	}

	mhKeyFile = fopen(CAMERA_KEY_CONFIG_PATH, "rb+");
	if (!mhKeyFile){
		printf("error key file handle");
		return 0;
	}

    tempKeyFile = fopen("/etc/temprecorder.cfg", "wb+");
    if (!tempKeyFile){
		printf("error temp key file handle");
        fclose(mhKeyFile);
		return 0;
	}

	fseek(mhKeyFile, 0L, SEEK_SET);
    fseek(tempKeyFile, 0L, SEEK_SET);

	memset(str, 0, KEY_LENGTH);
	while (fgets(str, KEY_LENGTH , mhKeyFile)){
		if (!bFlagBegin){
			if (!strncmp(str, "camera_id", strlen("camera_id"))){
				getValue(str, strId);
				if (atoi(strId) == cameraId)
					bFlagBegin = 1;

                str[strlen(str)] = '\n';
			}

            fwrite(str, strlen(str), 1, tempKeyFile);
            memset(str, 0, KEY_LENGTH);
			continue;
		}

		if (!strncmp(str, "camera_id", strlen("camera_id"))){
            fwrite(str, strlen(str), 1, tempKeyFile);
            memset(str, 0, KEY_LENGTH);
            break;
        }

		if (!strncmp(key, str, strlen(key))){
			setValue(str, value);
            replaceLineLen = strlen(str);

            str[replaceLineLen] = '\n';

            bRet = 1;
            fwrite(str, strlen(str), 1, tempKeyFile);
		    memset(str, 0, KEY_LENGTH);
            break;
		}
        fwrite(str, strlen(str), 1, tempKeyFile);
		memset(str, 0, KEY_LENGTH);
	}

	while (fgets(str, KEY_LENGTH , mhKeyFile)){

        fwrite(str, strlen(str), 1, tempKeyFile);
        memset(str, 0, KEY_LENGTH);
	}

    fclose(mhKeyFile);
    fclose(tempKeyFile);

    remove(CAMERA_KEY_CONFIG_PATH);
    rename("/etc/temprecorder.cfg", CAMERA_KEY_CONFIG_PATH);

	return bRet;
}
