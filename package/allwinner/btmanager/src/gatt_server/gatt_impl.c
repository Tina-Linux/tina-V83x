/*
 *  Copyright (C) 2019 Realtek Semiconductor Corporation.
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <semaphore.h>
#include <pthread.h>

#include "log.h"
#include "gatt_blue_cmd.h"

#include "lib/bluetooth.h"
#include "lib/hci.h"
#include "lib/hci_lib.h"
#include "lib/l2cap.h"
#include "lib/uuid.h"

#include "src/shared/mainloop.h"
#include "src/shared/util.h"
#include "src/shared/att.h"
#include "src/shared/queue.h"
//#include "src/shared/timeout.h"
#include "src/shared/gatt-db.h"
#include "src/shared/gatt-server.h"
#include "gatt_impl.h"
#include "gatt_server_glue.h"
#include "aw_gatt_server.h"

/* cmd handler declaration */
static int gatt_add_service(int argc, void **argv);
static int gatt_add_char(int argc, void **argv);
static int gatt_add_desc(int argc, void **argv);
static int gatt_start_service(int argc, void **argv);
static int gatt_stop_service(int argc, void **argv);
static int gatt_send_rsp(int argc, void **argv);
static int gatt_send_ind(int argc, void **argv);
static int gatt_del_service(int argc, void **argv);
static int gatt_unreg_service(int argc, void **argv);

#define ATT_CID     4

struct server {
	int fd;
	struct bt_att *att;
	struct gatt_db *db;
	struct bt_gatt_server *gatt;

	uint8_t *device_name;
	unsigned int name_len;
	char bd_addr[18];
	sem_t bt_sem;

	/* transaction id for a req/rsp sequence. */
	uint32_t trans_id;
	/* for current transaction */
	uint32_t cur_trans_id;
	unsigned int id; /* For internal att */

	bool in_read_req;
	uint8_t curr_opcode;

};

struct service_data {
	struct gatt_db_attribute *match;
	bool found;
	uint16_t handle;
};

static pthread_t gatt_tid;
static int sockfds[2];
static struct server *gatt_server;

static bool verbose = true;
static int mainloop_running;
static int netcfg_state = 0;

static void att_debug_cb(const char *str, void *user_data)
{
	const char *prefix = user_data;

	pr_info("%s" "%s", prefix, str);
}

static void gatt_debug_cb(const char *str, void *user_data)
{
	const char *prefix = user_data;

	pr_info("%s %s", prefix, str);
}

void print_uuid(const bt_uuid_t * uuid)
{
	char uuid_str[MAX_LEN_UUID_STR];
	bt_uuid_t uuid128;

	bt_uuid_to_uuid128(uuid, &uuid128);
	bt_uuid_to_string(&uuid128, uuid_str, sizeof(uuid_str));

	pr_info("%s", uuid_str);
}

static void server_destroy(struct server *server)
{
	pr_info("++");
	pr_info("Freeing gatt");
	bt_gatt_server_unref(server->gatt);
	pr_info("Freeing db");
	gatt_db_unref(server->db);
	//pr_info("destroy sem");
	//sem_destroy(&server->bt_sem);
}

static void att_disconnect_cb(int err, void *user_data)
{
	pr_info("++");
	pr_info("Device disconnected: %s\n", strerror(err));


	/* TODO: Report Disconnection Event */

	/* TODO: Whether we should  re-enable advertising */
	gatt_connection_event_t event = BT_GATT_DISCONNECT;

	if(gatt_server_cb) {
		gatt_server_cb->gatt_connection_event_cb(gatt_server->bd_addr, event);
	}

}

static int l2cap_le_att_accept(int sk)
{
	int nsk;
	socklen_t optlen;
	struct sockaddr_l2 addr;
	char ba[18];

	pr_info("++");

	if (sk < 0)
		goto fail;

	memset(&addr, 0, sizeof(addr));
	optlen = sizeof(addr);
	nsk = accept(sk, (struct sockaddr *)&addr, &optlen);
	if (nsk < 0) {
		pr_error("Accept failed");
		goto fail;
	}

	ba2str(&addr.l2_bdaddr, ba);
	memcpy(gatt_server->bd_addr, ba, AG_MAX_BDADDR_LEN);
	pr_info("Connect from %s", ba);

	gatt_connection_event_t event = BT_GATT_CONNECTION;

	if(gatt_server_cb) {
		gatt_server_cb->gatt_connection_event_cb(ba, event);
	}
	/* TODO: Report Connection event */

	return nsk;
fail:
	return -1;

}

