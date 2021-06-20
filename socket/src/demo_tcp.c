#include <stdio.h>
#include "global_types.h"
#include "oc_net.h"
#include "sockets.h"
#define SERV_IP    "182.61.41.198"
#define SERV_PORT  6600

static OSTaskRef demoWorkerRef;
static BOOL bRun = FALSE;

void customer_app_tcp_demo(void)
{
	struct sockaddr_in servaddr;
	int sock_fd;
	int bytes =0;
	int res=-1;
	char sendbuf[64]={0};
	char recvbuf[64]={0};
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);  // server port
    servaddr.sin_addr.s_addr = inet_addr(SERV_IP);  // server ip
	 if(connect(sock_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        CPUartLogPrintf("connect faild!!");
        return;
    }
    CPUartLogPrintf("connect: Success\n");
	strcpy(sendbuf,"tcp test!");

	while (bRun)
	{
		bytes = send(sock_fd, sendbuf, strlen(sendbuf), 0);
		bytes = recv(sock_fd, recvbuf, sizeof(recvbuf), 0);
		CPUartLogPrintf("%s: recvbuf:%s,bytes:%d", __func__,recvbuf,bytes);
		OSATaskSleep(2000);
	}
}

void customer_app_udp_demo(void)
{
	struct sockaddr_in servaddr;
	int sock_fd;
	int bytes =0;
	int res=-1;
	char sendbuf[64]={0};
	char recvbuf[64]={0};
	sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);  // server port
    servaddr.sin_addr.s_addr = inet_addr(SERV_IP);  // server ip
	 if(connect(sock_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        CPUartLogPrintf("connect faild!!");
        return;
    }
    CPUartLogPrintf("connect: Success\n");
	strcpy(sendbuf,"udp test!");
	bytes = sendto(sock_fd,sendbuf, strlen(sendbuf), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
	struct sockaddr_in server;
	socklen_t addrlen;
	bytes = recvfrom(sock_fd, recvbuf, sizeof(recvbuf), 0, &server, &addrlen);
	CPUartLogPrintf("%s: recvbuf:%s\n", __func__,recvbuf);
}

