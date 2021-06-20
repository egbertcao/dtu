#include <stdio.h>
#include "global_types.h"
#include "oc_net.h"
#include "oc_fota.h"
#include "oc_sys.h"
#include "osa.h"
static OSTaskRef demoWorkerRef;
void netopen_worker_thread(void * argv)
{
	int bRet,regState,netStatus = 0;
	int progress = 0;
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
	//Open  network Success!
	OC_UART_LOG_Printf("Open  network Success!\n");
	OC_FOTA_Download("182.148.114.87:2210/file/fbf_dfota.bin","","");
	while(100 != progress)
	{
		OSATaskSleep(200);
		progress = OC_FOTA_GetDownloadProgress();
		OC_UART_LOG_Printf("%s:OC_FOTA_DownloadProgress:%d\n", __func__,progress);
	}
    OC_UART_LOG_Printf("PowerReset\n");
	OC_PowerReset();
}

void customer_app_fota_demo(void)
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


