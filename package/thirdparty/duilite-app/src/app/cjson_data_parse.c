#include "cjson_data_parse.h"

/*
typedef struct {
    char *func;

    wakeup_audio_t *words;
    int words_count;
    char *begin_timeout_prompt;
    char *error_prompt;
} app_cfg_t;
static app_cfg_t app_cfg;
*/

static char *read_config_file(char *file, int *len) {
    FILE *fd = fopen(file, "r");
    int size;
    char *data = NULL;
    if (fd) {
        fseek(fd, 0, SEEK_END);
        size = ftell(fd);
        fseek(fd, 0, SEEK_SET);
        data = (char *)malloc(size + 1);
        if (data) {
            fread(data, 1, size, fd);
            if (len) *len = size;
        }
        fclose(fd);
    }
    return data;
}

/*---------------------------------------------------
function  :  char *cjson(char *buf)
effect    :  Use cjson to parse the data received by WiFi in JSON format.
attribute :  The function returns a string that passes a string as a parameter.
---------------------------------------------------*/
/*
static int parse_app_config(char *file) {
	char *cfg = read_config_file(file, NULL);
	if (!cfg) {
		printf("read_config_file:%s fail!\n", *file);
		return -1;
	}

	cJSON *root = cJSON_Parse(cfg);
	if (!root) {
		printf("cJSON_Parse fail!\n");
		return -1;
	}

	app_cfg.func = cJSON_Print(cJSON_GetObjectItem(root, "func"));
	cJSON *vad_js = cJSON_GetObjectItem(app_cfg.func, "vad");
	cJSON *vad_cfg_js = cJSON_GetObjectItem(vad_js, "cfg");
	int i;
	for (i = 0; i < app_cfg.words_count; i++) {
		cJSON *item_js = cJSON_GetArrayItem(wakeup_js, i);
		cJSON *word_js = cJSON_GetObjectItem(item_js, "word");
	        cJSON *audio_js = cJSON_GetObjectItem(item_js, "audio");
	        app_cfg.words[i].word = os_strdup(word_js->valuestring);
	        app_cfg.words[i].audio = os_strdup(audio_js->valuestring);
	}
	cJSON *vad_js = cJSON_GetObjectItem(app_js, "vad");
	cJSON *beginTimeoutAudio_js = cJSON_GetObjectItem(vad_js, "beginTimeoutAudio");
	app_cfg.begin_timeout_prompt = os_strdup(beginTimeoutAudio_js->valuestring);
	cJSON *network_js = cJSON_GetObjectItem(app_js, "network");
	cJSON *errorAudio_js = cJSON_GetObjectItem(network_js, "errorAudio");
	app_cfg.error_prompt = os_strdup(errorAudio_js->valuestring);

	cJSON_Delete(js);
	return 0;
}
*/

int duilite_parse_demo(char *file)
{
    char *cfg = read_config_file(file, NULL);
    if (!cfg) {
        printf("read_config_file:%s fail!\n", *file);
	return -1;
    }

    cJSON *json = cJSON_Parse(cfg);
    if ( NULL != json ) {
        cJSON * temp = cJSON_GetObjectItem(json, "name");
        if ( NULL != temp )
	    printf( "name : %s\n", temp->valuestring);

	temp = cJSON_GetObjectItem(json, "province");
	printf( "province : \n");
        if ( NULL != temp ) {
            int i = 0;
            int icount = cJSON_GetArraySize(temp);
            for (; i < icount; ++i)
            {
                cJSON * province = cJSON_GetArrayItem(temp, i);
                if ( NULL != province)  {
                    cJSON *name = NULL;
                    cJSON *cities = NULL;
                    name = cJSON_GetObjectItem(province, "name");
                    cities = cJSON_GetObjectItem(province, "cities");
	            if ( NULL != name )
		        printf("    name : %s\n", name->valuestring);

	            printf("    cities : \n");
		    if ( NULL != cities ) {
		        cJSON *city = cJSON_GetObjectItem(cities, "city");

			printf ("        city:");
			if ( NULL != city ) {
		            int j = 0;
			    int jcount = cJSON_GetArraySize(city);
	                    for (; j < jcount; ++j) {
			        cJSON *cityItem = cJSON_GetArrayItem(city, j);
	                        if ( NULL != cityItem )
	                            printf ("%s ", cityItem->valuestring);
		            }
	                }
	                printf ("\n\n");
		    }
	        }
            }
	}
	cJSON_Delete(json);
	json = NULL;
    }
}
#if 0
int duilite_parse_func(char *file)
{
    char *cfg = read_config_file(file, NULL);
    if (!cfg) {
        printf("read_config_file:%s fail!\n", *file);
	return -1;
    }

    cJSON *json = cJSON_Parse(cfg);
    if ( NULL != json ) {
        cJSON * temp = cJSON_GetObjectItem(json, "name");
        if ( NULL != temp )
	    printf( "name : %s\n", temp->valuestring);

	temp = cJSON_GetObjectItem(json, "province");
	printf( "province : \n");
        if ( NULL != temp ) {
            int i = 0;
            int icount = cJSON_GetArraySize(temp);
            for (; i < icount; ++i)
            {
                cJSON * province = cJSON_GetArrayItem(temp, i);
                if ( NULL != province)  {
                    cJSON *name = NULL;
                    cJSON *cities = NULL;
                    name = cJSON_GetObjectItem(province, "name");
                    cities = cJSON_GetObjectItem(province, "cities");
	            if ( NULL != name )
		        printf("    name : %s\n", name->valuestring);

	            printf("    cities : \n");
		    if ( NULL != cities ) {
		        cJSON *city = cJSON_GetObjectItem(cities, "city");

			printf ("        city:");
			if ( NULL != city ) {
		            int j = 0;
			    int jcount = cJSON_GetArraySize(city);
	                    for (; j < jcount; ++j) {
			        cJSON *cityItem = cJSON_GetArrayItem(city, j);
	                        if ( NULL != cityItem )
	                            printf ("%s ", cityItem->valuestring);
		            }
	                }
	                printf ("\n\n");
		    }
	        }
            }
	}
	cJSON_Delete(json);
	json = NULL;
    }
}
#endif
