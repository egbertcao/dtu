#include <stdio.h>
#include "global_types.h"
#include "oc_net.h"
#include "oc_location.h"
#include "oc_sys.h"
#include "osa.h"
static OSTaskRef demoWorkerRef;
void location_worker_thread(void * argv)
{
	int bRet,regState,netStatus = 0;
	Oc_Loc_Info location_info;
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
	bRet = OC_GetLocation(&location_info);
	OC_UART_LOG_Printf("bRet:%d, %s,%s\n", bRet, location_info.longitude,location_info.latitude);
}

void customer_app_location_demo(void)
{
	void *TaskStack;
	TaskStack=malloc(4096);
	if(TaskStack == NULL)
	{
		return;
	}
	if(OSATaskCreate(&demoWorkerRef,
	                 TaskStack,
	                 4096,80,(char*)"demoLocationTask",
	                 location_worker_thread, NULL) != 0)
	{
		return;
	}
}


