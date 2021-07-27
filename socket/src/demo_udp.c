#include <stdio.h>
#include "global_types.h"
#include "oc_net.h"
#include "sockets.h"
#include "dtu_common.h"
#include "oc_uart.h"

static OSTaskRef udpWorkerRef;
static OSTaskRef udpMonitorRef;
static BOOL bWorkerRun = FALSE;
static BOOL bMonitorRun = FALSE;
extern dtu_config_t g_dtu_config;

static BOOL UDP_CONNECTION = FALSE;

void dtu_udp_send(char *message)
{
	int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(8001);  // server port
	servaddr.sin_addr.s_addr = inet_addr("182.61.41.198");  // server ip
	int nSize = sizeof(servaddr);
	OC_UART_LOG_Printf("%s: sendto\n", __func__);
	int bytes = sendto(sock_fd, message, strlen(message), 0, (struct sockaddr*)&servaddr, nSize);
	printf("bytes = %d", bytes);
	closesocket(sock_fd);
}

// 只接收数据
void udp_worker_thread(void)
{
	OC_UART_LOG_Printf("[%s] udp Worker Start!\n", __func__);
	while (bWorkerRun)
	{
	
		int netstatus = OC_GetNetStatus();
		if(netstatus == 1) { 
			OC_UART_LOG_Printf("[%s] recving...\n", __func__);
			char recvbuf[64]={0};
			int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
			struct sockaddr_in servaddr;	
			memset(&servaddr, 0, sizeof(servaddr));
			servaddr.sin_family = AF_INET;
			servaddr.sin_port = htons(8001);  // server port
			servaddr.sin_addr.s_addr = inet_addr("182.61.41.198");  // server ip
			int nSize = sizeof(servaddr);
			struct timeval tv;
			tv.tv_sec = 5;
			tv.tv_usec = 0;
			if (setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
				OC_UART_LOG_Printf("[%s] error\n", __func__);
				continue;
			}
			int bytes = recvfrom(sock_fd, recvbuf, sizeof(recvbuf), 0, &servaddr, &nSize);
			if(bytes > 0){
				OC_UART_LOG_Printf("[%s] recvbuf:%s\n", __func__,recvbuf);
			}
			closesocket(sock_fd);
		}
		else{
			OSATaskSleep(2000);
		}
	}
	
}

void customer_app_udp_start(void)
{
	void *udpWorkTaskStack;
	udpWorkTaskStack=malloc(4096);
	if(udpWorkTaskStack == NULL)
	{
		return;
	}
	bWorkerRun = TRUE;
	if(OSATaskCreate(&udpWorkerRef,
	                 udpWorkTaskStack,
	                 4096,80,(char*)"udpWorkerTask",
	                 udp_worker_thread, NULL) != 0)
	{
		return;
	}
}