static const char aw_gatt_dev_name[] = "AW_GATT_SERVER";

static void server_listen_cb(int fd, uint32_t events, void *user_data)
{
	struct server *server = user_data;
	int accept_fd;
	int mtu = 517;

	pr_info("++");
	if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
		mainloop_remove_fd(fd);
		return;
	}

	accept_fd = l2cap_le_att_accept(fd);
	if (accept_fd < 0) {
		pr_error("Accept error\n");
		return;
	}

	pr_info("accept_fd %d", accept_fd);
	server->fd = accept_fd;

	server->att = bt_att_new(accept_fd, false);
	if (!server->att) {
		pr_error("Failed to initialze ATT transport layer\n");
		goto fail;
	}

	if (!bt_att_set_close_on_unref(server->att, true)) {
		pr_error("Failed to set up ATT transport layer\n");
		goto fail;
	}

	if (!bt_att_register_disconnect(server->att, att_disconnect_cb, server,
					NULL)) {
		pr_error("Failed to set ATT disconnect handler\n");
		goto fail;
	}

	server->gatt = bt_gatt_server_new(server->db, server->att, mtu, 0);
	if (!server->gatt) {
		pr_error("Failed to create GATT server\n");
		goto fail;
	}

	if (verbose) {
		bt_att_set_debug(server->att, att_debug_cb, "att: ", NULL);
		bt_gatt_server_set_debug(server->gatt, gatt_debug_cb,
					 "server: ", NULL);
	}

fail:
	return;
}

static void gatt_thread_woken_cb(int fd, uint32_t events, void *user_data)
{
	ssize_t result;
	uint8_t c;

	pr_info("++");
	if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
		pr_error("Events error");
		mainloop_quit();
		return;
	}

	result = read(fd, &c, 1);
	if (result < 0) {
		pr_error("Read channel error");
		mainloop_quit();
		return;
	}
#define BTA_THREAD_EXIT_CODE 0x65
	if (result == 1 && c == BTA_THREAD_EXIT_CODE) {
		pr_info("Received bta thread exit code");
		mainloop_quit();
		return;
	}

	btgatt_run_command();

	return;
}

static int l2cap_le_att_listen(bdaddr_t * src, int sec, uint8_t src_type)
{
	int sk;
	struct sockaddr_l2 srcaddr;
	struct bt_security btsec;

	pr_info("++");
	sk = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
	if (sk < 0) {
		perror("Failed to create L2CAP socket");
		return -1;
	}

	/* Set up source address */
	memset(&srcaddr, 0, sizeof(srcaddr));
	srcaddr.l2_family = AF_BLUETOOTH;
	srcaddr.l2_cid = htobs(ATT_CID);
	srcaddr.l2_bdaddr_type = src_type;
	bacpy(&srcaddr.l2_bdaddr, src);

	if (bind(sk, (struct sockaddr *)&srcaddr, sizeof(srcaddr)) < 0) {
		pr_error("Failed to bind L2CAP socket");
		goto fail;
	}

	/* Set the security level */
	memset(&btsec, 0, sizeof(btsec));
	btsec.level = sec;
	if (setsockopt(sk, SOL_BLUETOOTH, BT_SECURITY, &btsec,
		       sizeof(btsec)) != 0) {
		pr_error("Failed to set L2CAP security level\n");
		goto fail;
	}

	if (listen(sk, 10) < 0) {
		pr_error("Listening on socket failed");
		goto fail;
	}

	pr_info("Started listening on ATT channel");

	return sk;

fail:
	close(sk);
	return -1;
}

