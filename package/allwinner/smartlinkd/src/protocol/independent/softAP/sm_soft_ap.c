#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <net/if.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sm_link_manager.h>

#define IP_SIZE 16

#define MSG_LEN_MX 256
#define CMD_LEN_MX 128

#define PORT_DEFAULT 8066

static int f_send_finished = 0;

int thread_handle(char *buf, int length)
{
    int ret;

    ret = strcmp(buf,"OK");
    if(0 == ret)
    {
	f_send_finished = 1;
	printf("thread_handle of softap: recieve OK from server!\n");
	return 0;
//	return THREAD_EXIT;
    }
	return -1;
//    return THREAD_CONTINUE;
}
struct soft_ap_resource {
	int fd;
	char *ssid;
	char *pwd;
};

static struct soft_ap_resource resource = {
	.fd = -1,
	.ssid = NULL,
	.pwd = NULL,
};

int soft_ap_protocol_resource_free(void *arg)
{
	if(resource.fd >0)
		close(resource.fd);

	system("softap_down");   //close softap mode
//	usleep(5*1000000);
	if(resource.ssid)
		free(resource.ssid);
	if(resource.pwd)
		free(resource.pwd);
}

int soft_ap_protocol(void *arg)
{
	int port = PORT_DEFAULT;
	bool is_receive = false;
	struct net_info netInfo;
	struct pro_worker *_worker = (struct pro_worker *)arg;
	struct pro_feedback info;

	info.force_quit_sm = false;
	info.protocol = _worker->type;

	 _worker->enable = true;



	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	printf("*****port:%d *****\n", port);

	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	int sock;

	_worker->free_cb=soft_ap_protocol_resource_free;

	if((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		printf("*****socket error!*****\n");
		printf("*****%s *****\n",strerror(errno));
		goto end;
	}else{
		printf("*****socket success.*****\n");
		resource.fd = sock;
	}

	if(bind(sock, (struct sockaddr *)&addr, sizeof(addr))< 0){
		printf("*****bind error!*****\n");
		printf("*****%s *****\n", strerror(errno));
		goto end;
	}else{
		printf("bind success.\n");
	}

	//receive usr app ssid information
	char buff[512];
	struct sockaddr_in clientAddr;
	int len = sizeof(clientAddr);
	int n;

	system("softap_down");
	usleep(1000000);

	system("softap_up aw_smartlink_softap open broadcast"); //softap up

	while(_worker->enable){

		printf("recvfrom...\n");

		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

		n = recvfrom(sock, buff, 511, 0, (struct sockaddr*)&clientAddr, &len);
		printf("%s *****\n", buff);
//		printf("*****addr: %s -port: %u says: %s *****\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), buff);
		if(n>0){
			printf("recvfrom success.\n");
			break;
		}
	}

	//feedback message to usr app
	char buff_confirm[3];
	strcpy(buff_confirm,"OK");
	while(_worker->enable) {

		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

		n = sendto(sock, buff_confirm, n, 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));
//		printf("*****addr: %s -port: %u says: %s *****\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), buff_confirm);
		if(n>0){
			printf("sendto success.\n");
			//usleep(10*1000000);
			break;
		}
	}


	//Analyze usr app information
	char* smartlink_softAP_ssid;// = "12345678";
	char* smartlink_softAP_password;// = "12345678";

	char *p =NULL;

	int ssid_len = strstr(buff, "::Password::")-buff-8;
	printf("ssid_len  %d\n",ssid_len);
	smartlink_softAP_ssid = (char *)malloc(ssid_len+1);
	resource.ssid = smartlink_softAP_ssid;
	if((p = strstr(buff, "::SSID::")) != NULL){
		p += strlen("::SSID::");

		if(*p){
			if(strstr(p, "::Password::") != NULL){
				memset((void*)smartlink_softAP_ssid,'\0',ssid_len+1);
				strncpy(smartlink_softAP_ssid, p, ssid_len);
			}
		}

	}

	printf("%s\n",smartlink_softAP_ssid);

	int password_len = strstr(buff, "::End::") - strstr(buff, "::Password::") - 12;
	printf("password_len  %d\n",password_len);

	smartlink_softAP_password = (char *)malloc(password_len+1);

	resource.pwd = smartlink_softAP_password;

	if((p = strstr(buff, "::Password::")) != NULL){
		p += strlen("::Password::");

		if(*p){
			if(strstr(p, "::End::") != NULL){
				memset((void*)smartlink_softAP_password,'\0',password_len+1);
				strncpy(smartlink_softAP_password, p, password_len);
			}
		}

	}
	printf("%s\n",smartlink_softAP_password);

	is_receive = true;
	strcpy(netInfo.ssid,smartlink_softAP_ssid);
	strcpy(netInfo.password,smartlink_softAP_password);

finish:
end:
	_worker->enable = false;
	_worker->cb(&info,is_receive,&netInfo);
	return 0;
}
