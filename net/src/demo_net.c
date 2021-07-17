#include <stdio.h>
#include "global_types.h"
#include "oc_net.h"
#include "osa.h"
static OSTaskRef netMonitorRef;
static BOOL NetMonitorRun = FALSE;

int dtu_netopen_worker()
{
	int bRet,regState,netStatus = 0;
	do{
		OC_NW_GetRegState(&regState);
		OSATaskSleep(200);//sleep 1s
	}while(!regState);
	
	OC_NetOpen();
	OSATaskSleep(200);
	while(!OC_GetNetStatus())
	{
		OSATaskSleep(200);
	}
	OC_UART_LOG_Printf("[%s]:Open  network Success!\n", __func__);
	return 0;
}

void net_monitor_thread(void * argv)
{
	OC_UART_LOG_Printf("[%s]:start net Monitor Thread!\n", __func__);
	while(NetMonitorRun)
	{
		int netstatus = OC_GetNetStatus();
		if(netstatus == 0) {
			OC_UART_LOG_Printf("[%s]:Reconnectting network!\n", __func__);
			dtu_netopen_worker();
		}
		OSATaskSleep(2000);  // 每10s查询一次网络状态
	}
	OC_UART_LOG_Printf("[%s]:Open  network Success!\n", __func__);
}

void customer_app_netopen_start(void)
{
	void *TaskStack;
	TaskStack=malloc(4096);
	if(TaskStack == NULL)
	{
		return;
	}
	dtu_netopen_worker();  // 阻塞等待网络连接
	NetMonitorRun = TRUE;
	if(OSATaskCreate(&netMonitorRef,
	                 TaskStack,
	                 4096,80,(char*)"NetOpenTask",
	                 net_monitor_thread, NULL) != 0)
	{
		return;
	}
}