static int gatt_cmd_handler(uint16_t opcode, int argc, void **argv)
{
	int result = -1;

	pr_info("opcode %d", opcode);

	switch (opcode) {
	case GATTCMD_OP_ADD_SERVICE:
		result = gatt_add_service(argc, argv);
		break;
	case GATTCMD_OP_ADD_CHAR:
		result = gatt_add_char(argc, argv);
		break;
	case GATTCMD_OP_ADD_DESC:
		result = gatt_add_desc(argc, argv);
		break;
	case GATTCMD_OP_START_SERVICE:
		result = gatt_start_service(argc, argv);
		break;
	case GATTCMD_OP_STOP_SERVICE:
		result = gatt_stop_service(argc, argv);
		break;
	case GATTCMD_OP_SEND_RSP:
		result = gatt_send_rsp(argc, argv);
		break;
	case GATTCMD_OP_SEND_IND:
		result = gatt_send_ind(argc, argv);
		break;
	case GATTCMD_OP_DEL_SERVICE:
		result = gatt_del_service(argc, argv);
		break;
	case GATTCMD_OP_UNREG_SERVICE:
		result = gatt_unreg_service(argc, argv);
		break;
	default:
		pr_error("Unknown bta command %04x", opcode);
		break;
	}

	return result;
}

static void *gatt_run(void *arg)
{
	int fd, ret;
	int sec = BT_SECURITY_LOW;
	uint8_t src_type = BDADDR_LE_PUBLIC;
	bdaddr_t src_addr;
	struct server *server = (struct server *)arg;

	pr_info("++");
	if (mainloop_running) {
		pr_error("mainloop has been alread running");
		goto done;
	}

	bacpy(&src_addr, BDADDR_ANY);
	fd = l2cap_le_att_listen(&src_addr, sec, src_type);

	ret = socketpair(PF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,
			 0, sockfds);
	if (ret == -1) {
		pr_error("Couldn't create paired sockets");
		return NULL;
	}

	pr_info("sockfds %d %d", sockfds[0], sockfds[1]);
	gatt_set_bta_mainloop_id(pthread_self());
	pr_info("tid %lu", (unsigned long)pthread_self());

	gatt_set_bta_cmd_fd(sockfds);
	gatt_bta_register_handler(gatt_cmd_handler);

	mainloop_init();

	netcfg_state = 1;
	if (mainloop_add_fd(fd, EPOLLIN, server_listen_cb, server, NULL) < 0) {
		pr_error("Failed to add listen socket\n");
		close(fd);
		netcfg_state = 0;
		return NULL;
	}

	if (mainloop_add_fd(sockfds[0], EPOLLIN, gatt_thread_woken_cb,
			    NULL, NULL) < 0) {
		pr_error("Failed to install command handler\n");
		netcfg_state = 0;
		return NULL;
	}
	pr_info("wake up main thread");
	sem_post(&server->bt_sem);
	if (server->device_name) {
		free(server->device_name);
		server->device_name = NULL;
	}

	mainloop_running = 1;
	mainloop_run();
	netcfg_state = 0;

	mainloop_running = 0;
	close(sockfds[0]);
	close(sockfds[1]);
	sockfds[0] = -1;
	sockfds[1] = -1;
	close(fd);

	bt_gatt_server_unref(server->gatt);
done:
	gatt_db_unref(server->db);

	free(server);

	return NULL;
}

int gatt_thread_create(void)
{
	struct server *server;
	struct gatt_db *db;
	int ret;
	int name_len = strlen(aw_gatt_dev_name);

	pr_info("++");
	server = new0(struct server, 1);

	if (!server)
		return -1;
	gatt_server = server;

	db = gatt_db_new();
	if (!db) {
		pr_error("failed to create GATT database");
		ret = -1;
		goto free_server;
	}

	server->db = db;

	ret = sem_init(&server->bt_sem, 0, 0);
	if (ret != 0) {
		pr_error("sem_init failed");
		goto free_db;
	}

	server->name_len = name_len+1;
	server->device_name = malloc(name_len+1);
	if (!server->device_name) {
		pr_error("failed to malloc for server device name");
		goto free_db;
	}
	memcpy(server->device_name, aw_gatt_dev_name, name_len);
	server->device_name[name_len] = '\0';

	ret = pthread_create(&gatt_tid, NULL, gatt_run, server);
	if (ret) {
		pr_error("pthread_create error\n");
		goto sem_destroy;
	}
	sem_wait(&server->bt_sem);

	/* TODO: Gatt Initialization Completes */

	pr_info("stack running");

	return 0;

sem_destroy:
	free(server->device_name);
	sem_destroy(&server->bt_sem);
free_db:
	free(db);
free_server:
	free(server);

	return ret;
}

