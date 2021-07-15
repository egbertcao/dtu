#include <stdio.h>
#include "global_types.h"
#include "oc_net.h"
#include "osa.h"
static OSTaskRef demoWorkerRef;
void netopen_worker_thread(void * argv)
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
	//Open  network Success!
}

void customer_app_netopen_demo(void)
{
	void *TaskStack;
	TaskStack=malloc(4096);
	if(TaskStack == NULL)
	{
		return;
	}
	if(OSATaskCreate(&demoWorkerRef,
	                 TaskStack,
	                 4096,80,(char*)"demoNetOpenTask",
	                 netopen_worker_thread, NULL) != 0)
	{
		return;
	}
}


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