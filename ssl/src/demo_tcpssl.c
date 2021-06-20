#include <stdio.h>
#include "global_types.h"
#include "oc_net.h"
#include "sockets.h"
#include "oc_ssl.h"
#define SERV_IP    "182.148.114.87"
#define SERV_PORT  2210
static OSTaskRef demoWorkerRef;
static BOOL bRun = FALSE;

void tcpssl_demo(void)
{
	struct sockaddr_in servaddr;
	int sock_fd;
	int bytes =0;
	int res=-1;
	void* sslclient;
	char sendbuf[64]={0};
	char recvbuf[64]={0};
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);  // server port
    servaddr.sin_addr.s_addr = inet_addr(SERV_IP);  // server ip
	 if(connect(sock_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        OC_UART_LOG_Printf("connect faild!!");
        return;
    }
	sslclient = OC_Ssl_Open(sock_fd,0);
    OC_UART_LOG_Printf("OC_Ssl_Init:%x\n",sslclient);
	strcpy(sendbuf,"tcp test1!");
	bytes = OC_Ssl_Write(sslclient, sendbuf, strlen(sendbuf));
	OC_UART_LOG_Printf("%s: OC_Ssl_Write:%d", __func__,bytes);
	bytes = OC_Ssl_Read(sslclient, recvbuf, sizeof(recvbuf));
	OC_UART_LOG_Printf("%s: recvbuf:%s,bytes:%d", __func__,recvbuf,bytes);
	close(sock_fd);
	OC_Ssl_Close(sslclient);
}

static void netopen_worker_thread(void * argv)
{
	int bRet,regState,netStatus = 0;
	int dataLen = 0;
	char strDataLen[32]={0};
	char postData[256]={0};
	while(bRun)
	{
		OC_NW_GetRegState(&regState);
		if(regState)
		{
			break;
		}
		OSATaskSleep(200);//sleep 1s
	}
	OC_NetOpen();
	OSATaskSleep(100);
	while(bRun)
	{
		bRet = OC_GetNetStatus();
		if(bRet)
		{
			break;//Open  network Success!
		}
		OSATaskSleep(100);
		
	}
	OC_UART_LOG_Printf("%s:Open  network Success!\n", __func__);
	tcpssl_demo();
}

void customer_app_tcpssl_demo(void)
{
	void *TaskStack;
	TaskStack=malloc(4096);
	if(TaskStack == NULL)
	{
		return;
	}
	bRun = TRUE;
	if(OSATaskCreate(&demoWorkerRef,
	                 TaskStack,
	                 4096,80,(char*)"demoNetOpenTask",
	                 netopen_worker_thread, NULL) != 0)
	{
		return;
	}
}