void gatt_thread_quit(void)
{
	uint8_t c;
	int result;

	pr_info("++");
	pr_debug("Write exit code");
#define BTA_THREAD_EXIT_CODE    0x65
	c = BTA_THREAD_EXIT_CODE;
	result = write(sockfds[1], &c, 1);
	if (result < 0)
		pr_error("Couldn't send exit code, %s", strerror(errno));
	pthread_join(gatt_tid, NULL);
}

/*
 * argv[0]: uuid;
 * argv[1]: is_primary;
 * argv[2]: num of handles
 */
static int gatt_add_service(int argc, void **argv)
{
	struct gatt_db_attribute *service;
	bool primary;
	uint16_t num_handle;
	bt_uuid_t uuid;
	uint128_t value;

	pr_info("++");
	pr_info("service uuid: %s", (char *)argv[0]);
	bt_string_to_uuid(&uuid, argv[0]);
	primary = (bool) PTR_TO_UINT(argv[1]);
	num_handle = (uint16_t) PTR_TO_UINT(argv[2]);

	print_uuid(&uuid);
	pr_info("num_handle = %d, primary: %s", num_handle,
		primary ? "yes" : "no");
	service = gatt_db_add_service(gatt_server->db, &uuid, primary, num_handle);
	if (!service) {
		pr_error("add service failed.");
		return -1;
	}

	uint16_t start_handle;

	gatt_db_attribute_get_service_handles(service, &start_handle, NULL);

	gatt_add_svc_msg_t server_msg;

	server_msg.svc_handle = start_handle;
	server_msg.num_handle = num_handle;

	if(gatt_server_cb) {
		gatt_server_cb->gatt_add_svc_cb(&server_msg);
	}
	/* TODO: Report the register service information
	 * Parameters:
	 *   uuid
	 *   is primary
	 *   start handle
	 *
	 * */

	return 0;
}

static void attrib_char_read_cb(struct gatt_db_attribute *attrib,
			   unsigned int id, uint16_t offset,
			   uint8_t opcode, struct bt_att *att, void *user_data)
{
	uint16_t attr_handle = gatt_db_attribute_get_handle(attrib);
	bool is_long_read;
	uint32_t trans_id;

	pr_info("++");
	printf("%s:test id:%d\n", __func__, id);
	gatt_server->id = id;
	gatt_server->in_read_req = true;
	gatt_server->curr_opcode = opcode;

	trans_id = gatt_server->trans_id++;
	gatt_server->cur_trans_id = trans_id;

	if (opcode == BT_ATT_OP_READ_REQ)
		is_long_read = false;
	else if (opcode == BT_ATT_OP_READ_BLOB_REQ)
		is_long_read = true;
	else {
		is_long_read = false;
		pr_info("Unexpected opcode for read req");
	}

	pr_info("read_cb called");


	/* TODO: Call upper that there is a read request from remote
	 * Parameters:
	 *   trans_id: It will be used in gatt_send_rsp()
	 *   attr_handle
	 *   offset
	 *
	 * gatt_send_rsp() will be called later
	 *
	 * */
	gatt_char_read_req_t ChrMsg;
	ChrMsg.attr_handle = attr_handle;
	ChrMsg.is_blob_req = is_long_read;
	ChrMsg.offset = offset;
//	ChrMsg.trans_id = trans_id;
	ChrMsg.trans_id = id;
	if(gatt_server_cb) {
		gatt_server_cb->gatt_char_read_req_cb(&ChrMsg);
	}

}

