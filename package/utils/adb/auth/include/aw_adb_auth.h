#ifndef __AW_ADB_AUTH_H
#define __AW_ADB_AUTH_H
#include <stdbool.h>
#include <ev.h>

#if __cplusplus
extern "C" {
#endif

enum {
	AW_AUTH_NONE    = 0,
	AW_AUTH_INFO    = 1 << 1,
	AW_AUTH_DEBUG   = 1 << 2,
	AW_AUTH_VERBOSE = 1 << 3,
}AW_AUTH_DEBUG_LEVEL;

typedef bool (*pubkey_detector_t)(const char *pubkey, int len);

typedef struct {
	int transmit_fd;
	pid_t adbd_pid;
	char *key_path;
	struct ev_loop *loop;
	ev_io adbd_watcher;
}aw_adbd_handle_t;

/**
 * brief set debug level
 * param level, debug level detail in AW_AUTH_DEBUG_LEVEL
 * return 0 on success otherwise a negative error code
 */
int aw_adbd_debug_level(int level);
/**
 * brief create aw adbd handle
 * return handle on success otherwise NULL
 */
aw_adbd_handle_t *aw_adbd_create(void);
/**
 * brief set public key path
 * param handle, aw adbd handle
 * param path, public key path
 * return 0 on success otherwise a negative error code
 */
int aw_adbd_set_key_path(aw_adbd_handle_t *handle, const char *path);
/**
 * brief install pubkey detector, deceided install key or not
 * param func, detecort function pointer
 */
void aw_adbd_install_pubkey_detector(pubkey_detector_t func);
/**
 * brief start adbd
 * param handle, aw adbd handle
 * return 0 on success otherwise a negative error code
 */
int aw_adbd_start(aw_adbd_handle_t *handle);
/**
 * brief start event loop(commucate with adbd)
 * param handle, aw adbd handle
 * return 0 on success otherwise a negative error code
 */
int aw_adbd_event_loop(aw_adbd_handle_t *handle);
/**
 * brief destroy adbd handle
 * param handle, aw adbd handle
 * return 0 on success otherwise a negative error code
 */
int aw_adbd_destroy(aw_adbd_handle_t *handle);

#if __cplusplus
}
#endif

#endif
