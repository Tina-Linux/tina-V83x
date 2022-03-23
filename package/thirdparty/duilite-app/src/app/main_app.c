#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "duilite.h"

char *auth_cfg = "{\"productId\":\"278581853\",\"savedProfile\":\"/mnt/app/auth/ec08463b20034e4dabf5c6e86b600b24\"}";

static int _callback(void *user_data, int type, char *msg, int len) {
	if (type == DUILITE_MSG_TYPE_JSON) {
		printf("%.*s\n", len, msg);
	} else {
		FILE *audio = fopen("/mnt/app/temp/vad.pcm", "a+");
		if (audio) {
			fwrite(msg, 1, len, audio);
			fclose(audio);
		}
	}
	return 0;
}

int main(int argc, char **argv) {
	char *path = "/mnt/app/configs/demo.json";
	duilite_parse_demo(path);

	char *cfg = "{\"resBinPath\": \"/mnt/app/bin/vad_aihome_v0.9b.bin\",\"pauseTime\":500}";
	duilite_library_load(auth_cfg);
	struct duilite_vad *vad = duilite_vad_new(cfg, _callback, NULL);
	assert(vad != NULL);
	duilite_vad_start(vad, NULL);
	remove("/mnt/app/temp/vad.pcm");
	FILE *audio = fopen(argv[1], "r");
	assert(audio != NULL);
	fseek(audio, 44, SEEK_SET);
	char buf[3200];
	int len;
	while (1) {
		len = fread(buf, 1, sizeof(buf), audio);
		if (0 == len) {
			break;
		}
		duilite_vad_feed(vad, buf, len);
	}
	fclose(audio);
	duilite_vad_delete(vad);
	duilite_library_release();
	return 0;
}