static void attrib_char_write_cb(struct gatt_db_attribute *attrib,
			    unsigned int id, uint16_t offset,
			    const uint8_t * value, size_t len,
			    uint8_t opcode, struct bt_att *att, void *user_data)
{
	uint16_t attr_handle;
	bool need_rsp;
	uint32_t trans_id;

	pr_info("++");
	printf("%s:test id:%d\n", __func__, id);

	gatt_server->id = id;
	gatt_server->in_read_req = false;
	gatt_server->curr_opcode = opcode;

	attr_handle = gatt_db_attribute_get_handle(attrib);

	trans_id = gatt_server->trans_id++;
	gatt_server->cur_trans_id = trans_id;

	if (opcode == BT_ATT_OP_WRITE_REQ)
		need_rsp = true;
	else if (opcode == BT_ATT_OP_WRITE_CMD)
		need_rsp = false;
	else
		pr_info("Unexpected opcode %d", opcode);

//	memcpy(bt_gatts_req_write.value, value, len);

	pr_info("write_cb called");

//	if (opcode == BT_ATT_OP_WRITE_CMD || opcode == BT_ATT_OP_WRITE_REQ)
	if (opcode == BT_ATT_OP_WRITE_CMD)
		gatt_db_attribute_write_result(attrib, id, 0);

	/* TODO: Notify upper layer that there is a write req/cmd
	 * Parameters:
	 *   trans_id
	 *   attr_handle
	 *   offset
	 *   value: Must be copied to one buffer
	 *   len
	 * */

	gatt_char_write_req_t ChrWriteMsg;
	memcpy(ChrWriteMsg.value,value,len);
	ChrWriteMsg.value_len = len;
	ChrWriteMsg.attr_handle = attr_handle;
//	ChrWriteMsg.trans_id = trans_id;
	ChrWriteMsg.trans_id = id;
	ChrWriteMsg.offset = offset;
	ChrWriteMsg.need_rsp = need_rsp;
	if(gatt_server_cb) {
		gatt_server_cb->gatt_char_write_req_cb(&ChrWriteMsg);
	}

}

static void attrib_desc_read_cb(struct gatt_db_attribute *attrib,
			   unsigned int id, uint16_t offset,
			   uint8_t opcode, struct bt_att *att, void *user_data)
{
	uint16_t attr_handle = gatt_db_attribute_get_handle(attrib);
	bool is_long_read;
	uint32_t trans_id;

	pr_info("++");
	gatt_server->id = id;
	gatt_server->in_read_req = true;
	gatt_server->curr_opcode = opcode;

	trans_id = gatt_server->trans_id++;
	gatt_server->cur_trans_id = trans_id;

	if (opcode == BT_ATT_OP_READ_REQ)
		is_long_read = false;
	else if (opcode == BT_ATT_OP_READ_BLOB_REQ)
		is_long_read = true;
	else {
		is_long_read = false;
		pr_info("Unexpected opcode for read req");
	}

	pr_info("read_cb called");


	/* TODO: Call upper that there is a read request from remote
	 * Parameters:
	 *   trans_id: It will be used in gatt_send_rsp()
	 *   attr_handle
	 *   offset
	 *
	 * gatt_send_rsp() will be called later
	 *
	 * */
	gatt_desc_read_req_t DescMsg;
	DescMsg.attr_handle = attr_handle;
	DescMsg.is_blob_req = is_long_read;
	DescMsg.offset = offset;
//	DescMsg.trans_id = trans_id;
	DescMsg.trans_id = id;
	if(gatt_server_cb) {
		gatt_server_cb->gatt_desc_read_req_cb(&DescMsg);
	}

}

