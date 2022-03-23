/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <string.h>
#include <resolv.h>
#include <cutils/list.h>
#include <cutils/sockets.h>

#include "sysdeps.h"
#include "adb.h"
#include "adb_auth.h"
#include "fdevent.h"
#include "mincrypt/rsa.h"
#include <assert.h>

#define TRACE_TAG TRACE_AUTH


struct adb_public_key {
    struct listnode node;
    RSAPublicKey key;
};

static struct listnode key_list;

char *external_key_path = NULL;

static char *key_paths[] = {
    "/mnt/UDISK/adb_keys",
    NULL
};

static fdevent listener_fde;
static int framework_fd = -1;

static const char Base64[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char Pad64 = '=';
int
b64_pton(src, target, targsize)
	char const *src;
	u_char *target;
	size_t targsize;
{
	size_t tarindex;
	int state, ch;
	char *pos;

	assert(src != NULL);
	assert(target != NULL);

	state = 0;
	tarindex = 0;

	while ((ch = (u_char) *src++) != '\0') {
		if (isspace(ch))	/* Skip whitespace anywhere. */
			continue;

		if (ch == Pad64)
			break;

		pos = strchr(Base64, ch);
		if (pos == 0) 		/* A non-base64 character. */
			return (-1);

		switch (state) {
		case 0:
			if (target) {
				if (tarindex >= targsize)
					return (-1);
				target[tarindex] = (pos - Base64) << 2;
			}
			state = 1;
			break;
		case 1:
			if (target) {
				if (tarindex + 1 >= targsize)
					return (-1);
				target[tarindex] |=
				    (u_int32_t)(pos - Base64) >> 4;
				target[tarindex+1]  = ((pos - Base64) & 0x0f)
							<< 4 ;
			}
			tarindex++;
			state = 2;
			break;
		case 2:
			if (target) {
				if (tarindex + 1 >= targsize)
					return (-1);
				target[tarindex] |=
					(u_int32_t)(pos - Base64) >> 2;
				target[tarindex+1] = ((pos - Base64) & 0x03)
							<< 6;
			}
			tarindex++;
			state = 3;
			break;
		case 3:
			if (target) {
				if (tarindex >= targsize)
					return (-1);
				target[tarindex] |= (pos - Base64);
			}
			tarindex++;
			state = 0;
			break;
		default:
			abort();
		}
	}

	/*
	 * We are done decoding Base-64 chars.  Let's see if we ended
	 * on a byte boundary, and/or with erroneous trailing characters.
	 */

	if (ch == Pad64) {		/* We got a pad char. */
		ch = *src++;		/* Skip it, get next. */
		switch (state) {
		case 0:		/* Invalid = in first position */
		case 1:		/* Invalid = in second position */
			return (-1);

		case 2:		/* Valid, means one byte of info */
			/* Skip any number of spaces. */
			for (; ch != '\0'; ch = (u_char) *src++)
				if (!isspace(ch))
					break;
			/* Make sure there is another trailing = sign. */
			if (ch != Pad64)
				return (-1);
			ch = *src++;		/* Skip the = */
			/* Fall through to "single trailing =" case. */
			/* FALLTHROUGH */

		case 3:		/* Valid, means two bytes of info */
			/*
			 * We know this char is an =.  Is there anything but
			 * whitespace after it?
			 */
			for (; ch != '\0'; ch = (u_char) *src++)
				if (!isspace(ch))
					return (-1);

			/*
			 * Now make sure for cases 2 and 3 that the "extra"
			 * bits that slopped past the last full byte were
			 * zeros.  If we don't check them, they become a
			 * subliminal channel.
			 */
			if (target && target[tarindex] != 0)
				return (-1);
		}
	} else {
		/*
		 * We ended by seeing the end of the string.  Make sure we
		 * have no partial bytes lying around.
		 */
		if (state != 0)
			return (-1);
	}

	return (tarindex);
}


static void read_keys(const char *file, struct listnode *list)
{
    struct adb_public_key *key;
    FILE *f;
    char buf[MAX_PAYLOAD];
    char *sep;
    int ret;

    f = fopen(file, "r");
    if (!f) {
        D("Can't open '%s'\n", file);
        return;
    }

    while (fgets(buf, sizeof(buf), f)) {
        /* Allocate 4 extra bytes to decode the base64 data in-place */
        key = calloc(1, sizeof(*key) + 4);
        if (!key) {
            D("Can't malloc key\n");
            break;
        }

        sep = strpbrk(buf, " \t");
        if (sep)
            *sep = '\0';
        ret = b64_pton(buf, (u_char *)&key->key, sizeof(key->key) + 4);
        if (ret != sizeof(key->key)) {
            D("%s: Invalid base64 data ret=%d\n", file, ret);
            free(key);
            continue;
        }

        if (key->key.len != RSANUMWORDS) {
            D("%s: Invalid key len %d\n", file, key->key.len);
            free(key);
            continue;
        }

        list_add_tail(list, &key->node);
    }

    fclose(f);
}

static void free_keys(struct listnode *list)
{
    struct listnode *item;

    while (!list_empty(list)) {
        item = list_head(list);
        list_remove(item);
        free(node_to_item(item, struct adb_public_key, node));
    }
}

void adb_auth_reload_keys(void)
{
    char *path;
    char **paths = key_paths;
    struct stat buf;

    free_keys(&key_list);

    if (external_key_path != NULL
	&& !stat(external_key_path, &buf))
        read_keys(external_key_path, &key_list);

    while ((path = *paths++)) {
        if (!stat(path, &buf)) {
            D("Loading keys from '%s'\n", path);
            read_keys(path, &key_list);
        }
    }
}

int adb_auth_generate_token(void *token, size_t token_size)
{
    FILE *f;
    int ret;

    f = fopen("/dev/urandom", "r");
    if (!f)
        return 0;

    ret = fread(token, token_size, 1, f);

    fclose(f);
    return ret * token_size;
}

int adb_auth_verify(void *token, void *sig, int siglen)
{
    struct listnode *item;
    struct adb_public_key *key;
    int ret;

    if (siglen != RSANUMBYTES)
        return 0;

    list_for_each(item, &key_list) {
        key = node_to_item(item, struct adb_public_key, node);
        ret = RSA_verify(&key->key, sig, siglen, token);
        if (ret)
            return 1;
    }

    return 0;
}

static void adb_auth_event(int fd, unsigned events, void *data)
{
    atransport *t = data;
    char response[2];
    int ret;

    if (events & FDE_READ) {
        ret = unix_read(fd, response, sizeof(response));
        if (ret < 0) {
            D("Disconnect");
            fdevent_remove(&t->auth_fde);
            framework_fd = -1;
        }
        else if (ret == 2 && response[0] == 'O' && response[1] == 'K') {
            adb_auth_reload_keys();
            adb_auth_verified(t);
        }
    }
}

void adb_auth_confirm_key(unsigned char *key, size_t len, atransport *t)
{
    char msg[MAX_PAYLOAD];
    int ret;

    if (framework_fd < 0) {
        D("Client not connected\n");
        return;
    }

    if (key[len - 1] != '\0') {
        D("Key must be a null-terminated string\n");
        return;
    }

    ret = snprintf(msg, sizeof(msg), "PK%s", key);
    if (ret >= (signed)sizeof(msg)) {
        D("Key too long. ret=%d", ret);
        return;
    }
    D("Sending '%s'\n", msg);

    ret = unix_write(framework_fd, msg, ret);
    if (ret < 0) {
        D("Failed to write PK, errno=%d\n", errno);
        return;
    }

    fdevent_install(&t->auth_fde, framework_fd, adb_auth_event, t);
    fdevent_add(&t->auth_fde, FDE_READ);
}

static void adb_auth_listener(int fd, unsigned events, void *data)
{
    struct sockaddr addr;
    socklen_t alen;
    int s;

    alen = sizeof(addr);

    s = adb_socket_accept(fd, &addr, &alen);
    if (s < 0) {
        D("Failed to accept: errno=%d\n", errno);
        return;
    }

    framework_fd = s;
}

void adb_auth_init(void)
{
    int fd, ret;

    D("adb_auth_init\n");
    list_init(&key_list);
    adb_auth_reload_keys();

    fd = android_get_control_socket("adbd");
    if (fd < 0) {
        D("Failed to get adbd socket\n");
        return;
    }

    ret = listen(fd, 4);
    if (ret < 0) {
        D("Failed to listen on '%d'\n", fd);
        return;
    }

    fdevent_install(&listener_fde, fd, adb_auth_listener, NULL);
    fdevent_add(&listener_fde, FDE_READ);
}
