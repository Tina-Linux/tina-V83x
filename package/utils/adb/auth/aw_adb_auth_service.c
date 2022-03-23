#include <stdio.h>
#include <signal.h>
#include <aw_adb_auth.h>

aw_adbd_handle_t *handle = NULL;

static void signal_handler(int sig)
{
	switch (sig) {
		case SIGTERM:
		aw_adbd_destroy(handle);
		break;
	}
}

static bool publickey_detector(const char *pubkey, int len)
{
	char *ptr = NULL;
	printf("get public key:\n%s\n", pubkey);
	ptr = strstr(pubkey, "forevercai");
	if (!ptr)
		return false;
	return true;
}

int main()
{
	int ret;

	signal(SIGTERM, signal_handler);

	aw_adbd_debug_level(AW_AUTH_INFO);
	handle = aw_adbd_create();
	if (!handle) {
		printf("aw_adbd_create failed\n");
		return -1;
	}
	/*aw_adbd_set_key_path(handle, "/opt/adb_keys");*/
	ret = aw_adbd_start(handle);
	if (ret != 0) {
		printf("aw_adbd_start failed\n");
		return -1;
	}

	/*aw_adbd_install_pubkey_detector(publickey_detector); */
	aw_adbd_event_loop(handle);
	printf("adbd_auth_service finish\n");
}