static void attrib_desc_write_cb(struct gatt_db_attribute *attrib,
			    unsigned int id, uint16_t offset,
			    const uint8_t * value, size_t len,
			    uint8_t opcode, struct bt_att *att, void *user_data)
{
	uint16_t attr_handle;
	bool need_rsp;
	uint32_t trans_id;

	pr_info("++");

	gatt_server->id = id;
	gatt_server->in_read_req = false;
	gatt_server->curr_opcode = opcode;

	attr_handle = gatt_db_attribute_get_handle(attrib);

	trans_id = gatt_server->trans_id++;
	gatt_server->cur_trans_id = trans_id;

	if (opcode == BT_ATT_OP_WRITE_REQ)
		need_rsp = true;
	else if (opcode == BT_ATT_OP_WRITE_CMD)
		need_rsp = false;
	else
		pr_info("Unexpected opcode %d", opcode);

//	memcpy(bt_gatts_req_write.value, value, len);

	pr_info("write_cb called");

	/* TODO: Notify upper layer that there is a write req/cmd
	 * Parameters:
	 *   trans_id
	 *   attr_handle
	 *   offset
	 *   value: Must be copied to one buffer
	 *   len
	 * */

	if (opcode == BT_ATT_OP_WRITE_CMD || opcode == BT_ATT_OP_WRITE_REQ)
		gatt_db_attribute_write_result(attrib, id, 0);

	gatt_desc_write_req_t DescWriteMsg;
	memcpy(DescWriteMsg.value,value,len);
	DescWriteMsg.value_len = len;
	DescWriteMsg.attr_handle = attr_handle;
//	DescWriteMsg.trans_id = trans_id;
	DescWriteMsg.trans_id = id;
	DescWriteMsg.offset = offset;
	DescWriteMsg.need_rsp = need_rsp;

	if(gatt_server_cb) {
		gatt_server_cb->gatt_desc_write_req_cb(&DescWriteMsg);
	}

}

static void find_matching_service(struct gatt_db_attribute *service,
				  void *user_data)
{
	struct service_data *service_data = user_data;
	uint16_t start_handle;

	pr_info("++");
	if (service_data->found)
		return;

	gatt_db_attribute_get_service_handles(service, &start_handle, NULL);

	if (service_data->handle == start_handle) {
		service_data->found = true;
		service_data->match = service;
	}
}

static struct gatt_db_attribute *find_service_by_handle(uint16_t handle,
							struct gatt_db *db)
{
	struct service_data service_data;

	pr_info("++");
	service_data.handle = handle;
	service_data.found = false;
	gatt_db_foreach_service(db, NULL, find_matching_service, &service_data);

	if (service_data.found)
		return service_data.match;
	return NULL;
}

/* argv[0]: service_handle;
 * argv[1]: uuid;
 * argv[2]: prop;
 * argv[3]: perm
 */
static int gatt_add_char(int argc, void **argv)
{
	struct gatt_db_attribute *service;
	struct gatt_db_attribute *character;
	bt_uuid_t uuid;
	uint128_t value;
	uint16_t service_handle;
	uint32_t perm;
	uint8_t prop;

	pr_info("++");

	service_handle = (uint16_t) (uint32_t) (PTR_TO_UINT(argv[0]));
	bt_string_to_uuid(&uuid, argv[1]);
	print_uuid(&uuid);
	prop = (uint8_t) (uint32_t) PTR_TO_UINT(argv[2]);
	perm = (uint32_t) PTR_TO_UINT(argv[3]);
	pr_info("server handle : %d , prop: %d ,perm :%d\n",service_handle,
			prop,perm);

	service = find_service_by_handle(service_handle, gatt_server->db);
	if (!service) {
		pr_info("no service found by %d", service_handle);
		return -1;
	}
	character = gatt_db_service_add_characteristic(service, &uuid,
						       perm, prop,
						       attrib_char_read_cb,
						       attrib_char_write_cb,
						       gatt_server);

	/* gatt_db_service_set_active(service, true); */
	uint16_t char_handle = gatt_db_attribute_get_handle(character);

	pr_info("Permission of characteristic at handle %u is 0x%08x",
		char_handle, perm);

	/* TODO: Report Characteristic information
	 * Parameters:
	 *   service handle
	 *   characteristic handle
	 *
	 *
	 * */
	gatt_add_char_msg_t AddChrMsg;

	AddChrMsg.char_handle = char_handle;
	AddChrMsg.uuid = argv[1];
	if(gatt_server_cb) {
		gatt_server_cb->gatt_add_char_cb(&AddChrMsg);
	}

	return 0;
}

