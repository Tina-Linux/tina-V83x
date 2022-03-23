#include "usr_config.h"

int save_conf_info(const char *file_path, uint8_t* buffer)
{
    FILE *fp = NULL;

    if((fp = fopen(file_path, "w+")) == NULL)
    {
        LOGE("open/create file: %s fail", file_path);
        goto fail;
    } else {
        LOGD("open/create file: %s success", file_path);
    }

    if(fwrite(buffer, sizeof(buffer), 1, fp) == -1)
    {
        LOGE("write file: %s fail", file_path);
        goto fail;
    } else {
        LOGD("write file: %s success", file_path);
    }

    LOGI("write buffer: %d", atoi(buffer));
    fclose(fp);
    return RESULT_OK;
fail:
    if (fp != NULL) {
    fclose(fp);
    }
    return RESULT_ERR;
}

int read_conf_info(const char *file_path)
{
    FILE *fp = NULL;
    uint8_t buffer[10];

    fp = fopen(file_path, "r+");
    if(fp == NULL)
    {
        LOGE("open/create file: %s fail", file_path);
        goto fail;
    } else {
        LOGD("open/create file: %s success", file_path);
    }

    if(fread(buffer, sizeof(buffer), 1, fp) == -1)
    {
        LOGE("write file: %s fail", file_path);
        goto fail;
    } else {
        LOGD("write file: %s success", file_path);
    }
    LOGI("read buffer: %d",atoi(buffer));
    fclose(fp);
    return atoi(buffer);
fail:
    if (fp != NULL) {
    fclose(fp);
    }
    return RESULT_ERR;
}

int is_file_exist(const char *file_path, uint32_t delay_time_us)
{
    if (access(file_path, F_OK) != 0)
    {
        usleep(delay_time_us);
        return RESULT_ERR;
    }
    return RESULT_OK;
}

uint8_t random_value_in_range(uint8_t min_value, uint8_t max_value)
{
	uint8_t random_value;

	random_value = rand()%(max_value -min_value + 1) + min_value;
	//log_info("random_value:%d in range [%d,%d]\n",random_value, min_value, max_value);

	return random_value;
}
