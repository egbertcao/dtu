#include <stdio.h>
#include "global_types.h"
#include "oc_net.h"
#include "sockets.h"
#include "dtu_common.h"
#include "oc_uart.h"

#define SERV_IP    "182.61.41.198"
#define SERV_PORT  6601

extern dtu_config_t g_dtu_config;

static int sock_fd;
static struct sockaddr_in servaddr;
static OSTaskRef demoWorkerRef;
static BOOL bRun = FALSE;

void dtu_udp_send(char *message)
{
	int bytes = sendto(sock_fd, message, strlen(message), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
	printf("bytes = %d", bytes);
}

void udp_worker_thread(void)
{
	int sock_fd;
	int bytes =0;
	int res=-1;
	char sendbuf[64]={0};
	char recvbuf[64]={0};
	sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(g_dtu_config.currentsocket.udpport);  // server port
    servaddr.sin_addr.s_addr = inet_addr(g_dtu_config.currentsocket.udpaddress);  // server ip
	 if(connect(sock_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        OC_UART_LOG_Printf("connect faild!!");
        return;
    }
    OC_UART_LOG_Printf("connect: Success\n");
	strcpy(sendbuf,"udp test!");
	bytes = sendto(sock_fd,sendbuf, strlen(sendbuf), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
	struct sockaddr_in server;
	socklen_t addrlen;
	bytes = recvfrom(sock_fd, recvbuf, sizeof(recvbuf), 0, &server, &addrlen);
	OC_UART_LOG_Printf("%s: recvbuf:%s\n", __func__,recvbuf);
}

void customer_app_udp_start(void)
{
	OC_UART_LOG_Printf("[%s] udp Start!\n", __func__);
	void *TaskStack;
	TaskStack=malloc(4096);
	if(TaskStack == NULL)
	{
		return;
	}
	bRun = TRUE;
	if(OSATaskCreate(&demoWorkerRef,
	                 TaskStack,
	                 4096,80,(char*)"demoMqttTask",
	                 udp_worker_thread, NULL) != 0)
	{
		return;
	}
}