/* argv[0]: service_handle;
 * argv[1]: uuid;
 * argv[2]: permission
 */
static int gatt_add_desc(int argc, void **argv)
{
	struct gatt_db_attribute *service;
	struct gatt_db_attribute *desc;
	bt_uuid_t uuid;
	uint128_t value;
	uint16_t service_handle;
	uint32_t perm;

	pr_info("++");

	service_handle = (uint16_t) (uint32_t) (PTR_TO_UINT(argv[0]));
	bt_string_to_uuid(&uuid, argv[1]);
	print_uuid(&uuid);
	perm = (uint32_t) PTR_TO_UINT(argv[2]);
	pr_info("server handle : %d ,perm :%d\n",service_handle,
			perm);
	service = find_service_by_handle(service_handle, gatt_server->db);
	if (!service) {
		pr_info("No service found by %d", service_handle);
		return -1;
	}
	desc = gatt_db_service_add_descriptor(service, &uuid,
					      perm, attrib_desc_read_cb,
					      attrib_desc_write_cb, gatt_server);

	uint16_t desc_handle = gatt_db_attribute_get_handle(desc);


	pr_info("Permission of desc at handle %u is %08x", desc_handle, perm);
	/* TODO: Call callback to report Descriptor information
	 * Parameters
	 *   service handle
	 *   desc_handle
	 *   [optional] desc uuid
	 *
	 * */
	gatt_add_desc_msg_t AddDescMsg;

	AddDescMsg.desc_handle = desc_handle;

	if(gatt_server_cb) {
		gatt_server_cb->gatt_add_desc_cb(&AddDescMsg);
	}

	return 0;
}

/* argv[0]: service_handle;
 */
static int gatt_start_service(int argc, void **argv)
{
	struct gatt_db_attribute *service;
	uint16_t service_handle;

	pr_info("++");
	service_handle = (uint16_t) (uint32_t) (PTR_TO_UINT(argv[0]));

	pr_info("server handle : %d\n",service_handle);
	/* look for service by service handle */
	service = find_service_by_handle(service_handle, gatt_server->db);
	if (!service) {
		pr_info("No service found by %d", service_handle);
		return -1;
	}

	gatt_db_service_set_active(service, true);

	/* TODO: Call callback to report service start success */

	return 0;
}

/* argv[0]: service_handle;
 */
static int gatt_stop_service(int argc, void **argv)
{
	struct gatt_db_attribute *service;
	uint16_t service_handle;

	pr_info("++");

	/* Don't enable advertising if service is stopped */
	netcfg_state = 0;

	service_handle = (uint16_t) (uint32_t) (PTR_TO_UINT(argv[0]));
	service = find_service_by_handle(service_handle, gatt_server->db);
	if (!service) {
		pr_info("no service found by %d", service_handle);
		return -1;
	}
	gatt_db_service_set_active(service, false);

	/* TODO: Call callback to report service stopped */

	return 0;
}

int gatt_send_write_rsp(unsigned int id, unsigned int handle, unsigned int status)
{
	struct gatt_db_attribute *attrib;
	bool ret = false;

	attrib = gatt_db_get_attribute(gatt_server->db, handle);
	if (!attrib) {
		pr_error("couldn't find attribute for handle: %d", handle);
		return -1;
	}

	ret = gatt_db_attribute_write_result(attrib, id, status);

	if (ret)
		return 0;
	else {
		pr_error("write rsp failed for trans_id: %d", id);
		return -1;
	}
}

/*
 * argv[0]: trans_id;
 * argv[1]: status
 * argv[2]: handle;
 * argv[3]: p_value;
 * argv[4]: value_len;
 * argv[5]: auth_req
 */
