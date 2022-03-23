#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stddef.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <signal.h>

#include <ev.h>
#include <aw_adb_auth.h>

#define ANDROID_SOCKET_ENV_PREFIX	"ANDROID_SOCKET_"
#define ANDROID_SOCKET_DIR		"/dev/socket/"
#define DEFAULT_KEY_PATH		"/mnt/UDISK/adb_keys"

#define  AW_AUTH_PRINT(level_mask, fmt, arg...) \
{ \
	if (debug_mask >= level_mask) { \
		printf("[%s:%d] "fmt"", __FUNCTION__, __LINE__, ##arg); \
	} \
}

static int debug_mask = 0;
static pubkey_detector_t g_pubkey_detector_func;
static char *g_key_path = DEFAULT_KEY_PATH;

void free_environment(const char *env[], int num)
{
	int i;
	for (i = 0; i < num; i++) {
		if (!env[i])
			continue;
		free(env[i]);
		env[i] = NULL;
	}
}

int add_environment(const char *env[], const char *key, const char *val)
{
	int n;

	for (n = 0; n < 31; n++) {
		if (!env[n]) {
			size_t len = strlen(key) + strlen(val) + 2;
			char *entry = malloc(len);
			snprintf(entry, len, "%s=%s", key, val);
			env[n] = entry;
			return 0;
		}
	}

	return 1;
}

int create_socket(const char *name, int type, mode_t perm, uid_t uid, gid_t gid)
{
	struct sockaddr_un addr;
	int fd, ret;
	struct stat sb;

	fd = socket(PF_UNIX, type, 0);
	if (fd < 0) {
		printf("Failed to open socket '%s': %s\n", name,
		       strerror(errno));
		return -1;
	}

	if (stat(ANDROID_SOCKET_DIR, &sb) < 0)
		mkdir(ANDROID_SOCKET_DIR, 0755);

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	snprintf(addr.sun_path, sizeof(addr.sun_path), ANDROID_SOCKET_DIR "%s",
		 name);

	ret = unlink(addr.sun_path);
	if (ret != 0 && errno != ENOENT) {
		printf("Failed to unlink old socket '%s': %s\n", name,
		       strerror(errno));
		goto out_close;
	}

	ret = bind(fd, (struct sockaddr *)&addr, sizeof(addr));
	if (ret) {
		printf("Failed to bind socket '%s': %s\n", name,
		       strerror(errno));
		goto out_unlink;
	}

	chown(addr.sun_path, uid, gid);
	chmod(addr.sun_path, perm);

	AW_AUTH_PRINT(AW_AUTH_DEBUG,
	      "Created socket '%s' with mode '%o', user '%d', group '%d'\n",
	      addr.sun_path, perm, uid, gid);

	return fd;

out_unlink:
	unlink(addr.sun_path);
out_close:
	close(fd);
	return -1;
}

static void publish_socket(const char *env[], const char *name, int fd)
{
	char key[64] = ANDROID_SOCKET_ENV_PREFIX;
	char val[64];

	strlcpy(key + sizeof(ANDROID_SOCKET_ENV_PREFIX) - 1,
		name, sizeof(key) - sizeof(ANDROID_SOCKET_ENV_PREFIX));
	snprintf(val, sizeof(val), "%d", fd);
	add_environment(env, key, val);

	/* make sure we don't close-on-exec */
	fcntl(fd, F_SETFD, 0);
}

static int socket_local_client_connect(int fd, const char *name)
{
	struct sockaddr_un addr;
	socklen_t alen;
	int err;
	size_t namelen;

	memset(&addr, 9, sizeof(addr));
	namelen = strlen(name) + strlen(ANDROID_SOCKET_DIR);
	if (namelen > sizeof(addr) - offsetof(struct sockaddr_un, sun_path) - 1)
		 goto error;
	strcpy(addr.sun_path, ANDROID_SOCKET_DIR);
	strcat(addr.sun_path, name);
	addr.sun_family = AF_LOCAL;
	alen = namelen + offsetof(struct sockaddr_un, sun_path) + 1;

	if (connect(fd, (struct sockaddr *)&addr, alen) < 0) {
		goto error;
	}

	return fd;

error:
	return -1;
}

static int socket_local_client(const char *name, int type)
{
	int s;

	s = socket(AF_LOCAL, type, 0);
	if (s < 0)
		return -1;

	if (0 > socket_local_client_connect(s, name)) {
		close(s);
		return -1;
	}

	return s;
}

#define MAX_PAYLOAD 4096

int mkdirmulti(char *path)
{
	char *curDir = NULL, *ptr;
	int len;

	AW_AUTH_PRINT(AW_AUTH_DEBUG, "%s\n", path);
	if (!access(path, F_OK))
		return 0;

	ptr = strrchr(path, '/');
	if (!ptr)
		return -1;
	/* it means / dir */
	if (ptr == path) {
		mkdir(path, 0644);
		return 0;
	}
	len = ptr - path + 1;
	curDir = malloc(len);
	memset(curDir, 0, len);
	memcpy(curDir, path, len - 1);
	if (!mkdirmulti(curDir)) {
		free(curDir);
		mkdir(path, 0644);
		return 0;
	}
	return -1;
}

static int create_dir(const char *path)
{
	char *buf[MAX_PAYLOAD];
	char *ptr = NULL;

	AW_AUTH_PRINT(AW_AUTH_DEBUG, "create %s\n", path);
	/* ignore file */
	ptr = strrchr(path, '/');
	if (!ptr)
		return ;
	memset(buf, 0, sizeof(buf));
	memcpy(buf, path, ptr-path);
	buf[ptr-path] = '\0';

	/* mkdir dir */
	return mkdirmulti(buf);
}

static void adbd_handler(EV_P_ struct ev_io *w, int revents)
{
	char buf[MAX_PAYLOAD];
	int len;

	len = recv(w->fd, buf, sizeof(buf), MSG_DONTWAIT);
	if (len < 1)
		return;
	buf[len] = '\n';
	AW_AUTH_PRINT(AW_AUTH_DEBUG, "recv msg:\n");
	AW_AUTH_PRINT(AW_AUTH_DEBUG, "%s\n", buf);
	if (buf[0] == 'P', buf[1] == 'K') {
		struct stat sb;
		int fd, size;

		/* detect public key install permission */
		if (g_pubkey_detector_func != NULL &&
		    !(*g_pubkey_detector_func) (&buf[2], len - 1)) {
			AW_AUTH_PRINT(AW_AUTH_INFO,
				      "pubkey_detector_func return false,"
				      "don't install public key!\n");
			return;
		}

		AW_AUTH_PRINT(AW_AUTH_INFO, "install public key!!\n");
		if (stat(g_key_path, &sb) < 0) {
			create_dir(g_key_path);
			fd = open(g_key_path, O_RDWR | O_CREAT, 664);
		} else
			fd = open(g_key_path, O_RDWR);
		if (fd < 0) {
			printf("open %s failed\n", g_key_path);
			return;
		}
		lseek(fd, 0, SEEK_END);
		size = write(fd, &buf[2], len - 1);
		if (size != (len - 1)) {
			printf("write error, expect %d, actual %d bytes\n",
			       len - 2, size);
		}
		close(fd);
		/* install public key successful, and ack to adbd */
		write(w->fd, "OK", 2);
	}
	return;
}

int aw_adbd_set_key_path(aw_adbd_handle_t *handle, const char *path)
{
	if (!handle || !path)
		return -1;
	handle->key_path = strdup(path);
	g_key_path = handle->key_path;
	return 0;
}

void aw_adbd_install_pubkey_detector(pubkey_detector_t func)
{
	g_pubkey_detector_func = func;
}

int aw_adbd_event_loop(aw_adbd_handle_t *handle)
{

	if (!handle)
		return -1;

	AW_AUTH_PRINT(AW_AUTH_DEBUG, "enter aw_adbd_event_loop\n");
	handle->loop = ev_default_loop(0);
	ev_io_init(&handle->adbd_watcher, adbd_handler, handle->transmit_fd, EV_READ);
	ev_io_start(handle->loop, &handle->adbd_watcher);
	ev_run(handle->loop, 0);
	AW_AUTH_PRINT(AW_AUTH_DEBUG, "exit aw_adbd_event_loop\n");
	return 0;
}

aw_adbd_handle_t *aw_adbd_create(void)
{
	aw_adbd_handle_t *handle;

	handle = malloc(sizeof(aw_adbd_handle_t));
	if (!handle) {
		printf("no memory.\n");
		return NULL;
	}

	return handle;
}

int aw_adbd_destroy(aw_adbd_handle_t *handle)
{
	/*ev_break(handle->loop, EVBREAK_ALL);*/
	ev_io_stop(handle->loop, &handle->adbd_watcher);
	ev_loop_destroy(handle->loop);
	if (handle->adbd_pid != 0)
		kill(handle->adbd_pid, SIGTERM);
	if (handle->key_path != NULL)
		free(handle->key_path);
	memset(handle, 0, sizeof(aw_adbd_handle_t));
	free(handle);
}

int aw_adbd_debug_level(int level)
{
	debug_mask = level;
	return 0;
}

int aw_adbd_start(aw_adbd_handle_t *handle)
{
	if (!handle)
		return -1;

	handle->adbd_pid = fork();
	if (handle->adbd_pid == 0) {
		int s;
		char *cmd[] = { "/bin/adbd", NULL };
		const char *ENV[32] = {0};
		char *adb_transport_port = NULL;

		if (debug_mask < AW_AUTH_VERBOSE) {
			int fd;
			fd = open("/dev/null", O_RDWR);
			if (fd > -1) {
				dup2(fd, STDIN_FILENO);
				dup2(fd, STDOUT_FILENO);
				dup2(fd, STDERR_FILENO);
				if (fd > STDERR_FILENO)
					close(fd);
			}
		}
		s = create_socket("adbd", SOCK_STREAM, 660, 0, 0);
		if (s >= 0)
			publish_socket(ENV, "adbd", s);
		/* add some env into adbd */
		add_environment(ENV, "ADB_AUTH_ENABLE", "1");
		adb_transport_port = getenv("ADB_TRANSPORT_PORT");
		if (adb_transport_port != NULL)
			add_environment(ENV,
					"ADB_TRANSPORT_PORT",
					adb_transport_port);
		if (g_key_path != NULL)
			add_environment(ENV,
					"ADB_AUTH_EXTERNAL_KEY_PATH",
					g_key_path);
		execve(cmd[0], cmd, (char **)ENV);
		printf("execve failed\n");
		free_environment(ENV, sizeof(ENV));
		exit(-1);
	}
	/* delay to ensure adbd had created /dev/socket/adbd */
	usleep(300000);
	handle->transmit_fd = socket_local_client("adbd", SOCK_STREAM);
	if (handle->transmit_fd < 0) {
		printf("socket_local_client failed, return %d\n",
					handle->transmit_fd);
		kill(handle->adbd_pid, SIGTERM);
		handle->adbd_pid = 0;
		return -1;
	}

	return 0;
}
