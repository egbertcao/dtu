#include <stdio.h>
#include "global_types.h"
#include "oc_allystar.h"
#include "osa.h"
#include "oc_uart.h"

static void Cors_recv_cb(char* payload, int	payloadlen)
{
	OC_UART_LOG_Printf("%s payload:%x, payloadlen:%d\r\n", __func__,payload,payloadlen);
}

static void Cors_event_cb(oc_allystar_event_type_t event_type)
{
	OC_UART_LOG_Printf("%s event_type:%d\r\n", __func__,event_type);
}

static OSTaskRef demoWorkerRef;
static BOOL bRun = FALSE;

static void demo_allystar_thread(void * argv)
{
	int bRet,regState = 0;
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
	
	OC_UART_LOG_Printf("%s:Open  network Success!\r\n", __func__);
	OC_CorsUrcRegister(Cors_recv_cb, Cors_event_cb);
	OC_CorsConfig("20040011", "shyk01", "Ally_yk047", "allycors.allystar.com", 56721);
	OC_CorsOpen();
	while(bRun)
	{
		OSATaskSleep(2000);
	}
}

void customer_app_allystar_demo(void)
{
	void *TaskStack;
	TaskStack = malloc(4096);
	if(TaskStack == NULL)
	{
		return;
	}
	bRun = TRUE;
	if(OSATaskCreate(&demoWorkerRef,
	                 TaskStack,
	                 4096, 80, (char*)"demoallystarTask",
	                 demo_allystar_thread, NULL) != 0)
	{
		return;
	}
}