static int gatt_send_rsp(int argc, void **argv)
{
	uint32_t trans_id, status, handle, value_len, auth_req;
	char *p_value;
	uint8_t opcode;
	struct gatt_db_attribute *attrib;

	pr_info("++");

	trans_id = PTR_TO_UINT(argv[0]);
	status = PTR_TO_UINT(argv[1]);
	handle = PTR_TO_UINT(argv[2]);
	p_value = argv[3];
	value_len = PTR_TO_UINT(argv[4]);
	auth_req = PTR_TO_UINT(argv[5]);

	pr_info("send rsp for handle: %u", handle);
	pr_info("response status: %u", status);

	opcode = gatt_server->curr_opcode + 1;
	attrib = gatt_db_get_attribute(gatt_server->db, handle);

/*
	if (trans_id != gatt_server->cur_trans_id) {
		pr_warning("Wrong trans id: trans_id = %d, cur_trans_id = %d",
			   trans_id, gatt_server->cur_trans_id);
		return -1;
	}
*/

	if (opcode == BT_ATT_OP_READ_RSP) {
		pr_info("read rsp");
//		gatt_db_attribute_read_result(attrib,
//					      gatt_server->id, 0,
//					      (uint8_t *) p_value, value_len);
		gatt_db_attribute_read_result(attrib,
					      trans_id, 0,
					      (uint8_t *) p_value, value_len);
	} else if (opcode == BT_ATT_OP_READ_BLOB_RSP) {
		pr_info("read blob rsp");
//		gatt_db_attribute_read_result(attrib,
//					      gatt_server->id, 0,
//					      (uint8_t *) p_value, value_len);
		gatt_db_attribute_read_result(attrib,
					      trans_id, 0,
					      (uint8_t *) p_value, value_len);
	} else if (opcode == BT_ATT_OP_WRITE_RSP) {
		pr_info("write rsp");
//		gatt_db_attribute_write_result(attrib, gatt_server->id, 0);
		gatt_db_attribute_write_result(attrib, trans_id, 0);

	} else {
		pr_info("Unexpected opcode for response pdu %u", opcode);
	}

	return 0;
}

static void conf_cb(void *user_data)
{
	pr_info("++");
	pr_info("Received confirmation");
	if(gatt_server_cb) {
		gatt_server_cb->gatt_send_indication_cb(user_data);
	}
}

/*
 * argv[0]: handle;
 * argv[1]: fg_confirm;
 * argv[2]: p_value;
 * argv[3]: value_len,
 */
static int gatt_send_ind(int argc, void **argv)
{
	uint32_t handle, value_len, fg_confirm;
	char *p_value;

	pr_info("++");

	handle = PTR_TO_UINT(argv[0]);
	fg_confirm = PTR_TO_UINT(argv[1]);
	p_value = argv[2];
	value_len = PTR_TO_UINT(argv[3]);

	pr_info("send %s for handle: %d",
		fg_confirm ? "indication" : "notification", handle);

	if (fg_confirm) {
		if (!bt_gatt_server_send_indication(gatt_server->gatt, handle,
						    (uint8_t *) p_value,
						    value_len, conf_cb, NULL,
						    NULL))
			pr_error("Failed to initiate indication");
	} else if (!bt_gatt_server_send_notification(gatt_server->gatt, handle,
						     (uint8_t *) p_value,
						     value_len))
		pr_error("Failed to initiate notification");

	return 0;
}

/* argv[0]: service handle */
static int gatt_del_service(int argc, void **argv)
{
	struct gatt_db_attribute *service;
	uint16_t service_handle;

	pr_info("++");

	service_handle = (uint16_t) (uint32_t) (PTR_TO_UINT(argv[0]));
	service = find_service_by_handle(service_handle, gatt_server->db);
	if (!service) {
		pr_info("No service found by %d", service_handle);
		return -1;
	}

	gatt_db_remove_service(gatt_server->db, service);

	/* TODO: Call callback to report service deleted success */

	return 0;
}

static int gatt_unreg_service(int argc, void **argv)
{
	/* The following function callings will be done when mainloop exit */
	/* bt_gatt_server_unref(gatt_server->gatt);
	 * gatt_db_unref(gatt_server->db);
	 */
	if (!mainloop_running) {
		pr_info("mainloop is not running");
		return 0;
	}
	gatt_thread_quit();
	return 0;
}
