#include <stdio.h>
#include "global_types.h"
#include "oc_at.h"
#include "osa.h"

static OSTaskRef demoWorkerRef;

static void at_urc_cb(char *urc,int urclen)
{
	OC_UART_LOG_Printf("urclen:%d,urc:%s\n",urclen,urc);
}


static void at_worker_thread(void * argv)
{
	OC_AT_URCRegister(at_urc_cb);
	 int ret;
	 char atcmd[64];
     char respBuf[200];
	 while(1)
	 {
	 	OSATaskSleep(10*200);
	 	memset(atcmd, 0, sizeof(atcmd));
	 	memset(respBuf, 0, sizeof(respBuf));
	 	sprintf(atcmd, "AT+CPIN?\r");
	 	ret = OC_AT_Cmd(atcmd,respBuf, sizeof(respBuf));
	 	OC_UART_LOG_Printf("cmd:%s, respBuf:%s\n", atcmd, respBuf);
	 	memset(atcmd, 0, sizeof(atcmd));
	 	memset(respBuf, 0, sizeof(respBuf));
	 	sprintf(atcmd, "AT+CEREG?\r");
	 	ret = OC_AT_Cmd(atcmd,respBuf, sizeof(respBuf));
	 	OC_UART_LOG_Printf("cmd:%s, respBuf:%s\n", atcmd, respBuf);
     	memset(atcmd, 0, sizeof(atcmd));
	 	memset(respBuf, 0, sizeof(respBuf));
		sprintf(atcmd, "AT+NETOPEN\r");
	 	ret = OC_AT_Cmd(atcmd,respBuf, sizeof(respBuf));
	 	OC_UART_LOG_Printf("cmd:%s, respBuf:%s\n", atcmd, respBuf);
	 }
}

void customer_app_at_demo(void)
{
	void *TaskStack;
	TaskStack=malloc(4096);
	if(TaskStack == NULL)
	{
		return;
	}
	if(OSATaskCreate(&demoWorkerRef,
	                 TaskStack,
	                 4096,200,(char*)"demoAtTask",
	                 at_worker_thread, NULL) != 0)
	{
		return;
	}
}



