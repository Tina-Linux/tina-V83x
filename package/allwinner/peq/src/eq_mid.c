#include <pthread.h>
#include "eq.h"
#include "eq_mid.h"
#include "cJSON.h"
#include <stdio.h>
#include <string.h>

static eq_prms_t prms;
static eq_prms_t prms_tmp;
static void *equalizer = NULL;
static void *equalizer_tmp = NULL;
static int bready_init = 0;
#define EQ_CONFIG_PATH "/mnt/SDCARD/eq.json"
#ifdef SYNC_TEST_EQ_AND_PLAYER
static pthread_t input_thread_id;
static pthread_mutex_t peq_sync_equalizer_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t peq_sync_param_lock = PTHREAD_MUTEX_INITIALIZER;
static unsigned char peq_sync_input_param_status = 1;
#endif
static int peq_param_process(void)
{
    unsigned int Index = 0, Index_tmp = 0;
    int freq_down_lim = 0, freq_up_lim = 0;

#ifdef SYNC_TEST_EQ_AND_PLAYER
    pthread_mutex_lock(&peq_sync_param_lock);
#endif
    freq_up_lim = prms.sampling_rate / 2;
    for (Index_tmp = 0; Index_tmp < prms_tmp.biq_num; Index_tmp++)
    {
        if (freq_up_lim > prms_tmp.core_prms[Index_tmp].fc && prms_tmp.core_prms[Index_tmp].fc > freq_down_lim)
        {
            prms.core_prms[Index].fc = prms_tmp.core_prms[Index_tmp].fc;
            prms.core_prms[Index].Q = prms_tmp.core_prms[Index_tmp].Q;
            prms.core_prms[Index].G = prms_tmp.core_prms[Index_tmp].G;
            prms.core_prms[Index].type = prms_tmp.core_prms[Index_tmp].type;
            Index++;
        }
    }
    prms.biq_num = Index;
#ifdef SYNC_TEST_EQ_AND_PLAYER
    pthread_mutex_unlock(&peq_sync_param_lock);
#endif
    return 0;
}
static int peq_load_eq_param_json(unsigned char *path)
{
    FILE *fp = NULL;
    char *data = NULL;
    int bin_num = 0, len = 0;
    cJSON *param_obj = NULL, *arrayItem_obj = NULL, *item_obj = NULL, *item = NULL;

    if (path != NULL)
    {
        fp = fopen(path, "rb");
    }
    else
    {
        fp = fopen(EQ_CONFIG_PATH, "rb");
    }
    if (fp == NULL)
    {
        printf("fopen %s fail!\n", EQ_CONFIG_PATH);
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    data = (char *)malloc(len + 1);
    if (data == NULL)
    {
        fclose(fp);
        printf("malloc fail!\n");
        return -1;
    }
    memset(data, 0x00, sizeof(char) * (len + 1));
    fread(data, 1, len, fp);
    fclose(fp);

    param_obj = cJSON_Parse((const char *)data);
    if (NULL == param_obj)
    {
        free(data);
        printf("cJSON_Parse fail!\n");
        return -1;
    }
    arrayItem_obj = cJSON_GetObjectItem(param_obj, "param");
    if (!arrayItem_obj)
    {
        free(data);
		cJSON_Delete(param_obj);
        printf("cJSON_GetObjectItem fail!\n");
        return -1;
    }
    bin_num = cJSON_GetArraySize(arrayItem_obj);
	if (bin_num <= 0)
	{
		free(data);
		cJSON_Delete(param_obj);
		LOGE("cJSON_GetArraySize fail!\n");
		return -1;
	}
    prms_tmp.biq_num = bin_num;
    memset(prms_tmp.core_prms, 0x00, sizeof(eq_core_prms_t)*EQ_PROCCESS_MAX_FREQ_NUM);
    if (bin_num > 0 && bin_num <= EQ_PROCCESS_MAX_FREQ_NUM)
    {
        float Q = 0.0;
        unsigned int Index = 0, freq = 0, G = 0, type = 0;

        for (Index = 0; Index < bin_num; Index++)
        {
            item_obj = cJSON_GetArrayItem(arrayItem_obj, Index);;
            item = cJSON_GetObjectItem(item_obj, "freq");
            if (item_obj)
            {
                freq = item->valueint;
            }

            item = cJSON_GetObjectItem(item_obj, "G");
            if (item_obj)
            {
                G = item->valueint;
            }

            item = cJSON_GetObjectItem(item_obj, "Q");
            if (item_obj)
            {
                Q = item->valuedouble;
            }
            item = cJSON_GetObjectItem(item_obj, "type");
            if (item_obj)
            {
                type = item->valueint;
            }
            prms_tmp.core_prms[Index].fc = freq;
            prms_tmp.core_prms[Index].G = G;
            prms_tmp.core_prms[Index].Q = Q;
            prms_tmp.core_prms[Index].type = type;
        }
        cJSON_Delete(param_obj);
        free(data);
        return 0;
    }
    else
    {
        printf("bin_num: %d out of range!\n", bin_num);
        prms_tmp.biq_num = 0;
        cJSON_Delete(param_obj);
        free(data);
        return -1;
    }
}

#ifdef SYNC_TEST_EQ_AND_PLAYER
static int peq_display_eq_param(void)
{
    int idx = 0;
    pthread_mutex_lock(&peq_sync_param_lock);
    for (idx = 0; idx < prms.biq_num; idx++)
    {
        printf("****************************************\n");
        eqlog("prms.core_prms[%d].fc:%d", idx, prms.core_prms[idx].fc);
        eqlog("prms.core_prms[%d].type:%d", idx, prms.core_prms[idx].type);
        eqlog("prms.core_prms[%d].G:%d", idx, prms.core_prms[idx].G);
        eqlog("prms.core_prms[%d].Q:%f", idx, prms.core_prms[idx].Q);
    }
    pthread_mutex_unlock(&peq_sync_param_lock);
}
static int peq_keep_eq_param(unsigned char *Path)
{
    FILE *fp = NULL;
    unsigned char *buf = NULL;
    unsigned int idx = 0;
    cJSON *all_obj = NULL, *arry_obj = NULL, *item_obj = NULL;

    fp = fopen(Path, "w+");
    fseek(fp, 0, SEEK_SET);

    all_obj = cJSON_CreateObject();	//创建根数据对象
    cJSON_AddItemToObject(all_obj, "param", arry_obj = cJSON_CreateArray());	//创建param数组

    pthread_mutex_lock(&peq_sync_param_lock);
    for (idx = 0; idx < prms.biq_num; idx++)
    {
        cJSON_AddItemToArray(arry_obj, item_obj = cJSON_CreateObject());
        cJSON_AddNumberToObject(item_obj, "freq", prms.core_prms[idx].fc);
        cJSON_AddNumberToObject(item_obj, "type", prms.core_prms[idx].type);
        cJSON_AddNumberToObject(item_obj, "G", prms.core_prms[idx].G);
        cJSON_AddNumberToObject(item_obj, "Q", prms.core_prms[idx].Q);
    }
    pthread_mutex_unlock(&peq_sync_param_lock);
    buf = cJSON_Print(all_obj);
    //printf("cJSON_Print:%s\r\n", buf);
    fwrite(buf, 1, strlen(buf), fp);
    free(buf);
    fclose(fp);
    cJSON_Delete(all_obj);
}

static int peq_input_eq_param_json(int idx)
{
    int freq = 0, gain = 0, temp = 0, type = 0;
    float Q = 0.0;
    if (idx >= 0 && idx < EQ_PROCCESS_MAX_FREQ_NUM)
    {
        eqlog("****************** %d *************", idx + 1);
        printf("\n****************** filter_types *************\n");
        printf("%d:Low_freq_shelving\n", LOWPASS_SHELVING);
        printf("%d:Bandpass_peak\n", BANDPASS_PEAK);
        printf("%d:High_freq_shelving\n", HIHPASS_SHELVING);
        printf("%d:Low_pass\n", LOWPASS);
        printf("%d:High_pass\n", HIGHPASS);
        printf("please input the filter type you want:\n");
        scanf("%d", &temp);
        switch (temp)
        {
            case LOWPASS_SHELVING:
                type = LOWPASS_SHELVING;
                eqlog("* freq point(hz) : ");
                scanf("%d", &freq);
                eqlog("* gain( -20 - 20 db) : ");
                scanf("%d", &gain);
                Q = 1;
                break;
            case BANDPASS_PEAK:
                type = BANDPASS_PEAK;
                eqlog("* freq point(hz) : ");
                scanf("%d", &freq);
                eqlog("* gain( -20 - 20 db) : ");
                scanf("%d", &gain);
                eqlog("* q( the bigger, the narrower: the smaller, the wider ) : ");
                scanf("%f", &Q);
                break;
            case HIHPASS_SHELVING:
                type = HIHPASS_SHELVING;
                eqlog("* freq point(hz) : ");
                scanf("%d", &freq);
                eqlog("* gain( -20 - 20 db) : ");
                scanf("%d", &gain);
                Q = 1;
                break;
            case LOWPASS:
                type = LOWPASS;
                eqlog("* freq point(hz) : ");
                scanf("%d", &freq);
                gain = 0;
                Q = 1;
                break;
            case HIGHPASS:
                type = HIGHPASS;
                eqlog("* freq point(hz) : ");
                scanf("%d", &freq);
                gain = 0;
                Q = 1;
                break;
            default:
                eqlog("error input");
                return -1;
                break;
        }
        prms.core_prms[idx].fc = freq;
        prms.core_prms[idx].type = type;
        prms.core_prms[idx].G = gain;
        prms.core_prms[idx].Q = Q;
    }
    return 0;
}
static void *peq_input_thread_func(void)
{
    void *tmp = NULL;
    unsigned char Path[256] = {0};
    int idx = 0, temp = 0, ret = 0, bin_num = 0, cmd = 0;

    idx = 0;
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, PTHREAD_CANCEL_ENABLE);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, PTHREAD_CANCEL_DEFERRED);
    while (peq_sync_input_param_status)
    {
        eqlog("*************current EQ param:*************");
        peq_display_eq_param();
        eqlog("*************Command List:*************");
        eqlog("******cmd 0 : modify param  *****");
        eqlog("******cmd 1 : add param  *****");
        eqlog("******cmd 2 : reset all param  *****");
        eqlog("******cmd 3 : keep current param to file*****");
        eqlog("******cmd 4 : load param file*****");
        eqlog("plese input cmd:");
        scanf("%d", &cmd);
        switch (cmd)
        {
            case 0://modify
            {
                eqlog("*************Which Item:*************");
                scanf("%d", &temp);
                eqlog("*************Item:%d will modify*************", temp);
                peq_input_eq_param_json(temp);
            }
            break;
            case 1://add
            {
                pthread_mutex_lock(&peq_sync_param_lock);
                if (prms.biq_num < EQ_PROCCESS_MAX_FREQ_NUM && prms.biq_num >= 0)
                {
                    peq_input_eq_param_json(prms.biq_num);
                    prms.biq_num++;
                }
                else
                {
                    pthread_mutex_unlock(&peq_sync_param_lock);
                    continue;
                }
                pthread_mutex_unlock(&peq_sync_param_lock);
            }
            break;
            case 2://reset all
            {
                pthread_mutex_lock(&peq_sync_param_lock);
                prms.biq_num = 0;
                memset(prms.core_prms, 0x00, sizeof(eq_core_prms_t)*EQ_PROCCESS_MAX_FREQ_NUM);
                pthread_mutex_unlock(&peq_sync_param_lock);
                eqlog("*************Input EQ Process Freq Num:*************");
                scanf("%d", &temp);
                eqlog("*************You Input EQ Process Freq Num:%d*************", temp);
                if (temp <= EQ_PROCCESS_MAX_FREQ_NUM && temp > 0)
                {
                    pthread_mutex_lock(&peq_sync_param_lock);
                    prms.biq_num = temp;
                    for (idx = 0; idx < prms.biq_num; idx++)
                    {
                        peq_input_eq_param_json(idx);
                    }
                    pthread_mutex_unlock(&peq_sync_param_lock);
                }
                else
                {
                    continue;
                }
            }
            break;
            case 3://key param
            {
                eqlog("\n*************Input PATH and Name EQ Process param will be keep:*************");
                scanf("%255s", Path);
                eqlog("\n*************You Input PATH and Name:%s*************", Path);
                peq_keep_eq_param(Path);
                continue;
            }
            break;
            case 4://load param file
            {
                eqlog("\n*************Input PATH and Name EQ Process param:*************");
                scanf("%255s", Path);
                eqlog("\n*************You Input PATH and Name:%s*************", Path);
                peq_load_eq_param_json(Path);
            }
            break;
        }
        //peq_param_process();

        pthread_mutex_lock(&peq_sync_param_lock);
        equalizer_tmp = eq_create(&prms);
        pthread_mutex_unlock(&peq_sync_param_lock);
        if (equalizer_tmp == NULL)
        {
            eqlog("create equalizer handle error!");
            return ;
        }
        pthread_mutex_lock(&peq_sync_equalizer_lock);
        tmp = equalizer;
        equalizer = equalizer_tmp;
        pthread_mutex_unlock(&peq_sync_equalizer_lock);
        if (tmp != NULL)
        {
            printf("%s %d %s\r\n", __FILE__, __LINE__, __func__);
            eq_destroy(tmp);
        }
        equalizer_tmp = NULL;
    }
    printf("%s %d %s\r\n", __FILE__, __LINE__, __func__);
    pthread_exit(NULL);
    return NULL;
}
#endif
void eq_init(unsigned int samplerate_tmp, unsigned int chan_tmp)
{
    int ret = 0;
    void *tmp = NULL;

    if (bready_init == 0)
    {
        prms_tmp.biq_num = 0;
		if (prms_tmp.core_prms == NULL)
		{
			prms_tmp.core_prms = calloc(sizeof(eq_core_prms_t), EQ_PROCCESS_MAX_FREQ_NUM);
		}

        prms.biq_num = 0;
		if (prms.core_prms == NULL)
		{
			prms.core_prms = calloc(sizeof(eq_core_prms_t), EQ_PROCCESS_MAX_FREQ_NUM);
		}
        ret = peq_load_eq_param_json(NULL);
        if (ret == -1)
        {
#ifdef SYNC_TEST_EQ_AND_PLAYER
            goto err;
#else
            eqlog("peq_load_eq_param_json %s fail\r\n", EQ_CONFIG_PATH);
            return ;
#endif
        }
        bready_init = 1;
    }
    prms.sampling_rate = samplerate_tmp;
    prms.chan = chan_tmp;
    peq_param_process();
    equalizer_tmp = eq_create(&prms);
#ifdef SYNC_TEST_EQ_AND_PLAYER
    pthread_mutex_lock(&peq_sync_equalizer_lock);
#endif
    tmp = equalizer;
    equalizer = equalizer_tmp;
#ifdef SYNC_TEST_EQ_AND_PLAYER
    pthread_mutex_unlock(&peq_sync_equalizer_lock);
#endif
    if (tmp != NULL)
    {
        eq_destroy(tmp);
    }
    equalizer_tmp = NULL;
#ifdef SYNC_TEST_EQ_AND_PLAYER
err:
    ret = pthread_create(&input_thread_id, NULL, (void *)&peq_input_thread_func, NULL);
    if (ret != 0)
    {
        printf("create adt thread faild : %s\n", strerror(ret));
        return ;
    }
    else
    {
        printf("adt thread: %lu created \n", input_thread_id);
    }
    return ;
#endif
}
void eq_mid_proccess(char *buffer, unsigned int len)
{
#ifdef SYNC_TEST_EQ_AND_PLAYER
    pthread_mutex_lock(&peq_sync_equalizer_lock);
#endif
    if (equalizer != NULL)
    {
        eq_process(equalizer, (short *)buffer, len / (prms.chan * sizeof(short)));
    }
#ifdef SYNC_TEST_EQ_AND_PLAYER
    pthread_mutex_unlock(&peq_sync_equalizer_lock);
#endif
}

void eq_mid_destroy(void)
{
#ifdef SYNC_TEST_EQ_AND_PLAYER
    pthread_mutex_lock(&peq_sync_equalizer_lock);
#endif
    free(prms.core_prms);
    bready_init = 0;
    eq_destroy(equalizer);
    equalizer = NULL;
#ifdef SYNC_TEST_EQ_AND_PLAYER
    pthread_mutex_unlock(&peq_sync_equalizer_lock);
    peq_sync_input_param_status = 0;
    pthread_cancel(input_thread_id);
#endif
}
